#include "widget.h"
#include "screenwidget.h"
#include <QApplication>
#include <QFont>
#include <QIcon>

int main(int argc, char *argv[])
{
#if(QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    //使用 96 DPI 的缩放策略
    QApplication::setAttribute(Qt::AA_Use96Dpi);
#endif
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    //将 DPI 缩放因子向下取整，确保应用程序在高 DPI 屏幕上以正确的比例显示
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif

    QApplication a(argc, argv);

    QFont font;
    font.setFamily("Microsoft Yahei");
    font.setPointSize(13);
    a.setFont(font);

    Widget w;
    w.setWindowTitle("小艾截图");
    QIcon icon(":/C:/Users/admin/Pictures/screen.png");
    w.setWindowIcon(icon);
    w.show();
    return a.exec();
}
