#include "screenshottool.h"
#include "ui_screenshottool.h"

#include <QDebug>
#include <QSettings>


/*
 * Author:qiuzhiqian
 * Email:xia_mengliang@163.com
 * Github:https://github.com/qiuzhiqian
 * Date:2017.07.23
 **/

/*
 * 画图原理
 * 1\首先在全屏幕设置一个透明画布
 * 2\然后在透明画布上绘制矩形
 * 3\对画布上的矩形区域截图
 * 4\保存截图到文件夹或剪切板
 * 修正
 * 1\全屏截屏，同时生成一个暗色图片
 * 2\设置全屏画布，然后将暗色全屏图片显示到画布
 * 3\在暗色全屏图片上画矩形，矩形内显示亮色图片
 * 4\对矩形区域截图
 * 5\保存截图到文件夹或剪切板
 * */

/*
 * 截图区域调整策略
 * 左边--pointLT.x
 * 右边--pointRB.x
 * 上边--pointLT.y
 * 下边--pointRB.y
 *
 * 为简化操作，将角调整转化为同时两个边的调整
 * 左上--同时作用左边、上边
 * 右下--同时作用右边、下边
 * 左下--同时作用左边、下边
 * 右上--同时作用右边、上边
 * */

ScreenShotTool::ScreenShotTool(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScreenShotTool)
{
    ui->setupUi(this);

    QWidget * xparent = new QWidget;        //设置一个空的父对象，从而达到掩藏任务栏图标的目的

    setParent(xparent);

    this->setWindowFlags(Qt::Dialog|Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
    this->setWindowTitle(tr("设置"));
    this->setWindowIcon(QIcon(":/ss.png"));

    sc_set=new ShotCut(ui->groupBox);
    sc_set->setReadOnly(true);
    sc_set->setGeometry(90,25,113,20);
    connect(sc_set,SIGNAL(sgn_hotKeyChanged(Qt::Key,Qt::KeyboardModifiers)),this,SLOT(slt_changeShotCut(Qt::Key,Qt::KeyboardModifiers)));
    connect(ui->checkBox,SIGNAL(stateChanged(int)),this,SLOT(slt_auto_run(int)));

    this->setFixedSize(this->size());

    setShotCut();

    initTray();                             //托盘显示
}

ScreenShotTool::~ScreenShotTool()
{
    unregisterHotKey(key,mods);
    delete ui;
}

void ScreenShotTool::ss_start()             //开始截屏
{
    QScreen *screen = QGuiApplication::primaryScreen();
    fullPixmap = screen->grabWindow(0);

    screenCanvas=new Canvas(0);     //创建画布

    screenCanvas->setbgPixmap(fullPixmap);  //传递全屏背景图片
}

void ScreenShotTool::initTray()
{
    m_systemTray = new QSystemTrayIcon(this);
    m_systemTray->setIcon(QIcon(":/ss.png"));
    m_systemTray->setToolTip("ScreenShot");

    setAction=new QAction(tr("设置"),this);
    aboutAction=new QAction(tr("关于"),this);
    exitAction=new QAction(tr("退出"),this);

    QMenu *trayMenu=new QMenu(this);
    trayMenu->addAction(setAction);
    trayMenu->addAction(aboutAction);
    trayMenu->addSeparator();
    trayMenu->addAction(exitAction);

    m_systemTray->setContextMenu(trayMenu);

    m_systemTray->show();

    connect(m_systemTray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(slt_clickTray(QSystemTrayIcon::ActivationReason)));
    connect(setAction,SIGNAL(triggered(bool)),this,SLOT(slt_setAction()));
    connect(aboutAction,SIGNAL(triggered(bool)),this,SLOT(slt_aboutAction()));
    connect(exitAction,SIGNAL(triggered(bool)),this,SLOT(slt_exitAction()));
}

void ScreenShotTool::slt_clickTray(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
        //单击托盘图标,开始截图
        ss_start();
        break;
    case QSystemTrayIcon::DoubleClick:
        //双击托盘图标
        //双击后显示设置窗口
        this->show();
        break;
    default:
        break;
    }
}

void ScreenShotTool::slt_setAction()
{
    this->show();
}

void ScreenShotTool::slt_aboutAction()
{

}

void ScreenShotTool::slt_exitAction()
{
    closeFlag=true;
    this->close();
    QApplication::quit();       //程序退出
}

void ScreenShotTool::closeEvent(QCloseEvent *event)
{
    if(closeFlag==true)
    {
        event->accept();
    }
    else
    {
        this->hide();
        event->ignore();
    }
}

void ScreenShotTool::setShotCut()
{
    key=Qt::Key_F1;
    mods=Qt::ControlModifier;
    registerHotKey(key,mods);
}

bool ScreenShotTool::nativeEventFilter(const QByteArray &eventType, void *message, long *result)        //windows事件捕捉器
{
    if(eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        MSG* msg = reinterpret_cast<MSG*>(message);
        if(msg->message == WM_HOTKEY)
        {
            qDebug("wParam=%d,keycode=%d,modifiers=%d",msg->wParam,HIWORD(msg->lParam),LOWORD(msg->lParam));
            if(HIWORD(msg->lParam)==nativeKeycode(key)&&LOWORD(msg->lParam)==nativeModifiers(mods))
            {
                ss_start();
            }
        }
    }
    return false;           //该函数一定要返回false，不然程序会直接崩溃
}

quint32 ScreenShotTool::nativeKeycode(Qt::Key key)                      //key值转换
{
    switch (key)
    {
    case Qt::Key_Escape:
        return VK_ESCAPE;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        return VK_TAB;
    case Qt::Key_Backspace:
        return VK_BACK;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return VK_RETURN;
    case Qt::Key_Insert:
        return VK_INSERT;
    case Qt::Key_Delete:
        return VK_DELETE;
    case Qt::Key_Pause:
        return VK_PAUSE;
    case Qt::Key_Print:
        return VK_PRINT;
    case Qt::Key_Clear:
        return VK_CLEAR;
    case Qt::Key_Home:
        return VK_HOME;
    case Qt::Key_End:
        return VK_END;
    case Qt::Key_Left:
        return VK_LEFT;
    case Qt::Key_Up:
        return VK_UP;
    case Qt::Key_Right:
        return VK_RIGHT;
    case Qt::Key_Down:
        return VK_DOWN;
    case Qt::Key_PageUp:
        return VK_PRIOR;
    case Qt::Key_PageDown:
        return VK_NEXT;
    case Qt::Key_F1:
        return VK_F1;
    case Qt::Key_F2:
        return VK_F2;
    case Qt::Key_F3:
        return VK_F3;
    case Qt::Key_F4:
        return VK_F4;
    case Qt::Key_F5:
        return VK_F5;
    case Qt::Key_F6:
        return VK_F6;
    case Qt::Key_F7:
        return VK_F7;
    case Qt::Key_F8:
        return VK_F8;
    case Qt::Key_F9:
        return VK_F9;
    case Qt::Key_F10:
        return VK_F10;
    case Qt::Key_F11:
        return VK_F11;
    case Qt::Key_F12:
        return VK_F12;
    case Qt::Key_F13:
        return VK_F13;
    case Qt::Key_F14:
        return VK_F14;
    case Qt::Key_F15:
        return VK_F15;
    case Qt::Key_F16:
        return VK_F16;
    case Qt::Key_F17:
        return VK_F17;
    case Qt::Key_F18:
        return VK_F18;
    case Qt::Key_F19:
        return VK_F19;
    case Qt::Key_F20:
        return VK_F20;
    case Qt::Key_F21:
        return VK_F21;
    case Qt::Key_F22:
        return VK_F22;
    case Qt::Key_F23:
        return VK_F23;
    case Qt::Key_F24:
        return VK_F24;
    case Qt::Key_Space:
        return VK_SPACE;
    case Qt::Key_Asterisk:
        return VK_MULTIPLY;
    case Qt::Key_Plus:
        return VK_ADD;
    case Qt::Key_Comma:
        return VK_SEPARATOR;
    case Qt::Key_Minus:
        return VK_SUBTRACT;
    case Qt::Key_Slash:
        return VK_DIVIDE;
        // numbers
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        return key;
        // letters
    case Qt::Key_A:
    case Qt::Key_B:
    case Qt::Key_C:
    case Qt::Key_D:
    case Qt::Key_E:
    case Qt::Key_F:
    case Qt::Key_G:
    case Qt::Key_H:
    case Qt::Key_I:
    case Qt::Key_J:
    case Qt::Key_K:
    case Qt::Key_L:
    case Qt::Key_M:
    case Qt::Key_N:
    case Qt::Key_O:
    case Qt::Key_P:
    case Qt::Key_Q:
    case Qt::Key_R:
    case Qt::Key_S:
    case Qt::Key_T:
    case Qt::Key_U:
    case Qt::Key_V:
    case Qt::Key_W:
    case Qt::Key_X:
    case Qt::Key_Y:
    case Qt::Key_Z:
        return key;
    default:
        return 0;
    }
}
quint32 ScreenShotTool::nativeModifiers(Qt::KeyboardModifiers modifiers)                //mod值转换
{
    // MOD_ALT, MOD_CONTROL, (MOD_KEYUP), MOD_SHIFT, MOD_WIN
    quint32 native = 0;
    if (modifiers & Qt::ShiftModifier)
        native |= MOD_SHIFT;
    if (modifiers & Qt::ControlModifier)
        native |= MOD_CONTROL;
    if (modifiers & Qt::AltModifier)
        native |= MOD_ALT;
    if (modifiers & Qt::MetaModifier)
        native |= MOD_WIN;
    // TODO: resolve these?
    //if (modifiers & Qt::KeypadModifier)
    //if (modifiers & Qt::GroupSwitchModifier)
    return native;
}

bool  ScreenShotTool::registerHotKey(Qt::Key key,Qt::KeyboardModifiers modifiers)       //注册快捷键
{
    const quint32 nativeKey = nativeKeycode(key);
    const quint32 nativeMods = nativeModifiers(modifiers);
    qDebug("nativeKey=%d,nativeMods=%d",nativeKey,nativeMods);
    return RegisterHotKey(0, nativeMods ^ nativeKey, nativeMods, nativeKey);
}
bool  ScreenShotTool::unregisterHotKey(Qt::Key key,Qt::KeyboardModifiers modifiers)     //注销快捷键
{
    return UnregisterHotKey(0, (quint32)nativeModifiers(modifiers) ^ (quint32)nativeKeycode(key));
}

void ScreenShotTool::slt_changeShotCut(Qt::Key t_key, Qt::KeyboardModifiers t_mod)      //快捷键修改
{
    qDebug("change shotcut");
    unregisterHotKey(key,mods);
    key=t_key;
    mods=t_mod;
    registerHotKey(key,mods);
}

void ScreenShotTool::slt_auto_run(int states)
{
    QSettings *reg=new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    if(states == Qt::Checked)
    {
        reg->setValue("app",QApplication::applicationFilePath());
    }
    else
    {
        reg->setValue("app","");
    }
}
