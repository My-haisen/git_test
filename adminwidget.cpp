#include "AdminWidget.h"
#include "ui_AdminWidget.h"
#include <QFileDialog>
#include <QDebug>
#include <QTimer>
#include <QTime>
AdminWidget::AdminWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminWidget)
{
    ui->setupUi(this);

    // 初始化音乐播放器 (MPlayer)
    initMusicPlayer();

    // 初始化网络管理器并绑定信号槽
    weatherNetManager = new QNetworkAccessManager(this);
    // --- 初始化定时发布检查器 ---
    scheduleTimer = new QTimer(this);
    scheduleTimer->setSingleShot(true);
    connect(scheduleTimer, &QTimer::timeout, this, &AdminWidget::checkScheduledTime);


}

AdminWidget::~AdminWidget()
{
    // 安全退出机制：如果界面关闭时 MPlayer 还在运行，强制结束它，防止变成后台僵尸进程
    if (mplayerProcess && mplayerProcess->state() == QProcess::Running) {
        mplayerProcess->write("quit\n"); // 发送退出指令
        mplayerProcess->kill();          // 强制杀进程
        mplayerProcess->waitForFinished(1000);
    }
    delete ui;
}

void AdminWidget::on_pushButton_quick2_clicked()
{
    ui->txtEmergency->setText("请大家注意社区安全，规范停车，共建美好家园！");
}
// --- 初始化 MPlayer 进程 ---
void AdminWidget::initMusicPlayer()
{
    mplayerProcess = new QProcess(this);
    isMusicPlaying = false;

    // 初始化按钮显示的文本
    ui->pushButton_Music->setText("▶️ 播放音乐");
}

// --- 选择音乐文件 ---
void AdminWidget::on_btnSelectBgm_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
                this,
                "选择背景音乐",
                "/media",
                "Audio Files (*.mp3 *.wav)"
                );

    if (!filePath.isEmpty()) {
        ui->txtBgmPath->setText(filePath);

        // 如果之前有音乐在播放，先杀掉旧的 MPlayer 进程
        if (mplayerProcess->state() == QProcess::Running) {
            mplayerProcess->write("quit\n");
            mplayerProcess->kill();
            mplayerProcess->waitForFinished(1000);
        }

        // 重置状态
        isMusicPlaying = false;
        ui->pushButton_Music->setText("▶️ 播放音乐");
    }
}


void AdminWidget::on_pushButton_Music_clicked()
{
    QString currentPath = ui->txtBgmPath->text();

    if (currentPath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要播放的音乐文件！");
        return;
    }

    // 判断 MPlayer 进程是否已经在运行
    if (mplayerProcess->state() == QProcess::NotRunning) {
        // 如果没有运行，则启动 MPlayer
        QString program = "./mplayer";
        QStringList arguments;

        // -slave: 开启从机模式，接收控制台指令
        // -quiet: 减少控制台不必要的输出
        // -loop 0: 无限循环播放
        arguments << "-slave" << "-quiet" << "-loop" << "0" << currentPath;

        mplayerProcess->start(program, arguments);

        isMusicPlaying = true;
        ui->pushButton_Music->setText("暂停播放");

    } else {
        //如果已经在运行，则向它发送 pause 指令进行 播放/暂停 切换
        mplayerProcess->write("pause\n");

        // 翻转状态标志位
        isMusicPlaying = !isMusicPlaying;

        if (isMusicPlaying) {
            ui->pushButton_Music->setText(" 暂停播放");
        } else {
            ui->pushButton_Music->setText("▶️ 播放音乐");
        }
    }
}


void AdminWidget::on_btnTestWeather_clicked()
{
    // 获取当前程序运行路径下的 week.json
    QString jsonPath = QCoreApplication::applicationDirPath() + "/week.json";

    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->lblWeatherResult->setText("❌ 获取数据失败：找不到 week.json 文件");
        ui->lblWeatherResult->setStyleSheet("color: #e74c3c; font-size: 18px;");
        return;
    }

    // 指定 UTF-8 编码读取，防止中文乱码
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString jsonString = in.readAll();
    file.close();


    auto extractValue = [&](const QString& key) -> QString {
        QString searchKey = "\"" + key + "\"";
        int keyPos = jsonString.indexOf(searchKey);
        if (keyPos == -1) return "未知"; // 找不到键

        int colonPos = jsonString.indexOf(":", keyPos);
        if (colonPos == -1) return "未知";

        int startQuote = jsonString.indexOf("\"", colonPos);
        if (startQuote == -1) return "未知";

        int endQuote = jsonString.indexOf("\"", startQuote + 1);
        if (endQuote == -1) return "未知";

        // 截取两个双引号之间的具体数值
        return jsonString.mid(startQuote + 1, endQuote - startQuote - 1);
    };

    // 暴力提取我们需要的数据
    QString currentCity = extractValue("city");
    QString updateTime  = extractValue("update_time");
    QString wea         = extractValue("wea");
    QString tem_day     = extractValue("tem_day");
    QString tem_night   = extractValue("tem_night");
    QString win         = extractValue("win");
    QString win_speed   = extractValue("win_speed");


    if(currentCity == "未知" && wea == "未知") {
        ui->lblWeatherResult->setText(" 获取数据失败，本地 week.json 格式异常！");
        ui->lblWeatherResult->setStyleSheet("color: #e74c3c; font-size: 18px;");
        return;
    }

    // 拼接输出结果
    QString resultStr = QString("📍 查询城市：%1\n🕒 更新时间：%2\n⛅ 今日天气：%3\n🌡️ 今日温度：%4~%5 ℃\n🍃 风向风力：%6 %7")
                        .arg(currentCity).arg(updateTime).arg(wea).arg(tem_night).arg(tem_day).arg(win).arg(win_speed);

    ui->lblWeatherResult->setText(resultStr);
    ui->lblWeatherResult->setStyleSheet("color: #27ae60; font-size: 18px; font-weight: bold;");
}
//void AdminWidget::onWeatherReply(QNetworkReply *reply) {
//    if(reply->error() != QNetworkReply::NoError) {
//        ui->lblWeatherResult->setText("❌ 网络错误：" );
//        ui->lblWeatherResult->setStyleSheet("color: #e74c3c; font-size: 18px;");
//        reply->deleteLater();
//        return;
//    }
//    QByteArray responseData = reply->readAll();
//    reply->deleteLater();
//    QJsonParseError jsonError;
//    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &jsonError);
//    if(jsonError.error != QJsonParseError::NoError || !jsonDoc.isObject()){
//        ui->lblWeatherResult->setText("❌ 获取数据失败，返回的数据格式异常！");
//        ui->lblWeatherResult->setStyleSheet("color: #e74c3c; font-size: 18px;");
//        return;
//    }
//    QJsonObject responseObj = jsonDoc.object();
//    if(responseObj.contains("errcode")) {
//        QString errMsg = responseObj.value("errmsg").toString();
//        ui->lblWeatherResult->setText("❌ 查询失败：" + errMsg);
//        ui->lblWeatherResult->setStyleSheet("color: #e74c3c; font-size: 18px;");
//        return;
//    }
//    QString currentCity = responseObj.value("city").toString();
//    QString updateTime = responseObj.value("update_time").toString();
//    QJsonArray weatherDataArray = responseObj.value("data").toArray();
//    if(!weatherDataArray.isEmpty()) {
//        QJsonObject todayObj = weatherDataArray[0].toObject();
//        QString wea = todayObj.value("wea").toString();
//        QString tem_day = todayObj.value("tem_day").toString();
//        QString tem_night = todayObj.value("tem_night").toString();
//        QString win = todayObj.value("win").toString();
//        QString win_speed = todayObj.value("win_speed").toString();
//        QString resultStr = QString("📍 查询城市：%1\n🕒 更新时间：%2\n⛅ 今日天气：%3\n🌡️ 今日温度：%4~%5 ℃\n🍃 风向风力：%6 %7")
//                            .arg(currentCity).arg(updateTime).arg(wea).arg(tem_night).arg(tem_day).arg(win).arg(win_speed);
//        ui->lblWeatherResult->setText(resultStr);
//        ui->lblWeatherResult->setStyleSheet("color: #27ae60; font-size: 18px; font-weight: bold;");
//    } else {
//        ui->lblWeatherResult->setText("获取成功，但缺少天气详细数据");
//        ui->lblWeatherResult->setStyleSheet("color: #f39c12; font-size: 18px;");
//    }
//}

//倒计时发布逻辑 ==================
void AdminWidget::on_btnApply_clicked()
{
    QString weatherInfo = ui->lblWeatherResult->text();
    QString tips = ui->txtTips->toPlainText();
    QString emergency = ui->txtEmergency->text();

    QStringList vList, iList;
    for(int i=0; i < ui->listVideos->count(); ++i) vList << ui->listVideos->item(i)->text();
    for(int i=0; i < ui->listImages->count(); ++i) iList << ui->listImages->item(i)->text();

    emit applyConfig(weatherInfo, tips, emergency, vList, iList);
}
void AdminWidget::on_btnVideoAdd_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "选择视频文件", "/media", "视频 (*.mp4 *.avi)");
    ui->listVideos->addItems(files);
}
void AdminWidget::on_btnVideoDel_clicked() { qDeleteAll(ui->listVideos->selectedItems()); }
void AdminWidget::on_btnVideoUp_clicked() {
    int row = ui->listVideos->currentRow();
    if (row > 0) {
        QListWidgetItem *item = ui->listVideos->takeItem(row);
        ui->listVideos->insertItem(row - 1, item);
        ui->listVideos->setCurrentRow(row - 1);
    }
}
void AdminWidget::on_btnVideoDown_clicked() {
    int row = ui->listVideos->currentRow();
    if (row >= 0 && row < ui->listVideos->count() - 1) {
        QListWidgetItem *item = ui->listVideos->takeItem(row);
        ui->listVideos->insertItem(row + 1, item);
        ui->listVideos->setCurrentRow(row + 1);
    }
}

void AdminWidget::on_btnImageAdd_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "选择图文海报", "/media", "图片 (*.png *.jpg *.jpeg)");
    ui->listImages->addItems(files);
}
void AdminWidget::on_btnImageDel_clicked() { qDeleteAll(ui->listImages->selectedItems()); }
void AdminWidget::on_btnImageUp_clicked() {
    int row = ui->listImages->currentRow();
    if (row > 0) {
        QListWidgetItem *item = ui->listImages->takeItem(row);
        ui->listImages->insertItem(row - 1, item);
        ui->listImages->setCurrentRow(row - 1);
    }
}
void AdminWidget::on_btnImageDown_clicked() {
    int row = ui->listImages->currentRow();
    if (row >= 0 && row < ui->listImages->count() - 1) {
        QListWidgetItem *item = ui->listImages->takeItem(row);
        ui->listImages->insertItem(row + 1, item);
        ui->listImages->setCurrentRow(row + 1);
    }
}
void AdminWidget::checkScheduledTime()
{
    // 如果倒计时结束时，用户已经取消了勾选，则不执行
    if (!ui->checkBox_timer->isChecked()) {
        return;
    }

    // 重新抓取界面数据（保证发出的是最新的）
    QString weatherInfo = ui->lblWeatherResult->text();
    QString tips = ui->txtTips->toPlainText();
    QString emergency = ui->txtEmergency->text();

    QStringList vList, iList;
    for(int i=0; i < ui->listVideos->count(); ++i) vList << ui->listVideos->item(i)->text();
    for(int i=0; i < ui->listImages->count(); ++i) iList << ui->listImages->item(i)->text();

    // 倒计时到达，发送包含紧急信息的完整配置
    emit applyConfig(weatherInfo, tips, emergency, vList, iList);

    // 任务完成，取消勾选
    ui->checkBox_timer->setChecked(false);
}

void AdminWidget::on_pushButton_videoget_clicked()
{
    QDir dir("./videos");
    if (!dir.exists()) {
        QMessageBox::warning(this, "错误", "视频目录不存在！\n" + dir.absolutePath());
        return;
    }

    QStringList filters;
    filters << "*.mp4" << "*.avi" << "*.mkv" << "*.mov"; // 添加常见视频格式
    dir.setNameFilters(filters);

    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    ui->listVideos->clear(); // 清空旧列表
    for (const QFileInfo &info : list) {
        ui->listVideos->addItem(info.absoluteFilePath());
    }

    QMessageBox::information(this, "提示", QString("已成功扫描并导入 %1 个视频！").arg(list.size()));
}
void AdminWidget::on_pushButton_imageget_clicked()
{
    QDir dir("./images");
    if (!dir.exists()) {
        QMessageBox::warning(this, "错误", "图片目录不存在！\n" + dir.absolutePath());
        return;
    }

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp"; // 添加常见图片格式
    dir.setNameFilters(filters);

    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    ui->listImages->clear(); // 清空旧列表
    for (const QFileInfo &info : list) {
        ui->listImages->addItem(info.absoluteFilePath());
    }

    QMessageBox::information(this, "提示", QString("已成功扫描并导入 %1 张图片！").arg(list.size()));
}

void AdminWidget::on_btnScanUsb_clicked()
{
    QDir udiskDir("/mnt/udisk/");

    // 1. 判断 U盘是否成功挂载
    if (!udiskDir.exists()) {
        ui->lineEdit_showcard->setText("ERROR: 未检测到 /mnt/udisk/ 挂载点，请检查U盘是否插入！");
        ui->lineEdit_showcard->setStyleSheet("color: #ff4757; background-color: #2f3542;"); // 报错时变为红色

        // 同步更新顶部的全局状态栏
        ui->lblStorageStatus->setText(" U盘/SD卡状态：未检测到设备");
        ui->lblStorageStatus->setStyleSheet("color: #e74c3c; font-size: 18px; font-weight: bold;");
        return;
    }

    // 恢复正常的科技蓝绿色样式
    ui->lineEdit_showcard->setStyleSheet("");

    // 2. 扫描文件并统计数量
    udiskDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList list = udiskDir.entryInfoList();

    int mp3Count = 0;
    int mp4Count = 0;
    int imgCount = 0;

    for (const QFileInfo &info : list) {
        QString suffix = info.suffix().toLower();
        if (suffix == "mp3" || suffix == "wav") mp3Count++;
        else if (suffix == "mp4" || suffix == "avi") mp4Count++;
        else if (suffix == "png" || suffix == "jpg" || suffix == "jpeg") imgCount++;
    }

    // 3. 将结果输出到展示屏
    QString statusStr = QString(">_ 扫描成功 | 发现 %1 个文件 [ 音乐: %2 | 视频: %3 | 图片: %4 ]")
                        .arg(list.size()).arg(mp3Count).arg(mp4Count).arg(imgCount);

    ui->lineEdit_showcard->setText(statusStr);

    // 同步更新顶部的全局状态栏
    ui->lblStorageStatus->setText(" U盘/SD卡状态：已连接 (/mnt/udisk/)");
    ui->lblStorageStatus->setStyleSheet("color: #2ecc71; font-size: 18px; font-weight: bold;");
}


void AdminWidget::on_btnLoadUsbPlaylist_clicked()
{
    QDir udiskDir("/mnt/udisk/");
    if (!udiskDir.exists()) {
        QMessageBox::warning(this, "错误", "未检测到 U盘挂载，请先插入并挂载 U盘！");
        return;
    }

    // 设置过滤条件，只找音乐文件
    QStringList filters;
    filters << "*.mp3" << "*.wav";
    udiskDir.setNameFilters(filters);

    QFileInfoList list = udiskDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    if (list.isEmpty()) {
        ui->lineEdit_showcard->setText("WARNING: /mnt/udisk/ 目录下没有找到 .mp3 或 .wav 文件！");
        ui->lineEdit_showcard->setStyleSheet("color: #ffa502; background-color: #2f3542;");
        QMessageBox::information(this, "提示", "U盘根目录中未找到任何音乐文件！");
        return;
    }

    // 提取第一个音乐文件的绝对路径
    QString firstMusicPath = list.first().absoluteFilePath();

    // 显示到全局背景音乐的文本框中
    ui->txtBgmPath->setText(firstMusicPath);

    // 在展示屏上也打印一条成功日志
    ui->lineEdit_showcard->setStyleSheet("");
    ui->lineEdit_showcard->setText(QString(">_ 已载入音轨: %1").arg(list.first().fileName()));

    // 提示用户可以点击播放了
    QMessageBox::information(this, "成功", "已成功读取 U盘！\n准备播放：\n" + list.first().fileName());
}
