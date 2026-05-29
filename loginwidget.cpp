#include "loginwidget.h"
#include "ui_loginwidget.h"

LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}
#include <QMessageBox>

void LoginWidget::on_btnLogin_clicked()
{
    QString user = ui->txtUsername->text();
    QString pass = ui->txtPassword->text();

    // 验证账号密码
    if (user == "root" && pass == "123456") {
        // 验证成功：清空密码框（防止下次进来还有密码），并发送成功信号
        ui->txtPassword->clear();
        emit loginSuccess();
    } else {
        // 验证失败：弹窗提示，并清空输入框
        QMessageBox::critical(this, "登录失败", "用户名或密码错误，请重新输入！");
        ui->txtPassword->clear();
        ui->txtPassword->setFocus(); // 光标重新定位到密码框
        ui->txtPassword->clear();
        emit loginSuccess();
    }
}

void LoginWidget::on_btnExit_clicked()
{
    emit cancelLogin(); // 用户点击返回，发送取消信号
}
