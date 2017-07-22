#include "canvas.h"
#include <QDebug>

//考虑继承QLabel
Canvas::Canvas(QWidget *parent) : QWidget(parent)
{
    okFlag=1;
    const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
    screen_width=screenGeometry.width();
    screen_height=screenGeometry.height();

    this->setWindowFlags(Qt::WindowStaysOnTopHint);     //窗口置顶
    this->showFullScreen();             //画布全屏显示

    btn_save=new QPushButton("完成",this);
    btn_save->setVisible(false);

    connect(btn_save,SIGNAL(clicked(bool)),this,SLOT(slt_saveFile()));
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        if(rectFlag==0)
        {
            start_x=end_x=event->x();
            start_y=end_y=event->y();
        }
        //update();
        dragFlag=0;
    }
    else if(event->button()==Qt::RightButton)
    {
        this->close();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons()&Qt::LeftButton)
    {
        if(rectFlag==0)
        {
            end_x=event->x();
            end_y=event->y();
            dragFlag=1;
        }
        else
        {

        }


        update();

    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        if(dragFlag==1)
        {
            if(rectFlag==0)
            {
                end_x=event->x();
                end_y=event->y();
                rectFlag=1;         //矩形绘制完成
                btn_save->setGeometry(end_x,end_y,50,20);
                btn_save->setVisible(true);
            }
        }

        update();
    }
}

void Canvas::paintEvent(QPaintEvent *e)
{
    if(start_x>end_x)
    {
        x1=end_x;
        x2=start_x;
    }
    else
    {
        x1=start_x;
        x2=end_x;
    }

    if(start_y>end_y)
    {
        y1=end_y;
        y2=start_y;
    }
    else
    {
        y1=start_y;
        y2=end_y;
    }

    QPainter painter(this);

    QPixmap tempmask(screen_width, screen_width);
    tempmask.fill((QColor(0, 0, 0, 160)));

    painter.drawPixmap(0,0,fullPixmap);     //先绘制全屏原图背景
    painter.drawPixmap(0,0,tempmask);       //然后绘制半透明背景，用来降低亮度
    painter.setPen(QPen(Qt::green,2,Qt::DashLine));//设置画笔形式
    //painter.setBrush(Qt::white);
    QRectF rectangle(x1,y1,(x2-x1),(y2-y1));
    painter.drawRect(rectangle);            //然后绘制矩形框
    painter.drawPixmap(rectangle,fullPixmap,rectangle);     //然后将矩形框中的半透明图像替换成原图

    if(rectFlag==1)     //绘图完成
    {
        shootScreen(x1,y1,(x2-x1),(y2-y1));
    }
    QWidget::paintEvent(e);
}

void Canvas::setbgPixmap(QPixmap &px)
{
    fullPixmap=px;
}

void Canvas::shootScreen(int x,int y,int width,int height)
{
    QScreen *screen = QGuiApplication::primaryScreen();

    originalPixmap = screen->grabWindow(0,x,y,width,height);

}

void Canvas::addToolButton()
{

}

void Canvas::deleteToolButton()
{

}

void Canvas::slt_saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save File"),
            ".",
            tr("JPEG File (*.jpg)"));
    originalPixmap.save(fileName,"jpg");

    this->deleteLater();
}

void Canvas::slt_saveClipboard()
{

}

void Canvas::slt_redo()
{

}
