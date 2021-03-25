#ifndef CONTROL_H
#define CONTROL_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "DefineDb.h"
#include <QMutex>
#include <QQueue>
namespace Ui {
    class Control;
}

class Control : public QWidget {
    Q_OBJECT
public:
    Control(QWidget *parent = 0);
    ~Control();
    void EnterProcess();
    void DisplayControlUI(INT32S LogicAddr);

    void key_f1_press();
    void key_f2_press();
    void key_f3_press();
    void key_return_press();
    void key_escape_press();

    void HandleReset();
    void HandleRelayOn();
    void HandleRelayOff();
    void HandleTest();

    void HandleControl(INT8U Channel,int logicaddr);

    Deivce_DataBase *pDeviceDataBase;
    QQueue<QUEUE_MSG> *pMsgQueue[MAX_COM_NUM];
    QMutex *pMsgQueueMutex[MAX_COM_NUM];


    int selecttype    = 0;
    int selectswitch  = 0;
    int selectDev     = 0;
    int selectOpt     = 0;
    int logic_Addr    = 0;
    int rewaterpump = 0 ;
    int add_r=0;int add_s=0;
    int pumpswitchnum =0;
    int control_step = 0;
    int feedtest = 0;
    int optaddr =0;

    QString tipstr;

    void display1();
    void display2();
    void display3();
    void operatypedisplay();
    void pumpswitchdeal();
    void selectDevfeeddisplay();
    void selectOptfeeddisplay();
    void feedsupdate();

    void pumpswitchconfim(int switchnum,QString tip);
    void handleOperation(int logicaddr, int step);


public slots:
    void UpdateControlCoilSlot(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U ChannelIndex);
    void UpdateDevStatusSlot(INT8U LogicAddr,INT8U FrameType,INT8U GroupIndex);

    void remoteoneswitch_signal(INT8U onekeyswitch);

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *e);
private:
    Ui::Control *ui;
    INT32S mLogicAddr;
    INT8U mSelectCode;
    INT8S mDevStatusIndex;
    INT8S mProtocolIndex;
    INT8S mDevStatus;
    INT8S mErrorStatusIndex;
    INT8S mErrorTotal;
    INT8S mErrorStatus;
};

#endif // CONTROL_H
