#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QProcess> // 引入 QProcess 替代 QMediaPlayer
#include <QTcpServer>
#include <QTcpSocket>
namespace Ui {
class DisplayWidget;
}

class DisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayWidget(QWidget *parent = nullptr);
    ~DisplayWidget();

    // 更新接口
    void updateDisplayData(QString weather, QString tips, QString emergency,
                           QStringList videoList, QStringList imageList);

signals:
    void requestLogin(); // 请求进入登录页面

private slots:
    void on_btnAdminLogin_clicked();

private:
    Ui::DisplayWidget *ui;

    // --- 核心修改：使用 QProcess 替代多媒体组件 ---
    QProcess *videoProcess;
    QWidget *mplayerWindow; // 专门用来承载 MPlayer 视频画面的空白窗口

    // 图片轮播控制
    QTimer *imageTimer;
    QStringList currentImageList;
    int currentImageIndex;

    // 文字特效控制
    QTimer *marqueeTimer;
    QString currentMarqueeText;

    QTimer *typewriterTimer;
    QString announcementText;
    int typeIndex;

    // 内部初始化与播放函数
    void initMediaPlayers();
    void playNextImageWithEffect();
    void updateMarquee();
    void updateTypewriter();
    // 在 class DisplayWidget 中找到 private slots 区域，添加：

    // --- 网络通信组件 ---
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
     QStringList currentVideoList; // 保存当前的视频列表
private slots:
    void onNewConnection();       // 有新客户端连接时触发
    void onReadyRead();           // 接收到客户端发来的数据时触发
    void onClientDisconnected();  // 客户端断开连接时触发
private slots:

    void on_pushButton_videomute_clicked(); // 新增：处理视频静音按钮点击
    void startMplayerVideo(); // 专门用来延迟启动 MPlayer
// 在 private 区域，添加一个标志位：
private:
    bool isVideoMuted; // 新增：记录当前视频是否被静音
};

#endif // DISPLAYWIDGET_H
