#include "screenwidget.h"
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QEvent>
#include <QMouseEvent>
#include "qapplication.h"
#include <QPainter>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>

#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include "qscreen.h"
//返回一个QRect对象，包含了主屏幕的左上角坐标和宽高信息
#define deskGeometry qApp->primaryScreen()->geometry()
//返回一个QRect对象，包含了可用工作区的左上角坐标和宽高信息
#define deskGeometry2 qApp->primaryScreen()->availableGeometry()
#else
//同上
#include "qdesktopwidget.h"
#define deskGeometry qApp->desktop()->geometry()
#define deskGeometry2 qApp->desktop()->availableGeometry()
#endif
//将当前时间格式调整为yyyy-MM-dd-HH-mm-ss并转换为const char*指针(qPrintable)
#define STRDATETIME qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"))

//静态成员和函数类外声明
QScopedPointer<ScreenWidget> ScreenWidget::self;
ScreenWidget *ScreenWidget::instance()
{
    //当实例为空时，通过上锁来保证单例模式
    if(self.isNull())
    {
        //静态成员函数使用静态成员变量
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        //第一个线程到达，创建实例,后续线程抢到也不执行
        if(self.isNull())
            self.reset(new ScreenWidget);
    }
    //将智能指针转换为实例指针返回
    return self.data();
}

ScreenWidget::ScreenWidget(QWidget *parent)
    : QWidget{parent}
{
    //菜单项
    menu = new QMenu(this);
    menu->addAction("保存当前截图",this,SLOT(saveScreen()));
    menu->addAction("保存全屏截图",this,SLOT(saveFullScreen()));
    menu->addAction("当前截图另存为",this,SLOT(saveScreenOth()));
    menu->addAction("全屏截图另存为",this,SLOT(saveFullScreenOth()));
    menu->addAction("退出截图",this,SLOT(hide()));      //隐藏窗口或控件
    //全屏大小作为初始化值
    screen = new Screen(deskGeometry.size());
    fullScreen = new QPixmap();
}

void ScreenWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(screen->getStatus() == Screen::SELECT){
        screen->setEnd(e->pos());
    }else if(screen->getStatus() == Screen::MOV){
        //p 是相对于上一次鼠标位置的偏移量(点击拖动的位置和释放的位置的偏移量)
        QPoint p(e->x()-movePos.x(),e->y()-movePos.y());
        screen->move(p);
        //重新记录点以准备下一次移动
        movePos=e->pos();
    }
    this->update();
}

void ScreenWidget::mousePressEvent(QMouseEvent *e)
{

    if(screen->getStatus() == Screen::SELECT){
        //截图是记录左上角的点，移动是记录鼠标点击的地方
        screen->setStart(e->pos());
    }else{
        //在矩形框内是移动，在框外是重选
        if(screen->isInArea(e->pos()) == false){
            screen->setStart(e->pos());
            screen->setStatus(Screen::SELECT);
        }else{
            //矩形框内鼠标点击的点
            movePos = e->pos();
            //将鼠标设置为方向标
            this->setCursor(Qt::SizeAllCursor);
        }
    }
    this->update();
}

void ScreenWidget::mouseReleaseEvent(QMouseEvent *)
{
    if(screen->getStatus() == Screen::SELECT){
        screen->setStatus(Screen::MOV);
    }else{
        //拖动完成将鼠标改为箭头
        this->setCursor(Qt::ArrowCursor);
    }
}

void ScreenWidget::contextMenuEvent(QContextMenuEvent *)
{
    //设置鼠标箭头，保证鼠标正常显示
    this->setCursor(Qt::ArrowCursor);
    //在鼠标指针的当前位置显示菜单，并等待用户选择菜单项;cursor().pos() 获取鼠标指针的当前位置，以确保菜单出现在用户右键点击的位置附近
    menu->exec(cursor().pos());
}

void ScreenWidget::paintEvent(QPaintEvent *)
{
    int x = screen->getLeftUp().x();
    int y = screen->getLeftUp().y();
    int w = screen->getRightDown().x()-x;
    int h = screen->getRightDown().y()-y;
//    qDebug() << x << y << w << h;

    QPainter painter(this);
    QPen pen;
    pen.setColor(Qt::green);
    pen.setWidth(2);
    //虚线绘制
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);
    //绘制背景图像*bgScreen
    painter.drawPixmap(0,0,*bgScreen);

    if(w != 0 && h !=0){
        painter.drawPixmap(x,y,fullScreen->copy(x,y,w,h));
    }

    //绘制矩形边框
    painter.drawRect(x,y,w,h);

    pen.setColor(Qt::yellow);
    painter.setPen(pen);
    painter.drawText(x+2,y-8,tr("截图范围:  (%1 x %2) - (%3 x %4)  图片大小:  (%5 x %6)")
                    .arg(x).arg(y).arg(x+w).arg(y+h).arg(w).arg(h));
}

void ScreenWidget::showEvent(QShowEvent *)
{
    QPoint point(-1,-1);
    screen->setStart(point);
    screen->setEnd(point);
//初始化全屏大小
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    QScreen *pscreen = QApplication::primaryScreen();
    *fullScreen = pscreen->grabWindow(0, 0, 0, screen->getmMxWidth(), screen->getMaxHeight());
#else
    *fullScreen = fullScreen->grabWindow(0, 0, 0, screen->getmMxWidth(), screen->getMaxHeight());
#endif

    //设置透明度实现模糊背景
    QPixmap pix(screen->getmMxWidth(),screen->getMaxHeight());
    pix.fill((QColor(160,160,160,200)));
    bgScreen = new QPixmap(*fullScreen);
    QPainter p(bgScreen);
    p.drawPixmap(0,0,pix);
}

void ScreenWidget::saveScreen()
{
    //根据一个点和宽高即可绘制截图
    int x = screen->getLeftUp().x();
    int y = screen->getLeftUp().y();
    int w = screen->getRightDown().x()-x;
    int h = screen->getRightDown().y()-y;

    //保存到此电脑->图片路径中
    QString filename = QString("%1/screen_%2.png")
                           .arg(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                           .arg(STRDATETIME);
    fullScreen->copy(x,y,w,h).save(filename,"png");
    QMessageBox::information(this,"保存成功","截图已保存至此电脑->图片中");
    close();
}

void ScreenWidget::saveFullScreen()
{
    QString filename = QString("%1/screen_%2.png")
                           .arg(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                           .arg(STRDATETIME);
    fullScreen->save(filename,"png");
    QMessageBox::information(this,"保存成功","截图已保存至此电脑->图片中");
    close();
}

void ScreenWidget::saveScreenOth()
{
    QString name = QString("%1.png").arg(STRDATETIME);
    QString filename = QFileDialog::getSaveFileName(this,"保存图片",name,"png files(*.png)");
    //后缀检查
    if(!filename.endsWith(".png")){
        filename += ".png";
    }

    if(filename.length() > 0){
        int x = screen->getLeftUp().x();
        int y = screen->getLeftUp().y();
        int w = screen->getRightDown().x()-x;
        int h = screen->getRightDown().y()-y;
        fullScreen->copy(x,y,w,h).save(filename,"png");
        QMessageBox::information(this,"保存成功","截图已保存");
        close();
    }

}

void ScreenWidget::saveFullScreenOth()
{
    QString name = QString("%1.png").arg(STRDATETIME);
    QString filename = QFileDialog::getSaveFileName(this,"保存图片",name,"png files(*.png)");
    if(!filename.endsWith(".png")){
        filename += ".png";
    }

    if(filename.length() > 0){
        fullScreen->save(filename,"png");
        QMessageBox::information(this,"保存成功","截图已保存");
        close();
    }
}

Screen::Screen(QSize size)
{
    //初始化为全屏大小
    maxWidth = size.width();
    maxHeight = size.height();

    startPos = QPoint(-1,-1);
    endPos = startPos;
    leftUp = startPos;
    rightDown = startPos;
    status = SELECT;
}

void Screen::setStart(QPoint pos)
{
    this->startPos = pos;
}

void Screen::setEnd(QPoint pos)
{
    //选择截图完成，更改数据
    endPos = pos;
    leftUp = startPos;
    rightDown = endPos;
    compare(leftUp,rightDown);
}

void Screen::setStatus(STATUS status)
{
    this->status = status;
}

QPoint Screen::getStart()
{
    return startPos;
}

QPoint Screen::getEnd()
{
    return endPos;
}

QPoint Screen::getLeftUp()
{
    return leftUp;
}

QPoint Screen::getRightDown()
{
    return rightDown;
}

Screen::STATUS Screen::getStatus()
{
    return status;
}

int Screen::getmMxWidth()
{
    return maxWidth;
}

int Screen::getMaxHeight()
{
    return maxHeight;
}

bool Screen::isInArea(QPoint pos)
{
    //判断点击的位置是否在截图框内
    if(pos.x() > leftUp.x() && pos.x() < rightDown.x() && pos.y() > leftUp.y() && pos.y() < rightDown.y())
    {
        return true;
    }
    return false;
}
//将偏移量相加到各点中
void Screen::move(QPoint pos)
{
    int lx = leftUp.x()+pos.x();
    int ly = leftUp.y()+pos.y();
    int rx = rightDown.x()+pos.x();
    int ry = rightDown.y()+pos.y();

    //边界判断
    if(lx < 0){
        lx = 0;
        rx -= pos.x();
    }
    if(ly < 0){
        ly = 0;
        ry -= pos.y();
    }
    if(rx > maxWidth){
        rx = maxWidth;
        lx -= pos.x();
    }
    if(ry > maxHeight){
        rx = maxHeight;
        ly -= pos.x();
    }

    leftUp = QPoint(lx,ly);
    rightDown = QPoint(rx,ry);
    startPos = leftUp;
    endPos = rightDown;
}

void Screen::compare(QPoint &leftUp, QPoint &rightDown)
{
    QPoint l = leftUp;
    QPoint r = rightDown;
    //l在左，r在右
    if(l.x() <= r.x())
    {
        //l在上，r在下，无需调整
        if(l.y() <= r.y()){
        }else{
            leftUp.setY(r.y());
            rightDown.setY(l.y());
        }
    }else{
        if(l.y() < r.y()){
            leftUp.setX(r.x());
            rightDown.setX(l.x());
        }else{
            //l和r的横纵坐标完全颠倒，互换两个坐标
            QPoint tmp;
            tmp = leftUp;
            leftUp = rightDown;
            rightDown = tmp;
        }
    }
}
