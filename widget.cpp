#include "widget.h"
#include "ui_widget.h"
#include "screenwidget.h"
#include <QTimer>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
    exit(0);
}


void Widget::on_pushButton_clicked()
{
    //获取ScreenWidget类的单例对象，并调用showFullScreen,窗口将会占据整个屏幕，并隐藏标题栏、边框和任务栏。
    this->showMinimized();
    //睡眠0.2s以便完成上述操作
    QTimer::singleShot(200, [this]() {
        // 在定时器超时时执行showFullScreen()
        ScreenWidget::instance()->showFullScreen();
    });
}

