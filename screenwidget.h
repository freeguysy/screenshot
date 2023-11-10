#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QSize>
#include <QPoint>
#include "widget.h"


class Screen
{
public:
    //截图框的状态
    enum STATUS{SELECT,MOV,SET_W_H};
    Screen();
    Screen(QSize size);

    void setStart(QPoint pos);
    void setEnd(QPoint pos);
    void setStatus(STATUS status);

    QPoint getStart();
    QPoint getEnd();
    QPoint getLeftUp();
    QPoint getRightDown();
    STATUS getStatus();
    int getmMxWidth();
    int getMaxHeight();

    bool isInArea(QPoint pos);      //是否在截图框区域内
    void move(QPoint pos);          //截图框移动函数

private:
    QPoint leftUp,rightDown;        //截图框的左上角和右下角
    QPoint startPos,endPos;         //截图框的起始位置和终止位置
    int maxWidth,maxHeight;         //记录屏幕的大小
    STATUS status;
    //始终保证p1在左上角，p2在右下角
    void compare(QPoint& p1,QPoint& p2);
};

class ScreenWidget : public QWidget
{
    Q_OBJECT
public:
    //静态函数
    static ScreenWidget* instance();
    explicit ScreenWidget(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *);          //鼠标移动
    void mousePressEvent(QMouseEvent *);         //鼠标左键
    void mouseReleaseEvent(QMouseEvent *);       //鼠标释放
    void contextMenuEvent(QContextMenuEvent *);  //鼠标右键
    void paintEvent(QPaintEvent *);              //绘制截图
    void showEvent(QShowEvent *);               //绘制模糊背景

private:
    //智能指针，独享对象(unique_ptr)
    static QScopedPointer<ScreenWidget> self;
    QMenu *menu;            //菜单项
    Screen *screen;         //截图对象
    QPixmap *fullScreen;    //全屏截图
    QPixmap *bgScreen;      //模糊背景图
    QPoint movePos;        //移动的偏移量

private slots:
    //保存和另存为
    void saveScreen();
    void saveFullScreen();
    void saveScreenOth();
    void saveFullScreenOth();
};

#endif // SCREENWIDGET_H
