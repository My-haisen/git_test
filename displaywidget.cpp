#include "displaywidget.h"
#include "ui_displaywidget.h"
#include <QTimer>
#include <QTime>

#include <QDebug>

DisplayWidget::DisplayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayWidget)
{
    ui->setupUi(this);
    initMediaPlayers();
    qsrand(QTime::currentTime().msec());
    isVideoMuted = false; // 初始状态为未静音
    // 初始化时设置未接入的样式
    ui->client_label->setText(" 远控未接入");
    ui->client_label->setStyleSheet("color: #e74c3c; font-size: 20px; font-weight: bold;");

    // === 初始化 TCP 服务器 ===
    tcpServer = new QTcpServer(this);
    // 监听本地所有 IP 的 8888 端口
    if (tcpServer->listen(QHostAddress::Any, 8888)) {
        qDebug() << "服务端启动成功，正在监听端口 8888...";
    } else {
        qDebug() << "服务端启动失败！";
    }

    // 绑定新连接信号
    connect(tcpServer, &QTcpServer::newConnection, this, &DisplayWidget::onNewConnection);
}
// 槽函数：处理客户端连入
void DisplayWidget::onNewConnection()
{
    // 获取与客户端通信的 Socket 句柄
    tcpSocket = tcpServer->nextPendingConnection();


    ui->client_label->setText("🟢 远控已接入");
    ui->client_label->setStyleSheet("color: #2ecc71; font-size: 20px; font-weight: bold;");

    // 绑定接收数据和断开连接的信号
    connect(tcpSocket, &QTcpSocket::readyRead, this, &DisplayWidget::onReadyRead);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &DisplayWidget::onClientDisconnected);
}

void DisplayWidget::onReadyRead()
{
    if (tcpSocket) {
        // 读取所有传过来的数据
        QByteArray data = tcpSocket->readAll();
        QString emergencyMsg = QString::fromUtf8(data);

        // 如果消息不为空，立刻更新到跑马灯
        if (!emergencyMsg.isEmpty()) {
            // 假设您原来定义的跑马灯变量叫 currentMarqueeText
            currentMarqueeText = "        【紧急通知】" + emergencyMsg + "        ";
            // 确保跑马灯定时器在运行
            marqueeTimer->start(150);

            // 可以给跑马灯加个高亮样式闪烁一下，提醒居民
            ui->lblScrollingText->setStyleSheet("color: #f1c40f; font-weight: bold; font-size: 38px;");
        }
    }
}


void DisplayWidget::onClientDisconnected()
{
    // 🌟 更新 UI 样式：恢复未接入状态
    ui->client_label->setText("🔴 远控未接入");
    ui->client_label->setStyleSheet("color: #e74c3c; font-size: 20px; font-weight: bold;");

    // 释放 Socket 资源
    tcpSocket->deleteLater();
    tcpSocket = nullptr;
}
DisplayWidget::~DisplayWidget()
{
    // 安全退出：关闭页面时强制杀掉后台的 MPlayer 进程，防止出现僵尸进程
    if (videoProcess && videoProcess->state() == QProcess::Running) {
        videoProcess->write("quit\n");
        videoProcess->kill();
        videoProcess->waitForFinished(1000);
    }
    delete ui;
}

void DisplayWidget::on_btnAdminLogin_clicked()
{
    // 【核心修复】：在离开展示页之前，彻底关闭视频，防止画面残留在屏幕上
    if (videoProcess && videoProcess->state() == QProcess::Running) {
        videoProcess->write("quit\n");
        videoProcess->kill();
        videoProcess->waitForFinished(500);
    }

    // 暂停其他定时器以节省 CPU
    imageTimer->stop();
    marqueeTimer->stop();
    typewriterTimer->stop();

    emit requestLogin(); // 告诉大管家切换界面
}


void DisplayWidget::updateDisplayData(QString weather, QString tips, QString emergency,
                                      QStringList videoList, QStringList imageList)
{

    if(!weather.isEmpty()) {
        // 将后台传来的纵向多行文本按换行符拆分
        QStringList parts = weather.split("\n");

        // 确保数据完整，提取城市、天气、温度和风力
        if(parts.size() >= 5) {
            QString city = parts[0].section("：", 1, 1).trimmed(); // 提取“北京”
            QString wea  = parts[2].section("：", 1, 1).trimmed(); // 提取“晴”
            QString temp = parts[3].section("：", 1, 1).trimmed(); // 提取“22~28 ℃”
            QString wind = parts[4].section("：", 1, 1).trimmed(); // 提取“北风 3级”

            // 拼接成带独立颜色的横向 HTML 富文本（&nbsp; 用于增加宽敞的空格间距）
            QString richWeather = QString("<span style='color: #ecf0f1;'>📍 %1</span>&nbsp;&nbsp;&nbsp;&nbsp;"
                                          "<span style='color: #f1c40f;'>⛅ %2</span>&nbsp;&nbsp;&nbsp;&nbsp;"
                                          "<span style='color: #ff9f43;'>🌡️ %3</span>&nbsp;&nbsp;&nbsp;&nbsp;"
                                          "<span style='color: #2ecc71;'>🍃 %4</span>")
                                          .arg(city).arg(wea).arg(temp).arg(wind);

            ui->lblWeather->setText(richWeather);
        } else {
            // 防错兜底：如果格式异常，直接把回车换成竖线分隔
            ui->lblWeather->setText(weather.replace("\n", " | "));
        }
    }
    if(!tips.isEmpty()) ui->lblTips->setText("温馨提示：" + tips);


    if(!emergency.isEmpty()) {
        currentMarqueeText = "        【紧急通知】" + emergency + "        ";
        marqueeTimer->start(150);
    }


    announcementText = "【社区近期公告】\n欢迎使用智慧社区系统。本系统现已升级为多媒体播放模式，祝您生活愉快！";
    typeIndex = 0;
    ui->lblAnnouncement->clear();
    typewriterTimer->start(100);


    if (videoProcess->state() == QProcess::Running) {
        videoProcess->write("quit\n");
        videoProcess->kill();
        videoProcess->waitForFinished(1000);
    }

    isVideoMuted = false;
    ui->pushButton_videomute->setText("🔇 视频静音");
    ui->pushButton_videomute->setStyleSheet("");

    currentVideoList = videoList;
    if(!currentVideoList.isEmpty()) {

        QTimer::singleShot(200, this, SLOT(startMplayerVideo()));
    }


    currentImageList = imageList;
    currentImageIndex = 0;
    if(!currentImageList.isEmpty()) {
        ui->lblImage->setGraphicsEffect(nullptr);
        playNextImageWithEffect();
        imageTimer->start(5000);
    } else {
        imageTimer->stop();
    }
}

void DisplayWidget::initMediaPlayers()
{
    //  QProcess
    videoProcess = new QProcess(this);


    mplayerWindow = new QWidget(this);
    mplayerWindow->setStyleSheet("background-color: black;");

    // 把这块画布加入到布局中，并隐藏原来的占位文字
    ui->verticalLayout_video->addWidget(mplayerWindow);
    ui->lblVideoPlaceholder->hide();


    imageTimer = new QTimer(this);
    currentImageIndex = 0;
    connect(imageTimer, &QTimer::timeout, this, &DisplayWidget::playNextImageWithEffect);


    marqueeTimer = new QTimer(this);
    connect(marqueeTimer, &QTimer::timeout, this, &DisplayWidget::updateMarquee);


    typewriterTimer = new QTimer(this);
    connect(typewriterTimer, &QTimer::timeout, this, &DisplayWidget::updateTypewriter);
}

//多种图片切换动画
void DisplayWidget::playNextImageWithEffect()
{
    if(currentImageList.isEmpty()) return;

    QString imgPath = currentImageList[currentImageIndex];
    QPixmap pix(imgPath);

    QPixmap scaledPix = pix.scaled(ui->lblImage->size(),
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);

    int cropX = (scaledPix.width() - ui->lblImage->width()) / 2;
    int cropY = (scaledPix.height() - ui->lblImage->height()) / 2;
    ui->lblImage->setPixmap(scaledPix.copy(cropX, cropY, ui->lblImage->width(), ui->lblImage->height()));


    // 随机选择一种特效 (0: 淡入, 1: 滑入)
    int effectType = qrand() % 2; // 【替换为这句】
    // =========================================================================

    if (effectType == 0) {
        // --- 效果 A: 纯淡入 (Fade In) ---
        QGraphicsOpacityEffect *opEffect = new QGraphicsOpacityEffect(this);
        ui->lblImage->setGraphicsEffect(opEffect);

        QPropertyAnimation *anim = new QPropertyAnimation(opEffect, "opacity");
        anim->setDuration(1000);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else {
        // --- 效果 B: 从上向下滑入 (Slide Down) ---
        ui->lblImage->setGraphicsEffect(nullptr); // 清除透明度特效

        QPropertyAnimation *anim = new QPropertyAnimation(ui->lblImage, "pos");
        anim->setDuration(800);
        anim->setEasingCurve(QEasingCurve::OutBounce);

        QPoint endPos = ui->lblImage->pos();
        QPoint startPos = QPoint(endPos.x(), endPos.y() - 200);

        anim->setStartValue(startPos);
        anim->setEndValue(endPos);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    currentImageIndex = (currentImageIndex + 1) % currentImageList.size();
}

void DisplayWidget::updateMarquee()
{
    // 将字符串第一个字符移到最后，形成视觉上的向左滚动
    if(currentMarqueeText.length() > 0) {
        currentMarqueeText = currentMarqueeText.mid(1) + currentMarqueeText.left(1);
        ui->lblScrollingText->setText(currentMarqueeText);
    }
}

void DisplayWidget::updateTypewriter()
{
    if (typeIndex < announcementText.length()) {
        ui->lblAnnouncement->setText(ui->lblAnnouncement->text() + announcementText.at(typeIndex));
        typeIndex++;
    } else {
        typewriterTimer->stop();
        QTimer::singleShot(10000, this, [=](){
            ui->lblAnnouncement->clear();
            typeIndex = 0;
            typewriterTimer->start(100);
        });
    }
}
void DisplayWidget::on_pushButton_videomute_clicked()
{
    // 切换静音状态
    isVideoMuted = !isVideoMuted;

    if (isVideoMuted) {
        // 【静音状态】按钮变红，文字变“解除静音”
        ui->pushButton_videomute->setText("🔊 解除静音");
        ui->pushButton_videomute->setStyleSheet(
            "background-color: rgba(231, 76, 60, 180);" // 红色
            "border: 1px solid #e74c3c;"
            "color: white; border-radius: 6px; padding: 4px 10px; font-weight: bold;"
        );

        // 向 mplayer 发送静音开启指令
        if (videoProcess && videoProcess->state() == QProcess::Running) {
            videoProcess->write("mute 1\n");
        }
    } else {
        // 【恢复正常】按钮变绿，文字变“视频静音”
        ui->pushButton_videomute->setText("🔇 视频静音");
        ui->pushButton_videomute->setStyleSheet(
            "background-color: rgba(46, 204, 113, 180);" // 绿色
            "border: 1px solid #2ecc71;"
            "color: white; border-radius: 6px; padding: 4px 10px; font-weight: bold;"
        );

        // 向 mplayer 发送静音解除指令
        if (videoProcess && videoProcess->state() == QProcess::Running) {
            videoProcess->write("mute 0\n");
        }
    }
}

void DisplayWidget::startMplayerVideo()
{
    QString program = "./mplayer";
    QStringList args;

    args << "-slave" << "-quiet" << "-vo" << "fbdev2";


    QPoint globalPos = ui->videoContainer->mapToGlobal(QPoint(0, 0));
    int videoWidth = ui->videoContainer->width();
    int videoHeight = ui->videoContainer->height();


    if(videoWidth <= 100) videoWidth = 380;
    if(videoHeight <= 100) videoHeight = 350;

    // 指定画面缩放大小
    args << "-zoom";
    args << "-x" << QString::number(videoWidth);
    args << "-y" << QString::number(videoHeight);

    // 指定起始物理坐标
    args << "-geometry" << QString("%1:%2").arg(globalPos.x()+20).arg(globalPos.y()+20);

    // 指定音频驱动
    args << "-ao" << "alsa";
    args << "-loop" << "0";

    foreach(const QString &v, currentVideoList) {
        args << v;
    }

    videoProcess->start(program, args);
}
