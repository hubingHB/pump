#ifndef CENTRALCTL_H
#define CENTRALCTL_H

#include <QWidget>
#include <QPixmap>
#include <QIcon>

#include "DefineDb.h"

#include "measure.h"
#include "constant.h"
#include "control.h"
#include "fault.h"
#include "audioplay.h"
#include "realtimefault.h"
#include "widgetmain.h"
#include "usrlogin.h"
#include "audioplay.h"
#include <QTimer>
#include "modbustcpserver.h"
#include "SerialCommu.h"
#include "pumpautorun.h"

namespace Ui {
class centralCtl;
}

class centralCtl : public QWidget
{
    Q_OBJECT

public:
    explicit centralCtl(QWidget *parent = 0);
    ~centralCtl();

    void cfgdebug();
    void protocollistdebug();

    DETAIL_VALUE testfaultinfo;

    int mBootProgress = 0;

    WidgetMain widgetmainIns;
    Measure    measureIns;
    Constant   constantIns;
    Control    controlIns;
    realtimefault realtimefaultIns;
    Fault      faultIns;

    usrlogin usrloginIns;
    pumpautorun pumpautorunIns;

    Audioplay AudioplayIns;
    modbustcpserver modbustcpserverInsA;
    modbustcpserver modbustcpserverInsB;
    SerialCommu SerialCommuIns[MAX_COM_NUM];

    void InitGlobalPointer();

    void ReadFromsysCfgfile();

    void readfromstructfile();


    void ReadDetailProto();
    void ReadFromProtoFile(char *filename,INT32S ProtoIndex,INT32S ProtoVer);
    void ReadFromProtoListFile();
    void InitMeaureEveryStartAddr();//

    void ReadFromUsrListFile();

//    void readfaultsummaryfaultfile();
//    void readfaultfile(int filenodeid);
//    void readfaultfile(int fileScode);

//    void savefaultsummaryfaultfile();
//    void savefaulttofaultfile();
//    void savetoonefaultfile(int fileScode,int currentScode,int newfaultnum);
//    void savetodoublefaultfile(int fileScode, int currentScode, int newfaultnum);
    void updatefaultlist();

public slots:
    void MeasureInsDisplayslot();
    void ConstantInsDisplayslot();
    void controlInsDisplayslot();
    void autorunInsDisplayslot();

    void sysconfigDisplayslot();

    void faultHistoryDisplayslot();
    void faultrealtimeDisplayslot();

    void PowerDownStampDisplayslot();

    void PowerDownStampSetupslot();

    void RealtimeFaultDisplayslot(int addr, DETAIL_VALUE*value);

    void usrloginDisplayslot();

    void usrpasswdmodifyDisplayslot();

    void managerloginslot();

    void confirmsaveusrinfoslot();

   // void modifypasswdslot();

  //  void debugtest();
  //  void debugtimeoutslot();
  //  void debugtestxmlslot();

    void savetimertimeout();
    void SysBootTimeoutSlot();

private:
    Ui::centralCtl *ui;

    QTimer mBootTimer;

    QTimer savetimer;

};

#endif // CENTRALCTL_H
