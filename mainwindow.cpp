#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 实例化堆栈窗口和三个子页面
    QStackedWidget *stackedWidget = new QStackedWidget(this);
    displayW = new DisplayWidget();
    loginW = new LoginWidget();
    adminW = new AdminWidget();

    // 2. 将子页面加入堆栈 (索引依次为 0, 1, 2)
    stackedWidget->addWidget(displayW);
    stackedWidget->addWidget(loginW);
    stackedWidget->addWidget(adminW);

    // 设置堆栈窗口为 MainWindow 的中心部件
    this->setCentralWidget(stackedWidget);

    // 默认显示展示页
    stackedWidget->setCurrentWidget(displayW);

    // ================= 3. 核心信号与槽绑定 ================= //

    // 动作A：展示页 -> 点击登录 -> 切换到登录页
    connect(displayW, &DisplayWidget::requestLogin, [=](){
        stackedWidget->setCurrentWidget(loginW);
    });

    // 动作B：登录页 -> 验证成功 -> 切换到管理员页
    connect(loginW, &LoginWidget::loginSuccess, [=](){
        stackedWidget->setCurrentWidget(adminW);
    });

    // 动作C：登录页 -> 点击取消/返回 -> 切回展示页
    connect(loginW, &LoginWidget::cancelLogin, [=](){
        stackedWidget->setCurrentWidget(displayW);
    });



    // 在 MainWindow.cpp 中更新动作 D
    connect(adminW, &AdminWidget::applyConfig, [=](QString w, QString t, QString e, QStringList videos, QStringList images){
        // 1. 传递视频和图片列表给展示页
        displayW->updateDisplayData(w, t, e, videos, images);
        // 2. 切换回展示页面
        stackedWidget->setCurrentWidget(displayW);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

