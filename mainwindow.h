#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "displaywidget.h"
#include "loginwidget.h"
#include "adminwidget.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DisplayWidget *displayW;
    LoginWidget *loginW;
    AdminWidget * adminW;

    int temp_dir;
    int test_git;
    int aaaa;
};
#endif // MAINWINDOW_H
