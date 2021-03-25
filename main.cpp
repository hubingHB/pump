/************************************************************
** 版权所有 (C)2015上海颐坤自动化设备有限公司系统部
** 项目名称: 本安电力保护分站
*************************************************************
** 文件名称:
** 文件标识:
** 摘要:
*************************************************************
** 作者：lann
** 版本信息:V3.0
** 完成日期:2016-1-1
** 暂时不做停电票界面，故障语音未完成
*************************************************************
** 作者：lann
** 版本信息:V3.0.1
** 完成日期:2016-3-22
** 暂时不做停电票界面，更新定值界面，最大支持2组定值
*************************************************************
** 作者：lann
** 版本信息:V3.1
** 完成日期:2016-4-19
** 停电票状态隐藏
*************************************************************
** 作者：lann
** 版本信息:V3.4
** 完成日期:2016-10-28
** 界面增加已授时提示 分站得到后台的时间才显示
** 定值修改/操作 由两步改为一步
*************************************************************
** 作者：lann
** 版本信息:V3.4.1
** 完成日期:2017-07-17
** 修改界面显示BUG
*************************************************************
** 作者：lann
** 版本信息:V3.5.1
** 完成日期:2017-12-20
** 修改界面光标，删除实时故障控件
*************************************************************/

#include <QApplication>
#include "widgetmain.h"
#include <QTextCodec>
#include <QKeyEvent>
#include <QDateTime>
#include <QDebug>
#include <QTime>
#include "centralctl.h"
#include <QMetaType>
#include <DefineDb.h>
//过滤F3 变为小数点
class NavigationKeyFilter:public QObject
{
    bool eventFilter(QObject *object, QEvent *event)
    {
        int key;
        switch(event->type())
        {
            // 只需要过滤按键事件
            case QEvent::KeyPress:
                // 对QEvent进行强制转换，并获得键值
                key = (static_cast<QKeyEvent *>(event))->key();
                // 需要过滤的按键
                if (key == Qt::Key_F3 || key == Qt::Key_F6)
                {
                    QKeyEvent  keyevent(QEvent::KeyPress,Qt::Key_Period,Qt::NoModifier,".", 0);
                    QApplication::sendEvent(object,&keyevent);
                    return true;
                }
#if 1
		break;
#endif
            case QEvent::KeyRelease:
#if 1
		break;
#endif
            default:
                return false;
        }
        return false;
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    static const QDate buildDate = QLocale( QLocale::English ).toDate( QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
//    static const QTime buildTime = QTime::fromString(__TIME__, "hh:mm:ss");
//    qDebug() << buildDate.toString("yyyy.MM.dd");    qDebug() << buildTime.toString();
//    qDebug() << QString("%1 %2").arg(buildDate.toString("yyyy.MM.dd")).arg(buildTime.toString());

    // 创建过滤器并安装给QApplication
  //  NavigationKeyFilter filter;
   // a.installEventFilter(&filter);
#if 0
    //设置字体
  //  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));//"UTF-8"
 //   QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
  //  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#else
  //  QFont f=QApplication::font();
  //  f.setStyleStrategy(QFont::PreferAntialias);
 //   QApplication::setFont(f);
#endif
    qRegisterMetaType<INT8U>("INT8U");

  //  WidgetMain w;
  //  w.show();
    centralCtl centralINS;
    centralINS.show();
    return a.exec();
}
