#ifndef ADMINWIDGET_H
#define ADMINWIDGET_H

#include <QWidget>
#include <QProcess> // 引入 QProcess 替代 QMediaPlayer
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QTextStream>
namespace Ui {
class AdminWidget;
}

class AdminWidget : public QWidget {
    Q_OBJECT

public:
    explicit AdminWidget(QWidget *parent = nullptr);
    ~AdminWidget();

private slots:
    // 选择音乐文件按钮
    void on_btnSelectBgm_clicked();
    // 播放/暂停 控制按钮
    void on_pushButton_Music_clicked();

    // 天气与其它后台配置槽函数 (保持不变)
    void on_btnTestWeather_clicked();
    void on_pushButton_quick2_clicked(); // 【新增】：一键填入快捷语
    void on_btnApply_clicked();

    // 媒体列表管理槽函数 (保持不变)
    void on_btnVideoAdd_clicked();
    void on_btnVideoDel_clicked();
    void on_btnVideoUp_clicked();
    void on_btnVideoDown_clicked();
    void on_btnImageAdd_clicked();
    void on_btnImageDel_clicked();
    void on_btnImageUp_clicked();
    void on_btnImageDown_clicked();
    void checkScheduledTime();

    // --- 新增：自动扫描获取槽函数 ---
    void on_pushButton_videoget_clicked();
    void on_pushButton_imageget_clicked();
    void on_btnScanUsb_clicked();
    void on_btnLoadUsbPlaylist_clicked();


signals:
    void applyConfig(QString weather, QString tips, QString emergency,
                     QStringList videoList, QStringList imageList);

private:
    Ui::AdminWidget *ui;

    // --- MPlayer 核心组件 ---
    QProcess *mplayerProcess;
    bool isMusicPlaying; // 由于 QProcess 没有 PlayingState，我们需要一个标志位来记录状态

    QNetworkAccessManager *weatherNetManager;
    // --- 新增：定时发布检查器 ---
    QTimer *scheduleTimer;
    void initMusicPlayer();

};

#endif // ADMINWIDGET_H
