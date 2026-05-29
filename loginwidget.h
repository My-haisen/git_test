#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

private:
    Ui::LoginWidget *ui;

    // 在 LoginWidget.h 中
signals:
    void loginSuccess();  // 登录成功信号
    void cancelLogin();   // 取消登录信号 (返回主页)

private slots:
    void on_btnLogin_clicked();
    void on_btnExit_clicked(); // 对应界面的退出/返回按钮
};

#endif // LOGINWIDGET_H
