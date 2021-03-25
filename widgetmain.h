#ifndef WIDGETMAIN_H
#define WIDGETMAIN_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "DefineDb.h"
//#include "basiccommu.h"
//#include "communication.h"
//#include "measure.h"
//#include "constant.h"
//#include "control.h"
//#include "operate.h"
//#include "fault.h"
//#include "audioplay.h"
//#include "sysconfig.h"
#include <QFrame>
#include <QLabel>
#include <QIcon>

#define FLAG_DISPLAY_COMMU    0
#define FLAG_DISPLAY_STRUCT   1

namespace Ui {
    class WidgetMain;
}

class WidgetMain : public QWidget {
    Q_OBJECT
public:
    WidgetMain(QWidget *parent = 0);
    ~WidgetMain();

//    CONFIGURE_FILE_CONTENT * pvalid_cfg_info;
//    sysStruct              * psystemStruct;
//    ProtocolList           * pProtocollistIns;

//    ALL_TYPE_VALUE         * pRealtimeData;

    Deivce_DataBase *pDeviceDataBase;
    int mFunCode = 1;

//     void initIconArray();



protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *e);

signals:
//    void MeasureInsDisplaysignal(int LogicAddr);
//    void ConstantInsDisplaysignal(int LogicAddr);
//    void controlInsDisplaysignal(int LogicAddr);
    void sysconfigDisplaysignal();

    void faulthistoryDisplaysignal();
    void faultrealtimeDisplaysignal();

    void PowerDownStampSetupsignal();
    void PowerDownStampDisplaysignal();

    void usrloginDisplaysignal();

    void measureInsDisplaysignal();
    void constantInsDisplaysignal();
    void controlInsDisplaysignal();
    void autorunInsDisplaysignal();
    void setfileupdate();

public:
    Ui::WidgetMain *ui;

    int checktimes;
    int checkindex = 0;
    int *psysMode;


    u8 deviceprotoisIntern[MAX_RTU_NUM]; // proto is intern or not.

    INT32S mDisPlayFlag = FLAG_DISPLAY_COMMU;

    int structinitflag = 0;

//    int currentLogicaddr=0;


    int getdevicestatus(int logicaddr);
    int pointerinit();

    void ui_init();
    void ui_display_info();

    QTimer mClockTimer;

    void key_f1_press();
    void key_f2_press();
    void key_f3_press();
    void key_up_press();
    void key_down_press();
    void key_left_press();
    void key_right_press();
    void key_return_press();
    void select_dev(int last,int now);
    int handle_value_adjust(float value);

    // 9 mainfunction
    void user_loginprocess();
    void modeswitchprocess();
    void sysviewswitchprocess();

    void ui_commu_show(int LogicAddr);
    void ui_operate_show(int LogicAddr);
    void ui_sys_config_show();

    void operamodeswitchprocess();
    void ui_autorun_show();
    void ui_measure_show();
    void ui_constant_show();
    void ui_control_show();
    void ui_soe_show();

    void displayBZ1();
    void displayBZ2();
    void displayBZ3();
    void displayBZ4();
    void displayBF1();

    void labeldisplay1_bz1();
    void labeldisplay2_bz1();
    void labeldisplay3_bz1();

    void labeldisplay1_bz2();
    void labeldisplay2_bz2();
    void labeldisplay3_bz2();

    void labeldisplay1_bz3();
    void labeldisplay2_bz3();
    void labeldisplay3_bz3();

    void labeldisplay1_bz4();
    void labeldisplay2_bz4();
    void labeldisplay3_bz4();


    int waterout=0,waterout1=0,waterout2=0,waterout3=0;
    int pipe =0;int pipe1 =0;int pipe2 =0;int pipe3 =0;

private slots:
    void DisplayTimeSlot();

    void ui_commu_update();
    void usrloginslot();
    void usrlogoutslot();

public slots:
    void remoteswitch_local_remote_slot(INT8U currentmode);
    void remoteswitch_manual_auto_slot(INT8U currentmode);

    //数据更新刷新界面
    void UpdateDevStatusSlot(INT8U LogicAddr,INT8U FrameType,INT8U GroupIndex);

    void setfilerelease();

};

#endif // WIDGETMAIN_H
