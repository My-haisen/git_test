#include "mainwindow.h"

#include <QApplication>
#include "adminwidget.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
//    w.setFixedSize(800, 480);

        // 全屏显示（如果物理屏幕大于 800x480，它会居中显示，或者配合样式表铺满）
    w.show();
    return a.exec();
}
