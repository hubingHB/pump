#include "centralctl.h"
#include "ui_centralctl.h"
#include "DefineDb.h"
#include "tinyxml2.h"
#include <stdlib.h>
#include <QDebug>

#include "stdio.h"

#include <QDateTime>

#include "modbustcpserver.h"
#include <QMutex>
#include <QReadWriteLock>

using namespace tinyxml2;
/*all config file info  here*/
/*all data process here and pass to other instance with pointer*/
//static CONFIGURE_FILE_CONTENT valid_cfg_info; // used in hmi

/*system struct picture*/
static sysStruct systemStruct;//struct  TODO

/* protocol list*/
//static ProtocolList ProtocollistIns;

/*this config file will share to other process*/
//static simpleCONFIGURE_FILE_CONTENT simplevalid_cfg_info;

/*all data here and other class will use the pointer which point here */
//static ALL_TYPE_VALUE valid_all_value[MAX_RTU_NUM];
//static ALL_TYPE_VALUE DeviceDataBase[MAX_RTU_NUM];

static Deivce_DataBase gDeviceRunData;
static Deivce_DataBase gDeviceDataBase;


static STUsrList TestValue;

//static ALL_TYPE_VALUE DeviceRunningDataValue[MAX_RTU_NUM]; //global save every device's current value.

static QList<storefaultSOEinfo>  faultRealTimelist;
static QList<storefaultSOEinfo>  faultHisTorylist;
static QList<storefaultSOEinfo>  faultDisplaylist;
static QList<storefaultSOEinfo>  faultSavebufferlist;
static HistoryFaultSummary       HistoryFaultSummaryIns;

static QQueue<QUEUE_MSG> ExchangMsgQueue[MAX_COM_NUM];
static QMutex    MsgQMutex[MAX_COM_NUM];
static QReadWriteLock RWLocker[MAX_COM_NUM];


XMLDocument structdoc;

centralCtl::centralCtl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::centralCtl)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    mBootTimer.start(500);
    connect(&mBootTimer,SIGNAL(timeout()),this,SLOT(SysBootTimeoutSlot()));
//静态变量会自动化初始化为0
//  memset(&gDeviceDataBase,0x00,sizeof(Deivce_DataBase));


    memset(&systemStruct,0x00,sizeof(sysStruct));

    InitGlobalPointer();

#if ARM_PATHFORM
    this->AudioplayIns.audio_init();
    this->AudioplayIns.play_welcome();
#endif

    memset(&gDeviceDataBase.SysCfgData,0x00,sizeof(STCFG_FILE_INFO));
    qDebug()<<"size = "<<sizeof(Deivce_DataBase);

//读取串口接线配置文件
    ReadFromsysCfgfile();
//读取电气接线配置文件
    readfromstructfile();

    ReadFromProtoListFile();

//#ifdef HBDEBUG
//    protocollistdebug();
//#endif

   // readfaultsummaryfaultfile();
   // this->realtimefaultIns.initfaultIndex();

    ReadDetailProto();

//    cfgdebug();
    ReadFromUsrListFile();

    for(INT32S i = 0;i < MAX_COM_NUM; i++)
    {
        SerialCommuIns[i].SetCurrentPort(i); //设置串口编号
        if(gDeviceDataBase.SysCfgData.OneComDevTotal[i] > 0) //串口接了设备才启动
            SerialCommuIns[i].start();
    }

    for(INT32S i = 0;i < MAX_COM_NUM; i++)
    {
        constantIns.pMsgQueue[i]  = &ExchangMsgQueue[i];
        constantIns.pMsgQMutex[i] = &MsgQMutex[i];
    }

    for(INT32S i = 0; i < MAX_COM_NUM; i++)
    {
        modbustcpserverInsA.pMsgQueue[i]  = &ExchangMsgQueue[i];
        modbustcpserverInsA.pMsgQMutex[i] = &MsgQMutex[i];
        modbustcpserverInsA.pRWLocker[i]  = &RWLocker[i];

        modbustcpserverInsB.pMsgQueue[i]  = &ExchangMsgQueue[i];
        modbustcpserverInsB.pMsgQMutex[i] = &MsgQMutex[i];
        modbustcpserverInsB.pRWLocker[i]  = &RWLocker[i];
    }

    modbustcpserverInsA.server_port = 10001;
    modbustcpserverInsB.server_port = 10002;

    modbustcpserverInsA.initserver();
    modbustcpserverInsB.initserver();

    // 查看当前总共多少设备，并将所有设备按照配置文件之中的顺序排列起来供上位机点表使用
    this->modbustcpserverInsA.initPointEdgeLogicAddrArray();
    this->modbustcpserverInsB.initPointEdgeLogicAddrArray();

    modbustcpserverInsA.start();
    modbustcpserverInsB.start();


    this->widgetmainIns.ui_display_info();

//切换到各个界面
    connect(&this->widgetmainIns,SIGNAL(measureInsDisplaysignal()),this,SLOT(MeasureInsDisplayslot()));
    connect(&this->widgetmainIns,SIGNAL(constantInsDisplaysignal()),this,SLOT(ConstantInsDisplayslot()));
    connect(&this->widgetmainIns,SIGNAL(controlInsDisplaysignal()),this,SLOT(controlInsDisplayslot()));
    connect(&this->widgetmainIns,SIGNAL(autorunInsDisplaysignal()),this,SLOT(autorunInsDisplayslot()));

//    connect(&commuIns,SIGNAL(valid_shm_value(ALL_TYPE_VALUE*)),&widgetmainIns,SLOT(get_valid_value(ALL_TYPE_VALUE*)));

   // connect(&widgetmainIns,SIGNAL(debugvaluesignal()),this,SLOT(debugtest()));

   // connect(&measureIns,SIGNAL(update_value(int)),&measureIns,SLOT(update_displayValueslot(int)));

//    connect(&constantIns,SIGNAL(constant_info(int,int,int,int,u32*,u8)),&commuIns,SLOT(msgsetting_tocom(int,int,int,int,u32*,u8)));
//    connect(&commuIns,SIGNAL(snd_result(int,int)),&constantIns,SLOT(handle_snd_result(int,int)));
//    connect(&commuIns,SIGNAL(rcv_result(int,int,u8,u32*)),&constantIns,SLOT(handle_rcv_result(int,int,u8,u32*)));


    for(INT32S i = 0; i < MAX_COM_NUM;i++)
    {
        //实时数据
        connect(&SerialCommuIns[i],SIGNAL(UpdateDevStatusSignal(INT8U,INT8U,INT8U)),&widgetmainIns,SLOT(UpdateDevStatusSlot(INT8U,INT8U,INT8U)));
        connect(&SerialCommuIns[i],SIGNAL(UpdateDevStatusSignal(INT8U,INT8U,INT8U)),&measureIns,SLOT(UpdateDevStatusSlot(INT8U,INT8U,INT8U)));
        connect(&SerialCommuIns[i],SIGNAL(UpdateDevStatusSignal(INT8U,INT8U,INT8U)),&controlIns,SLOT(UpdateDevStatusSlot(INT8U,INT8U,INT8U)));

        //定值管理
        connect(&SerialCommuIns[i],SIGNAL(UpdateDevConstantSignal(INT8U,INT8U,INT8U,INT8U)),&constantIns,SLOT(UpdateDevConstantSlot(INT8U,INT8U,INT8U,INT8U)));
        //控制
        connect(&SerialCommuIns[i],SIGNAL(UpdateControlCoilSignal(INT8U,INT8U,INT8U,INT8U)),&controlIns,SLOT(UpdateControlCoilSlot(INT8U,INT8U,INT8U,INT8U)));
        //远程定值
        connect(&SerialCommuIns[i],SIGNAL(UpdateDevConstantSignal(INT8U,INT8U,INT8U,INT8U)),&modbustcpserverInsA,SLOT(UpdateDevConstantSlot(INT8U,INT8U,INT8U,INT8U)));
        connect(&SerialCommuIns[i],SIGNAL(UpdateDevConstantSignal(INT8U,INT8U,INT8U,INT8U)),&modbustcpserverInsB,SLOT(UpdateDevConstantSlot(INT8U,INT8U,INT8U,INT8U)));
        //远程控制
        connect(&SerialCommuIns[i],SIGNAL(UpdateControlCoilSignal(INT8U,INT8U,INT8U,INT8U)),&modbustcpserverInsA,SLOT(UpdateControlCoilSlot(INT8U,INT8U,INT8U,INT8U)));
        connect(&SerialCommuIns[i],SIGNAL(UpdateControlCoilSignal(INT8U,INT8U,INT8U,INT8U)),&modbustcpserverInsB,SLOT(UpdateControlCoilSlot(INT8U,INT8U,INT8U,INT8U)));
    }


    //故障处理
  //  connect(&commuIns,SIGNAL(rcv_soe_info(int,DETAIL_VALUE*)),this,SLOT(RealtimeFaultDisplayslot(int,DETAIL_VALUE*)));
  //  connect(&this->widgetmainIns,SIGNAL(faulthistoryDisplaysignal()),this,SLOT(faultHistoryDisplayslot()));
  //  connect(&widgetmainIns,SIGNAL(faultrealtimeDisplaysignal()),this,SLOT(faultrealtimeDisplayslot()));
    //connect(&this->realtimefaultIns,SIGNAL(savehistoryfaultsignal()),&this->faultIns,SLOT(savehistoryfaultslot()));

    //停电票操作
//    connect(&this->widgetmainIns,SIGNAL(PowerDownStampDisplaysignal()),this,SLOT(PowerDownStampDisplayslot()));
//    connect(&widgetmainIns,SIGNAL(PowerDownStampDisplaysignal()),this,SLOT(PowerDownStampDisplayslot()));
//    connect(&widgetmainIns,SIGNAL(PowerDownStampSetupsignal()),this,SLOT(PowerDownStampSetupslot()));

    //monitor
  //  connect(&this->widgetmainIns,SIGNAL(nointersignal(int)),&nointerwarningIns,SLOT(nointerslot(int)));

    //login
     connect(&widgetmainIns,SIGNAL(usrloginDisplaysignal()),this,SLOT(usrloginDisplayslot()));

//    connect(&poweruserloginIns,SIGNAL(managerloginsignal()),this,SLOT(managerloginslot()));

//    connect(&powerusrmanagerIns,SIGNAL(confirmsaveusrinfosignal()),this,SLOT(confirmsaveusrinfoslot()));

    connect(&usrloginIns,SIGNAL(usrloginsignal()),&widgetmainIns,SLOT(usrloginslot()));
    connect(&usrloginIns,SIGNAL(usrlogoutsignal()),&widgetmainIns,SLOT(usrlogoutslot()));

//    connect(&poweruserloginIns,SIGNAL(usrpasswdmodifyDisplaysignal()),this,SLOT(usrpasswdmodifyDisplayslot()));

//    connect(&powerusrmodifypasswdIns,SIGNAL(modifypasswdsignal()),this,SLOT(confirmsaveusrinfoslot()));

   // connect(&widgetmainIns.testTimer,SIGNAL(timeout()),this,SLOT(debugtimeoutslot()));

  //  connect(&widgetmainIns,SIGNAL(debugtestxmlsignal()),this,SLOT(debugtestxmlslot()));

  //  connect(&widgetmainIns,SIGNAL(testsndERRMSGsignal()),&commuIns,SLOT(testsndERRMSGslot()));

    //上位机 控制
//    connect(&modbustcpserverInsA,SIGNAL(remote_constant_info(int,int,int,unsigned int*,unsigned char)),&commuIns,SLOT(remoteWriteConstantInfo(int,int,int,unsigned int*,unsigned char)));
    connect(&modbustcpserverInsA,SIGNAL(remote_switch_local_remote_signal(INT8U)),&widgetmainIns,SLOT(remoteswitch_local_remote_slot(INT8U)));
    connect(&modbustcpserverInsB,SIGNAL(remote_switch_local_remote_signal(INT8U)),&widgetmainIns,SLOT(remoteswitch_local_remote_slot(INT8U)));

    connect(&modbustcpserverInsA,SIGNAL(remote_auto_manual_signal(INT8U)),&widgetmainIns,SLOT(remoteswitch_manual_auto_slot(INT8U)));
    connect(&modbustcpserverInsB,SIGNAL(remote_auto_manual_signal(INT8U)),&widgetmainIns,SLOT(remoteswitch_manual_auto_slot(INT8U)));

    connect(&modbustcpserverInsA,SIGNAL(remote_switch_onekey_switch_signal(INT8U)),&controlIns,SLOT(remoteoneswitch_signal(INT8U)));
    connect(&modbustcpserverInsB,SIGNAL(remote_switch_onekey_switch_signal(INT8U)),&controlIns,SLOT(remoteoneswitch_signal(INT8U)));

#if 0
    //通信数据
    connect(&ui_commu,SIGNAL(update_value()),&widgetmainIns,SLOT(ui_commu_update()));

    //停电票操作
    //时间已授时
    connect(&commu,SIGNAL(rcv_server_info()),&widgetmainIns,SLOT(ui_handle_server()));
#endif

}

centralCtl::~centralCtl()
{
    delete ui;
}

void centralCtl::InitGlobalPointer()
{
    INT32S tCnt = 0;

    widgetmainIns.pDeviceDataBase = &gDeviceDataBase;
    measureIns.pDeviceDataBase    = &gDeviceDataBase;
    constantIns.pDeviceDataBase   = &gDeviceDataBase;
    controlIns.pDeviceDataBase    = &gDeviceDataBase;

    for(tCnt = 0;tCnt < MAX_COM_NUM; tCnt++)
    {
        controlIns.pMsgQueue[tCnt] = &(ExchangMsgQueue[tCnt]);
        controlIns.pMsgQueueMutex[tCnt] = &(MsgQMutex[tCnt]);

        constantIns.pMsgQueue[tCnt] = &(ExchangMsgQueue[tCnt]);
        constantIns.pMsgQMutex[tCnt] = &(MsgQMutex[tCnt]);

        SerialCommuIns[tCnt].pMsgQueue = &(ExchangMsgQueue[tCnt]);
        SerialCommuIns[tCnt].pMsgQueueMutex =  &(MsgQMutex[tCnt]);
        SerialCommuIns[tCnt].pRWLocker = &(RWLocker[tCnt]);
        SerialCommuIns[tCnt].pDeviceDataBase = &gDeviceDataBase;

        SerialCommuIns[tCnt].ModbusIns.pDeviceDataBase = &gDeviceDataBase;
//        SerialCommuIns[tCnt].Serial8330INS.pDeviceDataBase = &gDeviceDataBase;

        modbustcpserverInsA.pMsgQMutex[tCnt] = &(MsgQMutex[tCnt]);
        modbustcpserverInsA.pMsgQueue[tCnt] = &(ExchangMsgQueue[tCnt]);
        modbustcpserverInsA.pRWLocker[tCnt] = &(RWLocker[tCnt]);

        modbustcpserverInsB.pMsgQMutex[tCnt] = &(MsgQMutex[tCnt]);
        modbustcpserverInsB.pMsgQueue[tCnt] = &(ExchangMsgQueue[tCnt]);
        modbustcpserverInsB.pRWLocker[tCnt] = &(RWLocker[tCnt]);

    }

    faultIns.pDeviceDataBase = &gDeviceDataBase;

    realtimefaultIns.pDeviceDataBase = &gDeviceDataBase;

    usrloginIns.pDeviceDataBase =  &gDeviceDataBase;

    modbustcpserverInsA.pDeviceDataBase = &gDeviceDataBase;
    modbustcpserverInsB.pDeviceDataBase = &gDeviceDataBase;

}

//void centralCtl::debugtestxmlslot()
//{
//    storefaultSOEinfo storefaultSOEinfoins;
//    qDebug()<<__FUNCTION__;
//    storefaultSOEinfoins.time = tr("2020-01-01_19:00:00");
//    storefaultSOEinfoins.code = 2;
//    storefaultSOEinfoins.devname = tr("G111");
//    storefaultSOEinfoins.devtype = tr("PIR8110");
//    storefaultSOEinfoins.faultname = tr("短路故障");
//    faultSavebufferlist.append(storefaultSOEinfoins);
//    storefaultSOEinfoins.time = tr("2021-01-01_19:00:00");
//    faultSavebufferlist.append(storefaultSOEinfoins);
//    savetoonefaultfile(0,2,2);
//}


//void centralCtl::debugtimeoutslot()
//{
//  //  DETAIL_VALUE testfaultinfo;
//   // testfaultinfo.soe.code = 1;
//    testfaultinfo.soe.code++;
//    int phyaddr = 3;
//    testfaultinfo.value[8] = 2020;
//    testfaultinfo.value[9] = 8;
//    testfaultinfo.value[10] = 20;
//    testfaultinfo.value[11] = 10;
//    testfaultinfo.value[12] = 20;
//    testfaultinfo.value[13]++;

//    RealtimeFaultDisplayslot(phyaddr,&testfaultinfo);
//}

void centralCtl::SysBootTimeoutSlot()
{
      mBootProgress += 50;

      ui->progressBar->setValue(mBootProgress);

      if(mBootProgress == 100)
      {
          this->hide();
          this->widgetmainIns.show();
          mBootTimer.stop();
      }
}


//void centralCtl::debugtest()
//{
//    qDebug()<<"REAL TIME DATA DEBUG2"
//                <<valid_all_value[64].commu.status<<valid_all_value[64].commu.tx_byte
//                <<valid_all_value[64].commu.rx_byte
//                <<valid_all_value[64].commu.tx_frame<< valid_all_value[64].commu.rx_frame
//                <<valid_all_value[64].value[INFO_MEASURE_1].val_num<<INFO_MEASURE_1
//                <<valid_all_value[64].value[INFO_MEASURE_INTER_1].val_num<<INFO_MEASURE_INTER_1
//                <<valid_cfg_info.info[64].logic_addr<<valid_cfg_info.info[64].phy_addr;
//}

void centralCtl::cfgdebug()
{
    /*
    u8 Version;
    u8 protototal;           // protocol type total
    TYPE_INFO data[MAX_TYPE_NUM][MAX_VER_NUM]; // protocol info
    TYPE_INFO interdata[MAX_TYPE_NUM][MAX_VER_NUM];//inter protocol info
    */
    qDebug()<<"summary0"<<gDeviceDataBase.SysCfgData.StationName<<gDeviceDataBase.SysCfgData.Version
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[0][0].MeaureTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[0][0].MeasureInternTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[0][0].ConstantTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[0][0].FaulCodeGroupNum;

    qDebug()<<"summary1"<<gDeviceDataBase.SysCfgData.StationName<<gDeviceDataBase.SysCfgData.Version
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[1][0].MeaureTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[1][0].MeasureInternTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[1][0].ConstantTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[1][0].FaulCodeGroupNum;

    qDebug()<<"summary2"<<gDeviceDataBase.SysCfgData.StationName<<gDeviceDataBase.SysCfgData.Version
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[2][0].MeaureTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[2][0].MeasureInternTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[2][0].ConstantTotalNum
            <<gDeviceDataBase.SysCfgData.ProtoCfgData[2][0].FaulCodeGroupNum;

    for(int i = 0; i < MAX_COM_NUM; i++)
    {
        qDebug()<<"com param"
                <<gDeviceDataBase.SysCfgData.ComInfo[i].databits<<gDeviceDataBase.SysCfgData.ComInfo[i].parity
                <<gDeviceDataBase.SysCfgData.ComInfo[i].speed<<gDeviceDataBase.SysCfgData.ComInfo[i].stopbit;

        qDebug()<<"total"<<gDeviceDataBase.SysCfgData.OneComDevTotal[i];
    }

    for(int i=0;i<MAX_RTU_NUM;i++)
    {
        qDebug()<<"devinfo"
                <<gDeviceDataBase.SysCfgData.DevInfo[i].ad<<gDeviceDataBase.SysCfgData.DevInfo[i].com_id
                <<gDeviceDataBase.SysCfgData.DevInfo[i].logic_addr<<gDeviceDataBase.SysCfgData.DevInfo[i].devcode
                <<gDeviceDataBase.SysCfgData.DevInfo[i].phy_addr<<gDeviceDataBase.SysCfgData.DevInfo[i].rtu_typeIndex
                <<gDeviceDataBase.SysCfgData.DevInfo[i].rtu_typeVer;
    }

    for(int i=0;i<1;i++)
        for(int j=0;j<1;j++)
        {
             qDebug()<<i<<j<<"#####=======--------";
             qDebug()<<"MeasureSection1"<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[0].CommuTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[0].DisplayTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[0].startaddr;

             for(int k=0;k<MAX_VAL_NUM;k++)
             {
                  qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[0].Point[k].name
                          <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[0].Point[k].addr
                          <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[0].Point[k].bitsnum
                          <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[0].Point[k].comment
                          <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[0].Point[k].BitIndex;
             }

             qDebug()<<"MeasureSection2"<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[1].CommuTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[1].DisplayTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[1].startaddr;

             for(int k=0;k<MAX_VAL_NUM;k++)
             {
                 qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[1].Point[k].name
                         <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[1].Point[k].addr
                         <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[1].Point[k].bitsnum
                         <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[1].Point[k].comment
                         <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureSection[1].Point[k].BitIndex;
             }

             qDebug()<<"ConstantSection1"
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[0].CommuTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[0].DisplayTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[0].startaddr;

             for(int k=0;k<MAX_VAL_NUM;k++)
             {
                  qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[0].Point[k].name
                            <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[0].Point[k].addr
                              <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[0].Point[k].bitsnum
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[0].Point[k].comment
                                  <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[0].Point[k].BitIndex;
             }

             qDebug()<<"ConstantSection2"
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[1].CommuTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[1].DisplayTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[1].startaddr;

             for(int k=0;k<MAX_VAL_NUM;k++)
             {
                  qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[1].Point[k].name
                            <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[1].Point[k].addr
                              <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[1].Point[k].bitsnum
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[1].Point[k].comment
                                  <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ConstantSection[1].Point[k].BitIndex;
             }

             qDebug()<<"fault section"
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].Faultsection.CommuTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].Faultsection.DisplayTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].Faultsection.startaddr;

             for(int k=0;k<MAX_VAL_NUM;k++)
             {
                  qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].Faultsection.Point[k].name
                            <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].Faultsection.Point[k].addr
                              <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].Faultsection.Point[k].bitsnum
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].Faultsection.Point[k].comment
                                  <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].Faultsection.Point[k].BitIndex;
             }

             qDebug()<<"fault parse"
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultParseSection.Total
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultParseSection.SectionName;

             for(int k=0;k<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultParseSection.Total;k++)
                 qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultParseSection.MapIndex[k];

             qDebug()<<"switch status parse"
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].SwitchStatusParseSection.SectionName
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].SwitchStatusParseSection.MapIndex;

             qDebug()<<"powerstamp parse"
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].PowerDownStampParseSection.SectionName
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].PowerDownStampParseSection.MapIndex;

             qDebug()<<"voltage and current parse"
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].VoltageCurrentParseSection.SectionName
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].VoltageCurrentParseSection.vindex
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].VoltageCurrentParseSection.Iindex;

             qDebug()<<"ElecDegreeSection"
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeSection.CommuTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeSection.DisplayTotal
                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeSection.startaddr;

             for(int k=0;k<16;k++)
             {
                  qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeSection.Point[k].name
                            <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeSection.Point[k].addr
                              <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeSection.Point[k].bitsnum
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeSection.Point[k].comment
                                  <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeSection.Point[k].ratio;
            }

            qDebug()<<"timesync"<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].TimeSyncSection.enable
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].TimeSyncSection.addr[0]
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].TimeSyncSection.addr[1]
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].TimeSyncSection.addr[2]
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].TimeSyncSection.addr[3]
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].TimeSyncSection.addr[4]
                                <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].TimeSyncSection.addr[5];

            qDebug()<<"switch control"
                    <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].SwitchControlSection.total;
            for(int k=0;k<8;k++)
                qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].SwitchControlSection.Point[k].addr
                        <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].SwitchControlSection.Point[k].cmd
                        <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].SwitchControlSection.Point[k].name;

           qDebug()<<"fault code"<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultCodeSection[0].total;
           for(int k=0;k<50;k++)
               qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultCodeSection[0].Point[k].name
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultCodeSection[0].Point[k].Code;
            }

    qDebug()<<"inter data";
    for(int i=0;i<1;i++)
           for(int j=0;j<1;j++)
           {
                qDebug()<<i<<j<<"######";
                qDebug()<<" interMeasureSection1"
                        <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[0].CommuTotal
                        <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[0].DisplayTotal
                        <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[0].startaddr;

                for(int k = 0; k < MAX_VAL_NUM; k++)
                {
                     qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[0].Point[k].name
                               <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[0].Point[k].addr
                                 <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[0].Point[k].bitsnum
                                   <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[0].Point[k].comment
                                    <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[0].Point[k].BitIndex;
                }

                qDebug()<<"inter MeasureSection2"
                        <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[1].CommuTotal
                        <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[1].DisplayTotal
                        <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[1].startaddr;

                for(int k=0;k<MAX_VAL_NUM;k++)
                {
                     qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[1].Point[k].name
                               <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[1].Point[k].addr
                                 <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[1].Point[k].bitsnum
                                   <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[1].Point[k].comment
                                     <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].MeasureInternSection[1].Point[k].BitIndex;
                }

//                qDebug()<<"inter fault section"<<valid_cfg_info.interdata[i][j].Faultsection.CommuTotal<<valid_cfg_info.interdata[i][j].Faultsection.DisplayTotal
//                             <<valid_cfg_info.interdata[i][j].Faultsection.startaddr;

//                               for(int k=0;k<MAX_VAL_NUM;k++)
//                               {
//                                    qDebug()<<valid_cfg_info.interdata[i][j].Faultsection.Point[k].name
//                                              <<valid_cfg_info.interdata[i][j].Faultsection.Point[k].addr
//                                                <<valid_cfg_info.interdata[i][j].Faultsection.Point[k].bitsnum
//                                                  <<valid_cfg_info.interdata[i][j].Faultsection.Point[k].comment
//                                                  <<valid_cfg_info.interdata[i][j].Faultsection.Point[k].BitIndex;
//                               }

                qDebug()<<"inter fault parse"
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultParseInternSection.Total
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultParseInternSection.SectionName;

                for(int k=0;k<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultParseInternSection.Total;k++)
                      qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].FaultParseInternSection.MapIndex[k];

                qDebug()<<"inter switch status parse"
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].SwitchStatusParseInternSection.SectionName
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].SwitchStatusParseInternSection.MapIndex;

                qDebug()<<"inter powerstamp parse"
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].PowerDownStampParseSection.SectionName
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].PowerDownStampParseSection.MapIndex;

                qDebug()<<"inter voltage and current parse"
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].VoltageCurrentParseSection.SectionName
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].VoltageCurrentParseSection.vindex
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].VoltageCurrentParseSection.Iindex;

                qDebug()<<"inter ElecDegreeSection"
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeInternSection.CommuTotal
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeInternSection.DisplayTotal
                       <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeInternSection.startaddr;
                for(int k=0;k<16;k++)
                {
                     qDebug()<<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeInternSection.Point[k].name
                               <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeInternSection.Point[k].addr
                                 <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeInternSection.Point[k].bitsnum
                                   <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeInternSection.Point[k].comment
                                   <<gDeviceDataBase.SysCfgData.ProtoCfgData[i][j].ElecDegreeInternSection.Point[k].ratio;
               }

//               qDebug()<<"inter timesync"<<valid_cfg_info.interdata[i][j].TimeSyncSection.enable<<valid_cfg_info.interdata[i][j].TimeSyncSection.addr[0]
//                         <<valid_cfg_info.interdata[i][j].TimeSyncSection.addr[1]
//                           <<valid_cfg_info.interdata[i][j].TimeSyncSection.addr[2]
//                             <<valid_cfg_info.interdata[i][j].TimeSyncSection.addr[3]
//                           <<valid_cfg_info.interdata[i][j].TimeSyncSection.addr[4]
//                             <<valid_cfg_info.interdata[i][j].TimeSyncSection.addr[5];

//               qDebug()<<"inter switch control"<<valid_cfg_info.interdata[i][j].SwitchControlSection.total;
//                         for(int k=0;k<8;k++)
//                               qDebug()<<valid_cfg_info.interdata[i][j].SwitchControlSection.Point[k].addr
//                                           <<valid_cfg_info.interdata[i][j].SwitchControlSection.Point[k].cmd
//                                              <<valid_cfg_info.interdata[i][j].SwitchControlSection.Point[k].name;

//              qDebug()<<"interfault code"<<valid_cfg_info.interdata[i][j].FaultCodeSection.total;
//              for(int k=0;k<50;k++)
//                  qDebug()<<valid_cfg_info.interdata[i][j].FaultCodeSection.Point[k].name<<valid_cfg_info.interdata[i][j].FaultCodeSection.Point[k].Code;

               }

}

void centralCtl::protocollistdebug()
{
    for(int i=0;i<gDeviceDataBase.SysCfgData.ProtoList.ProtoTotal;i++)
    {
        qDebug()<<i
                <<gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[i].Name//.ProtoName[i]
                <<gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[i].VerNum
                //<<gDeviceDataBase.Protocollist.ProtoVerNum[i]
                //<<gDeviceDataBase.Protocollist.ProtoFileName[i][0];
                <<gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[i].FileName[0];
    }
    for(int i =0;i<96;i++)
        qDebug()<<i<<gDeviceDataBase.SysCfgData.DevInfo[i].logic_addr<<gDeviceDataBase.SysCfgData.DevInfo[i].phy_addr
                <<gDeviceDataBase.SysCfgData.DevInfo[i].rtu_typeIndex;
}

void centralCtl::MeasureInsDisplayslot()
{
    this->measureIns.EnterProcess();
    this->measureIns.move(0,0);
    this->measureIns.show();
}

void centralCtl::ConstantInsDisplayslot()
{
    this->constantIns.EnterProcess();
    this->constantIns.move(0,0);
    this->constantIns.show();
}

void centralCtl::controlInsDisplayslot()
{
    this->controlIns.EnterProcess();
    this->controlIns.move(0,0);
    this->controlIns.show();
}
void centralCtl::autorunInsDisplayslot()
{
    this->pumpautorunIns.EnterProcess();
    this->pumpautorunIns.move(0,0);
    this->pumpautorunIns.show();
}
void centralCtl::sysconfigDisplayslot()
{
//    this->sysconfigIns.EnterProcess();
//    this->sysconfigIns.show();
}

void centralCtl::faultHistoryDisplayslot()
{
    this->faultIns.EntryProcess();
    this->faultIns.show();
}

void centralCtl::faultrealtimeDisplayslot()
{
//    this->realtimefaultIns.displayFaultlist();
//    this->realtimefaultIns.show();
}

void centralCtl::PowerDownStampDisplayslot()
{
   // this->operateIns.show();
//    this->powerdownstampdisplayIns.show();
}

void centralCtl::PowerDownStampSetupslot()
{
//    this->powerdownstampsetupIns.show();
}

void centralCtl::usrloginDisplayslot()
{
    if(gDeviceDataBase.UsrLoginInfo.islogin == LOGOUT)
    {
        this->usrloginIns.loginInit();
        this->usrloginIns.move((800-540)/2,(480-374)/2);
        this->usrloginIns.show();
    }
}

void centralCtl::usrpasswdmodifyDisplayslot()
{
   // this->poweruserloginIns.loginInit();
    qDebug()<<"111111111";
//    this->powerusrmodifypasswdIns.EnterProcess();
//    this->powerusrmodifypasswdIns.move((800-540)/2,(480-374)/2);
//    this->powerusrmodifypasswdIns.show();
}

void centralCtl::managerloginslot()
{
//    this->powerusrmanagerIns.displayusrinfo();
//    this->powerusrmanagerIns.move(0,0);
//    this->powerusrmanagerIns.show();
}

void centralCtl::confirmsaveusrinfoslot()
{
    QString srcstr;
    std::string midstr;
    const char *descstr;

    tinyxml2::XMLDocument doc;

    //1.添加声明
    tinyxml2::XMLDeclaration* declaration = doc.NewDeclaration();
    doc.InsertFirstChild(declaration);

    //2.创建根节点
    tinyxml2::XMLElement* root = doc.NewElement("usrinfolist");
    doc.InsertEndChild(root);

    //3.创建子节点
    tinyxml2::XMLElement *admin  = doc.NewElement("admin");
    root->InsertFirstChild(admin);

    tinyxml2::XMLElement *adminsubnode = doc.NewElement("name");

//    srcstr = QString::number(usrlistdata.adminlvl.name);
    midstr = srcstr.toStdString();
    descstr = midstr.c_str();

    tinyxml2::XMLText* pText = doc.NewText(descstr);
    adminsubnode->InsertEndChild(pText);
    admin->InsertEndChild(adminsubnode);

    adminsubnode = doc.NewElement("passwd");
    //srcstr = QString::number(usrlistdata.adminlvl.passwd);
    midstr = srcstr.toStdString();
    descstr = midstr.c_str();
    pText = doc.NewText(descstr);
    adminsubnode->InsertEndChild(pText);
    admin->InsertEndChild(adminsubnode);

    tinyxml2::XMLElement *engineer = doc.NewElement("engineer");
    root->InsertEndChild(engineer);

    tinyxml2::XMLElement *engineersubnode;
    tinyxml2::XMLElement *usrinfonode;

    tinyxml2::XMLText *pengineerText;

    qDebug()<<"1111111111111111111";
        int i=0;
//    for(int i=0;i<usrlistdata.engineerlvl.count();i++)
    {
        engineersubnode = doc.NewElement("usrinfo");
        engineersubnode->SetAttribute("id",i);

        usrinfonode = doc.NewElement("name");
//        srcstr = QString::number(usrlistdata.engineerlvl.at(i).name);
        midstr = srcstr.toStdString();
        descstr = midstr.c_str();
        pengineerText = doc.NewText(descstr);
        usrinfonode->InsertEndChild(pengineerText);
        engineersubnode->InsertEndChild(usrinfonode);


        usrinfonode = doc.NewElement("passwd");
//        srcstr = QString::number(usrlistdata.engineerlvl.at(i).passwd);
        midstr = srcstr.toStdString();
        descstr = midstr.c_str();
        pengineerText = doc.NewText(descstr);
        usrinfonode->InsertEndChild(pengineerText);
        engineersubnode->InsertEndChild(usrinfonode);

        engineer->InsertEndChild(engineersubnode);
    }

    tinyxml2::XMLElement *opt = doc.NewElement("operat");
    root->InsertEndChild(opt);

    tinyxml2::XMLElement *optsubnode;
    tinyxml2::XMLElement *optusrinfonode;

    tinyxml2::XMLText *optText;

//    for(int i=0;i<usrlistdata.operaterlvl.count();i++)
    {
        optsubnode = doc.NewElement("usrinfo");
        optsubnode->SetAttribute("id",i);

        optusrinfonode = doc.NewElement("name");
//        srcstr = QString::number(usrlistdata.operaterlvl.at(i).name);
        midstr = srcstr.toStdString();
        descstr = midstr.c_str();
        optText = doc.NewText(descstr);
        optusrinfonode->InsertEndChild(optText);
        optsubnode->InsertEndChild(optusrinfonode);

        optusrinfonode = doc.NewElement("passwd");
//        srcstr = QString::number(usrlistdata.operaterlvl.at(i).passwd);
        midstr = srcstr.toStdString();
        descstr = midstr.c_str();
        optText = doc.NewText(descstr);
        optusrinfonode->InsertEndChild(optText);
        optsubnode->InsertEndChild(optusrinfonode);

        opt->InsertEndChild(optsubnode);
    }
    doc.SaveFile("/waterpump/gaspumpconfigfiles/usrinfo/usrpwdlist.xml");
}


void centralCtl::RealtimeFaultDisplayslot(int addr, DETAIL_VALUE *value)
{
//    this->realtimefaultIns.realtimefaultProcess(addr,value);
//    this->realtimefaultIns.show();
}

void centralCtl::ReadFromsysCfgfile()
{
    INT32S tPortNum   = 0;
    INT32S tLogicaddr = 0;

    XMLDocument doc;

#ifdef HBDEBUG
    XMLError ret = doc.LoadFile( "/home/imx6/Desktop/debugFolder/configfiles/sysconfigtest.xml" );
#else
    XMLError ret = doc.LoadFile( "/waterpump/gaspumpconfigfiles/sysconfigtest.xml" );
#endif
    if(ret != XML_SUCCESS)
    {
        qDebug()<<"sysconfigtest.xml failed"<<ret<<__LINE__;
        return;
    }
    qDebug()<<"load success"<<ret;

    XMLElement* root = doc.RootElement();
    if(root)
    {
        XMLElement* subNodecommu = root->FirstChildElement("SYSCOMMU");
        if(subNodecommu)
        {
            XMLElement *Node1 = subNodecommu->FirstChildElement("StationName");
            if(Node1)
            {
               // qDebug()<<"StationName"<<Node1->GetText();
                memcpy(gDeviceDataBase.SysCfgData.StationName,Node1->GetText(),strlen(Node1->GetText()));

                Node1 = Node1->NextSiblingElement("COM");
                while(Node1)
                {
                   // qDebug()<<Node1->Attribute("port");
                    tPortNum = atoi(Node1->Attribute("port"));

                    if((tPortNum < 0) || (tPortNum > 5))
                        tPortNum = 0;

                    XMLElement * paramNode = Node1->FirstChildElement("PARAM");
                    if(paramNode)
                    {
//                        qDebug()<<paramNode->FirstChildElement("SPEED")->GetText()
//                                <<paramNode->FirstChildElement("DATABITS")->GetText()
//                                <<paramNode->FirstChildElement("PARITY")->GetText()
//                                <<paramNode->FirstChildElement("STOPBITS")->GetText();

                        gDeviceDataBase.SysCfgData.ComInfo[tPortNum].speed    = atoi(paramNode->FirstChildElement("SPEED")->GetText());
                        gDeviceDataBase.SysCfgData.ComInfo[tPortNum].databits = atoi(paramNode->FirstChildElement("DATABITS")->GetText());
                        gDeviceDataBase.SysCfgData.ComInfo[tPortNum].parity   = atoi(paramNode->FirstChildElement("PARITY")->GetText());
                        gDeviceDataBase.SysCfgData.ComInfo[tPortNum].stopbit  = atoi(paramNode->FirstChildElement("STOPBITS")->GetText());

//                        qDebug()<<"serial param = "<<tPortNum<<valid_cfg_info.com[tPortNum].speed
//                                <<valid_cfg_info.com[tPortNum].databits<<valid_cfg_info.com[tPortNum].parity
//                               <<valid_cfg_info.com[tPortNum].stopbit;
                    }

                    paramNode = paramNode->NextSiblingElement("DEVICE_NUM");
                    if(paramNode)
                    {
                       // qDebug()<<paramNode->GetText();
                        gDeviceDataBase.SysCfgData.OneComDevTotal[tPortNum] = atoi(paramNode->GetText());
                    }
                    if(gDeviceDataBase.SysCfgData.OneComDevTotal[tPortNum])
                    {
                        paramNode = paramNode->NextSiblingElement("DEV");
                        while(paramNode)
                        {
//                            qDebug()<<"DEV"<<paramNode->Attribute("logicaddr")
//                                    <<paramNode->FirstChildElement("PHY_ADDR")->GetText()
//                                    <<paramNode->FirstChildElement("PROTO_INDEX")->GetText()
//                                    <<paramNode->FirstChildElement("PROTO_VER")->GetText()
//                                    <<paramNode->FirstChildElement("DEVICE_NAME")->GetText()
//                                    <<paramNode->FirstChildElement("FACTORY_AD")->GetText();

                            tLogicaddr = atoi(paramNode->Attribute("logicaddr"));
                            if((tLogicaddr < 0) ||(tLogicaddr > 95))
                                tLogicaddr = 0;

                            gDeviceDataBase.SysCfgData.DevInfo[tLogicaddr].logic_addr = tLogicaddr;
                            gDeviceDataBase.SysCfgData.DevInfo[tLogicaddr].com_id     = tPortNum;
                            gDeviceDataBase.SysCfgData.DevInfo[tLogicaddr].phy_addr   = atoi(paramNode->FirstChildElement("PHY_ADDR")->GetText());
                            gDeviceDataBase.SysCfgData.DevInfo[tLogicaddr].rtu_typeIndex = atoi(paramNode->FirstChildElement("PROTO_INDEX")->GetText());
                            gDeviceDataBase.SysCfgData.DevInfo[tLogicaddr].rtu_typeVer   = atoi(paramNode->FirstChildElement("PROTO_VER")->GetText());
                            memcpy(gDeviceDataBase.SysCfgData.DevInfo[tLogicaddr].devcode,
                                   paramNode->FirstChildElement("DEVICE_NAME")->GetText(),
                                   strlen(paramNode->FirstChildElement("DEVICE_NAME")->GetText()));
                            gDeviceDataBase.SysCfgData.DevInfo[tLogicaddr].ad = atoi(paramNode->FirstChildElement("FACTORY_AD")->GetText());

//                            qDebug()<<"information"<<valid_cfg_info.info[logicaddr].logic_addr
//                                    <<valid_cfg_info.info[logicaddr].phy_addr
//                                    <<valid_cfg_info.info[logicaddr].rtu_typeIndex
//                                    << valid_cfg_info.info[logicaddr].rtu_typeVer
//                                    <<valid_cfg_info.info[logicaddr].num;

                            paramNode = paramNode->NextSiblingElement("DEV");
                        }
                    }
                    Node1 = Node1->NextSiblingElement("COM");
                }
            }
        }
    }
}

void centralCtl::readfromstructfile()
{
    int structpointindex = 0;
    int structdevtotal = 0;

    int index[8];
    memset(index,0x00,32);

    //XMLDocument doc;
#ifdef HBDEBUG
    XMLError ret = structdoc.LoadFile( "/home/imx6/Desktop/debugFolder/configfiles/sysStruct.xml" );
#else
    XMLError ret = structdoc.LoadFile( "/waterpump/gaspumpconfigfiles/sysStruct.xml" );
#endif
    if(ret != XML_SUCCESS)
    {
        qDebug()<<"failed"<<ret<<__LINE__;
        return;
    }
    qDebug()<<"sysStruct.xml"<<"load success"<<ret;

    XMLElement* root = structdoc.RootElement();

    if(root)
    {
        XMLElement* subNodestruct = root->FirstChildElement("SYSSTRUCT_I");
        if(subNodestruct)
        {
                systemStruct.valid = VALIDSTRUCTINFO;
                XMLElement * subSTAGE1Node = subNodestruct->FirstChildElement("STAGE1");
                index[0] = 0;
                while(subSTAGE1Node)
                {
                    systemStruct.devPoint[structpointindex].level = 1;
                    //systemStruct.devPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                    systemStruct.devPoint[structpointindex].Index1 = index[0];

                    systemStruct.devPoint[structpointindex].logicaddr = atoi(subSTAGE1Node->FirstChildElement("ADDR")->GetText());

                    if(subSTAGE1Node->FirstChildElement("STAGE2"))
                        systemStruct.devPoint[structpointindex].childflag = 1;
                    else
                        systemStruct.devPoint[structpointindex].childflag = 0;

//                    qDebug()<<structpointindex<<systemStruct.devPoint[structpointindex].level
//                            <<systemStruct.devPoint[structpointindex].Index1
//                            <<systemStruct.devPoint[structpointindex].Index2
//                            <<systemStruct.devPoint[structpointindex].Index3
//                            <<systemStruct.devPoint[structpointindex].logicaddr
//                            <<systemStruct.devPoint[structpointindex].childflag;

                    structpointindex++;
                    structdevtotal++;

                    XMLElement *subSTAGE2Node = subSTAGE1Node->FirstChildElement("STAGE2");
                    while(subSTAGE2Node)
                    {
                        systemStruct.devPoint[structpointindex].level = 2;
//                        systemStruct.devPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
//                        systemStruct.devPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));

                        systemStruct.devPoint[structpointindex].Index1 = index[0];
                        systemStruct.devPoint[structpointindex].Index2 = index[1];
                        systemStruct.devPoint[structpointindex].logicaddr = atoi(subSTAGE2Node->FirstChildElement("ADDR")->GetText());

                        if(subSTAGE2Node->FirstChildElement("STAGE3"))
                           systemStruct.devPoint[structpointindex].childflag = 1;
                        else
                           systemStruct.devPoint[structpointindex].childflag = 0;

//                        qDebug()<<structpointindex<<systemStruct.devPoint[structpointindex].level
//                                <<systemStruct.devPoint[structpointindex].Index1
//                                <<systemStruct.devPoint[structpointindex].Index2
//                                <<systemStruct.devPoint[structpointindex].Index3
//                                <<systemStruct.devPoint[structpointindex].logicaddr
//                                <<systemStruct.devPoint[structpointindex].childflag;

                        structpointindex++;
                        structdevtotal++;

                        XMLElement * subSTAGE3Node = subSTAGE2Node->FirstChildElement("STAGE3");
                        while(subSTAGE3Node)
                        {
                            systemStruct.devPoint[structpointindex].level = 3;
//                            systemStruct.devPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
//                            systemStruct.devPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
//                            systemStruct.devPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
                            systemStruct.devPoint[structpointindex].Index1 = index[0];
                            systemStruct.devPoint[structpointindex].Index2 = index[1];
                            systemStruct.devPoint[structpointindex].Index3 = index[2];
                            systemStruct.devPoint[structpointindex].logicaddr = atoi(subSTAGE3Node->FirstChildElement("ADDR")->GetText());
                            if(subSTAGE3Node->FirstChildElement("STAGE4"))
                                systemStruct.devPoint[structpointindex].childflag = 1;
                            else
                                systemStruct.devPoint[structpointindex].childflag = 0
                                        ;
//                            qDebug()<<structpointindex<<systemStruct.devPoint[structpointindex].level
//                                    <<systemStruct.devPoint[structpointindex].Index1
//                                    <<systemStruct.devPoint[structpointindex].Index2
//                                    <<systemStruct.devPoint[structpointindex].Index3
//                                    <<systemStruct.devPoint[structpointindex].logicaddr
//                                    <<systemStruct.devPoint[structpointindex].childflag;
                            structpointindex++;
                            structdevtotal++;

                            XMLElement *subSTAGE4Node = subSTAGE3Node->FirstChildElement("STAGE4");
                            while(subSTAGE4Node)
                            {
                                systemStruct.devPoint[structpointindex].level = 4;
//                                systemStruct.devPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
//                                systemStruct.devPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
//                                systemStruct.devPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
//                                systemStruct.devPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
                                systemStruct.devPoint[structpointindex].Index1 = index[0];
                                systemStruct.devPoint[structpointindex].Index2 = index[1];
                                systemStruct.devPoint[structpointindex].Index3 = index[2];
                                systemStruct.devPoint[structpointindex].Index4 = index[3];
                                systemStruct.devPoint[structpointindex].logicaddr = atoi(subSTAGE4Node->FirstChildElement("ADDR")->GetText());
                                if(subSTAGE4Node->FirstChildElement("STAGE5"))
                                    systemStruct.devPoint[structpointindex].childflag = 1;
                                else
                                    systemStruct.devPoint[structpointindex].childflag = 0;
                                structpointindex++;
                                structdevtotal++;

                                XMLElement *subSTAGE5Node = subSTAGE4Node->FirstChildElement("STAGE5");
                                while(subSTAGE5Node)
                                {
                                    systemStruct.devPoint[structpointindex].level = 5;
//                                    systemStruct.devPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
//                                    systemStruct.devPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
//                                    systemStruct.devPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
//                                    systemStruct.devPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
//                                    systemStruct.devPoint[structpointindex].Index5 = atoi(subSTAGE5Node->Attribute("index"));
                                    systemStruct.devPoint[structpointindex].Index1 = index[0];
                                    systemStruct.devPoint[structpointindex].Index2 = index[1];
                                    systemStruct.devPoint[structpointindex].Index3 = index[2];
                                    systemStruct.devPoint[structpointindex].Index4 = index[3];
                                    systemStruct.devPoint[structpointindex].Index5 = index[4];
                                    systemStruct.devPoint[structpointindex].logicaddr = atoi(subSTAGE5Node->FirstChildElement("ADDR")->GetText());
                                    if(subSTAGE5Node->FirstChildElement("STAGE6"))
                                        systemStruct.devPoint[structpointindex].childflag = 1;
                                    else
                                        systemStruct.devPoint[structpointindex].childflag = 0;
                                    structpointindex++;
                                    structdevtotal++;

                                    XMLElement *subSTAGE6Node = subSTAGE5Node->FirstChildElement("STAGE6");
                                    while(subSTAGE6Node)
                                    {
                                        systemStruct.devPoint[structpointindex].level = 6;
//                                        systemStruct.devPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
//                                        systemStruct.devPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
//                                        systemStruct.devPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
//                                        systemStruct.devPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
//                                        systemStruct.devPoint[structpointindex].Index5 = atoi(subSTAGE5Node->Attribute("index"));
//                                        systemStruct.devPoint[structpointindex].Index6 = atoi(subSTAGE6Node->Attribute("index"));
                                        systemStruct.devPoint[structpointindex].Index1 = index[0];
                                        systemStruct.devPoint[structpointindex].Index2 = index[1];
                                        systemStruct.devPoint[structpointindex].Index3 = index[2];
                                        systemStruct.devPoint[structpointindex].Index4 = index[3];
                                        systemStruct.devPoint[structpointindex].Index5 = index[4];
                                        systemStruct.devPoint[structpointindex].Index6 = index[5];
                                        systemStruct.devPoint[structpointindex].logicaddr = atoi(subSTAGE6Node->FirstChildElement("ADDR")->GetText());
                                        if(subSTAGE6Node->FirstChildElement("STAGE7"))
                                            systemStruct.devPoint[structpointindex].childflag = 1;
                                        else
                                            systemStruct.devPoint[structpointindex].childflag = 0;
                                        structpointindex++;
                                        structdevtotal++;

                                        XMLElement *subSTAGE7Node = subSTAGE6Node->FirstChildElement("STAGE7");
                                        while(subSTAGE7Node)
                                        {
                                            systemStruct.devPoint[structpointindex].level = 7;
//                                            systemStruct.devPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
//                                            systemStruct.devPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
//                                            systemStruct.devPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
//                                            systemStruct.devPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
//                                            systemStruct.devPoint[structpointindex].Index5 = atoi(subSTAGE5Node->Attribute("index"));
//                                            systemStruct.devPoint[structpointindex].Index6 = atoi(subSTAGE6Node->Attribute("index"));
//                                            systemStruct.devPoint[structpointindex].Index7 = atoi(subSTAGE7Node->Attribute("index"));
                                            systemStruct.devPoint[structpointindex].Index1 = index[0];
                                            systemStruct.devPoint[structpointindex].Index2 = index[1];
                                            systemStruct.devPoint[structpointindex].Index3 = index[2];
                                            systemStruct.devPoint[structpointindex].Index4 = index[3];
                                            systemStruct.devPoint[structpointindex].Index5 = index[4];
                                            systemStruct.devPoint[structpointindex].Index6 = index[5];
                                            systemStruct.devPoint[structpointindex].Index7 = index[6];
                                            systemStruct.devPoint[structpointindex].logicaddr = atoi(subSTAGE7Node->FirstChildElement("ADDR")->GetText());
                                            if(subSTAGE7Node->FirstChildElement("STAGE8"))
                                                systemStruct.devPoint[structpointindex].childflag = 1;
                                            else
                                                systemStruct.devPoint[structpointindex].childflag = 0;
                                            structpointindex++;
                                            structdevtotal++;

                                            XMLElement *subSTAGE8Node = subSTAGE7Node->FirstChildElement("STAGE8");
                                            while(subSTAGE8Node)
                                            {
                                                systemStruct.devPoint[structpointindex].level = 8;
//                                                systemStruct.devPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
//                                                systemStruct.devPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
//                                                systemStruct.devPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
//                                                systemStruct.devPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
//                                                systemStruct.devPoint[structpointindex].Index5 = atoi(subSTAGE5Node->Attribute("index"));
//                                                systemStruct.devPoint[structpointindex].Index6 = atoi(subSTAGE6Node->Attribute("index"));
//                                                systemStruct.devPoint[structpointindex].Index7 = atoi(subSTAGE7Node->Attribute("index"));
//                                                systemStruct.devPoint[structpointindex].Index8 = atoi(subSTAGE8Node->Attribute("index"));
                                                systemStruct.devPoint[structpointindex].Index1 = index[0];
                                                systemStruct.devPoint[structpointindex].Index2 = index[1];
                                                systemStruct.devPoint[structpointindex].Index3 = index[2];
                                                systemStruct.devPoint[structpointindex].Index4 = index[3];
                                                systemStruct.devPoint[structpointindex].Index5 = index[4];
                                                systemStruct.devPoint[structpointindex].Index6 = index[5];
                                                systemStruct.devPoint[structpointindex].Index7 = index[6];
                                                systemStruct.devPoint[structpointindex].Index8 = index[7];
                                                systemStruct.devPoint[structpointindex].logicaddr = atoi(subSTAGE8Node->FirstChildElement("ADDR")->GetText());

                                                systemStruct.devPoint[structpointindex].childflag = 0;
                                                structpointindex++;
                                                structdevtotal++;

                                                index[7]++;
                                                subSTAGE8Node = subSTAGE8Node->NextSiblingElement("STAGE8");
                                            }
                                            index[6]++;
                                            subSTAGE7Node = subSTAGE7Node->NextSiblingElement("STAGE7");
                                        }
                                        index[5]++;
                                        subSTAGE6Node = subSTAGE6Node->NextSiblingElement("STAGE6");
                                    }
                                    index[4]++;
                                    subSTAGE5Node = subSTAGE5Node->NextSiblingElement("STAGE5");
                                }
                                index[3]++;
                                subSTAGE4Node = subSTAGE4Node->NextSiblingElement("STAGE4");
                            }
                            index[2]++;
                            subSTAGE3Node = subSTAGE3Node->NextSiblingElement("STAGE3");
                        }
                        index[1]++;
                        subSTAGE2Node = subSTAGE2Node->NextSiblingElement("STAGE2");
                    }
                    index[0]++;
                    subSTAGE1Node = subSTAGE1Node->NextSiblingElement("STAGE1");
                }
         }

        systemStruct.totalI = structdevtotal;
        structpointindex = 0;
//        qDebug()<<"totalI ="<< systemStruct.totalI;

        structdevtotal = 0;
        subNodestruct = root->FirstChildElement("SYSSTRUCT_II");
        if(subNodestruct)
        {
                //systemStruct.valid = VALIDSTRUCTINFO;
                XMLElement * subSTAGE1Node = subNodestruct->FirstChildElement("STAGE1");
                while(subSTAGE1Node)
                {
                    systemStruct.IIdevPoint[structpointindex].level = 1;
                    systemStruct.IIdevPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                    systemStruct.IIdevPoint[structpointindex].logicaddr = atoi(subSTAGE1Node->FirstChildElement("ADDR")->GetText());

                    if(subSTAGE1Node->FirstChildElement("STAGE2"))
                        systemStruct.IIdevPoint[structpointindex].childflag = 1;
                    else
                        systemStruct.IIdevPoint[structpointindex].childflag = 0;

//                    qDebug()<<structpointindex<<systemStruct.devPoint[structpointindex].level
//                            <<systemStruct.devPoint[structpointindex].Index1
//                            <<systemStruct.devPoint[structpointindex].Index2
//                            <<systemStruct.devPoint[structpointindex].Index3
//                            <<systemStruct.devPoint[structpointindex].logicaddr
//                            <<systemStruct.devPoint[structpointindex].childflag;

                    structpointindex++;
                    structdevtotal++;

                    XMLElement *subSTAGE2Node = subSTAGE1Node->FirstChildElement("STAGE2");
                    while(subSTAGE2Node)
                    {
                        systemStruct.IIdevPoint[structpointindex].level = 2;
                        systemStruct.IIdevPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                        systemStruct.IIdevPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
                        systemStruct.IIdevPoint[structpointindex].logicaddr = atoi(subSTAGE2Node->FirstChildElement("ADDR")->GetText());

                        if(subSTAGE2Node->FirstChildElement("STAGE3"))
                           systemStruct.IIdevPoint[structpointindex].childflag = 1;
                        else
                           systemStruct.IIdevPoint[structpointindex].childflag = 0;

//                        qDebug()<<structpointindex<<systemStruct.IIdevPoint[structpointindex].level
//                                <<systemStruct.IIdevPoint[structpointindex].Index1
//                                <<systemStruct.IIdevPoint[structpointindex].Index2
//                                <<systemStruct.IIdevPoint[structpointindex].Index3
//                                <<systemStruct.IIdevPoint[structpointindex].logicaddr
//                                <<systemStruct.IIdevPoint[structpointindex].childflag;

                        structpointindex++;
                        structdevtotal++;

                        XMLElement * subSTAGE3Node = subSTAGE2Node->FirstChildElement("STAGE3");
                        while(subSTAGE3Node)
                        {
                            systemStruct.IIdevPoint[structpointindex].level = 3;
                            systemStruct.IIdevPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                            systemStruct.IIdevPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
                            systemStruct.IIdevPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
                            systemStruct.IIdevPoint[structpointindex].logicaddr = atoi(subSTAGE3Node->FirstChildElement("ADDR")->GetText());
                            if(subSTAGE3Node->FirstChildElement("STAGE4"))
                                systemStruct.IIdevPoint[structpointindex].childflag = 1;
                            else
                                systemStruct.IIdevPoint[structpointindex].childflag = 0
                                        ;
//                            qDebug()<<structpointindex<<systemStruct.IIdevPoint[structpointindex].level
//                                    <<systemStruct.IIdevPoint[structpointindex].Index1
//                                    <<systemStruct.IIdevPoint[structpointindex].Index2
//                                    <<systemStruct.IIdevPoint[structpointindex].Index3
//                                    <<systemStruct.IIdevPoint[structpointindex].logicaddr
//                                    <<systemStruct.IIdevPoint[structpointindex].childflag;
                            structpointindex++;
                            structdevtotal++;

                            XMLElement *subSTAGE4Node = subSTAGE3Node->FirstChildElement("STAGE4");
                            while(subSTAGE4Node)
                            {
                                systemStruct.IIdevPoint[structpointindex].level = 4;
                                systemStruct.IIdevPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                                systemStruct.IIdevPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
                                systemStruct.IIdevPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
                                systemStruct.IIdevPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
                                systemStruct.IIdevPoint[structpointindex].logicaddr = atoi(subSTAGE4Node->FirstChildElement("ADDR")->GetText());
                                if(subSTAGE4Node->FirstChildElement("STAGE5"))
                                    systemStruct.IIdevPoint[structpointindex].childflag = 1;
                                else
                                    systemStruct.IIdevPoint[structpointindex].childflag = 0;
                                structpointindex++;
                                structdevtotal++;

                                XMLElement *subSTAGE5Node = subSTAGE4Node->FirstChildElement("STAGE5");
                                while(subSTAGE5Node)
                                {
                                    systemStruct.IIdevPoint[structpointindex].level = 5;
                                    systemStruct.IIdevPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                                    systemStruct.IIdevPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
                                    systemStruct.IIdevPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
                                    systemStruct.IIdevPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
                                    systemStruct.IIdevPoint[structpointindex].Index5 = atoi(subSTAGE5Node->Attribute("index"));
                                    systemStruct.IIdevPoint[structpointindex].logicaddr = atoi(subSTAGE5Node->FirstChildElement("ADDR")->GetText());
                                    if(subSTAGE5Node->FirstChildElement("STAGE6"))
                                        systemStruct.IIdevPoint[structpointindex].childflag = 1;
                                    else
                                        systemStruct.IIdevPoint[structpointindex].childflag = 0;
                                    structpointindex++;
                                    structdevtotal++;

                                    XMLElement *subSTAGE6Node = subSTAGE5Node->FirstChildElement("STAGE6");
                                    while(subSTAGE6Node)
                                    {
                                        systemStruct.IIdevPoint[structpointindex].level = 6;
                                        systemStruct.IIdevPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                                        systemStruct.IIdevPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
                                        systemStruct.IIdevPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
                                        systemStruct.IIdevPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
                                        systemStruct.IIdevPoint[structpointindex].Index5 = atoi(subSTAGE5Node->Attribute("index"));
                                        systemStruct.IIdevPoint[structpointindex].Index6 = atoi(subSTAGE6Node->Attribute("index"));
                                        systemStruct.IIdevPoint[structpointindex].logicaddr = atoi(subSTAGE6Node->FirstChildElement("ADDR")->GetText());
                                        if(subSTAGE6Node->FirstChildElement("STAGE7"))
                                            systemStruct.IIdevPoint[structpointindex].childflag = 1;
                                        else
                                            systemStruct.IIdevPoint[structpointindex].childflag = 0;
                                        structpointindex++;
                                        structdevtotal++;

                                        XMLElement *subSTAGE7Node = subSTAGE6Node->FirstChildElement("STAGE7");
                                        while(subSTAGE7Node)
                                        {
                                            systemStruct.IIdevPoint[structpointindex].level = 7;
                                            systemStruct.IIdevPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                                            systemStruct.IIdevPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
                                            systemStruct.IIdevPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
                                            systemStruct.IIdevPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
                                            systemStruct.IIdevPoint[structpointindex].Index5 = atoi(subSTAGE5Node->Attribute("index"));
                                            systemStruct.IIdevPoint[structpointindex].Index6 = atoi(subSTAGE6Node->Attribute("index"));
                                            systemStruct.IIdevPoint[structpointindex].Index7 = atoi(subSTAGE7Node->Attribute("index"));
                                            systemStruct.IIdevPoint[structpointindex].logicaddr = atoi(subSTAGE7Node->FirstChildElement("ADDR")->GetText());
                                            if(subSTAGE7Node->FirstChildElement("STAGE8"))
                                                systemStruct.IIdevPoint[structpointindex].childflag = 1;
                                            else
                                                systemStruct.IIdevPoint[structpointindex].childflag = 0;
                                            structpointindex++;
                                            structdevtotal++;

                                            XMLElement *subSTAGE8Node = subSTAGE7Node->FirstChildElement("STAGE8");
                                            while(subSTAGE8Node)
                                            {
                                                systemStruct.IIdevPoint[structpointindex].level = 8;
                                                systemStruct.IIdevPoint[structpointindex].Index1 = atoi(subSTAGE1Node->Attribute("index"));
                                                systemStruct.IIdevPoint[structpointindex].Index2 = atoi(subSTAGE2Node->Attribute("index"));
                                                systemStruct.IIdevPoint[structpointindex].Index3 = atoi(subSTAGE3Node->Attribute("index"));
                                                systemStruct.IIdevPoint[structpointindex].Index4 = atoi(subSTAGE4Node->Attribute("index"));
                                                systemStruct.IIdevPoint[structpointindex].Index5 = atoi(subSTAGE5Node->Attribute("index"));
                                                systemStruct.IIdevPoint[structpointindex].Index6 = atoi(subSTAGE6Node->Attribute("index"));
                                                systemStruct.IIdevPoint[structpointindex].Index7 = atoi(subSTAGE7Node->Attribute("index"));
                                                systemStruct.IIdevPoint[structpointindex].Index8 = atoi(subSTAGE8Node->Attribute("index"));
                                                systemStruct.IIdevPoint[structpointindex].logicaddr = atoi(subSTAGE8Node->FirstChildElement("ADDR")->GetText());

                                                systemStruct.IIdevPoint[structpointindex].childflag = 0;
                                                structpointindex++;
                                                structdevtotal++;

                                                subSTAGE8Node = subSTAGE8Node->NextSiblingElement("STAGE8");
                                            }
                                            subSTAGE7Node = subSTAGE7Node->NextSiblingElement("STAGE7");
                                        }
                                        subSTAGE6Node = subSTAGE6Node->NextSiblingElement("STAGE6");
                                    }
                                    subSTAGE5Node = subSTAGE5Node->NextSiblingElement("STAGE5");
                                }
                                subSTAGE4Node = subSTAGE4Node->NextSiblingElement("STAGE4");
                            }
                            subSTAGE3Node = subSTAGE3Node->NextSiblingElement("STAGE3");
                        }
                        subSTAGE2Node = subSTAGE2Node->NextSiblingElement("STAGE2");
                    }
                    subSTAGE1Node = subSTAGE1Node->NextSiblingElement("STAGE1");
                }
         }

         systemStruct.totalII = structdevtotal;
//         qDebug()<<"totalII = "<<systemStruct.totalII;

         structpointindex = 0;
         subNodestruct = root->FirstChildElement("CONDEV");
         if(subNodestruct)
         {
             XMLElement * subNode = subNodestruct->FirstChildElement("TOTAL");
             if(subNode)
             {
                 systemStruct.totalCon = atoi(subNode->GetText());
             }

             subNode = subNodestruct->FirstChildElement("DEV");
             while(subNode)
             {
                 XMLElement *subdevNode = subNode->FirstChildElement("LVL");
                 if(subdevNode)
                    systemStruct.ConDevPoint[structpointindex].lvl = atoi(subdevNode->GetText());

                 subdevNode = subNode->FirstChildElement("ADDR");
                 if(subdevNode)
                    systemStruct.ConDevPoint[structpointindex].logicaddr = atoi(subdevNode->GetText());
                 structpointindex++;
                 subNode = subNode->NextSiblingElement("DEV");
             }
         }
    }
}

void centralCtl::ReadFromProtoListFile()
{
    XMLDocument doc;
#ifdef HBDEBUG
    XMLError ret = doc.LoadFile( "/home/imx6/Desktop/debugFolder/configfiles/protocollist.xml" );
#else
    XMLError ret = doc.LoadFile( "/waterpump/gaspumpconfigfiles/protocollist.xml" );
#endif
    if(ret != XML_SUCCESS)
        return;
    else
        qDebug()<<"read protocollist.xml OK";

    XMLElement* root  = doc.RootElement();
    INT32S typeIndex  = 0;
    INT32S VerIndex   = 0;

    if(root)
    {
         XMLElement *subNode = root->FirstChildElement("PROTO_NUM");
         if(subNode)
         {
             qDebug()<<subNode->GetText();
             gDeviceDataBase.SysCfgData.ProtoList.ProtoTotal = atoi(subNode->GetText());

             subNode = subNode->NextSiblingElement("PROTOCOL");
             typeIndex = 0;
             while(subNode)
             {
//                 qDebug()<<subNode->Attribute("index");
                   XMLElement * typeNode = subNode->FirstChildElement("ICON");
                   if(typeNode)
                   {
//                      qDebug()<<typeNode->GetText();
                        memcpy(gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[typeIndex].Icon, typeNode->GetText(),strlen(typeNode->GetText()));
                   }

                   typeNode = subNode->FirstChildElement("NAME");
                   if(typeNode)
                   {
                       // qDebug()<<typeNode->GetText();
                        memcpy(gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[typeIndex].Name,typeNode->GetText(),strlen(typeNode->GetText()));
                   }

                   typeNode = subNode->FirstChildElement("VER_NUM");
                   if(typeNode)
                   {
//                        qDebug()<<typeNode->GetText();
                        gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[typeIndex].VerNum = atoi(typeNode->GetText());
                   }

                   VerIndex = 0;
                   typeNode = subNode->FirstChildElement("PROTOCOL_INS");
                   while(typeNode)
                   {
//                       qDebug()<<typeNode->Attribute("VER");
                       gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[typeIndex].VerArray[VerIndex] = atoi(typeNode->Attribute("VER"));
//                       XMLElement *INSNode = typeNode->FirstChildElement("PRIVATEENABLE");
//                       if(INSNode)
//                       {
//                           qDebug()<<INSNode->GetText();
//                           gDeviceDataBase.Protocollist.ProtoPrivEnable[typeIndex][VerIndex] = atoi(INSNode->GetText());
//                       }
                       XMLElement *INSNode = typeNode->FirstChildElement("FILENAME");
                       if(INSNode)
                       {
//                           qDebug()<<INSNode->GetText();
                           memcpy(gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[typeIndex].FileName[VerIndex],INSNode->GetText(),strlen(INSNode->GetText()));
                       }

                       VerIndex++;
                       typeNode = typeNode->NextSiblingElement("PROTOCOL_INS");
                   }

                   typeIndex++;
                   subNode = subNode->NextSiblingElement("PROTOCOL");
             }
         }
    }
}

void centralCtl::ReadDetailProto()
{
#ifdef HBDEBUG
    char path[100] = "/home/imx6/Desktop/debugFolder/configfiles/";
#else
    char path[100] = "/waterpump/gaspumpconfigfiles/";
#endif
    char wholefilepath[200];
    memset(wholefilepath,0x00,sizeof(wholefilepath));

    for(INT8U i = 0; i < gDeviceDataBase.SysCfgData.ProtoList.ProtoTotal;i++)
    {
        for(INT8U j = 0; j < gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[i].VerNum; j++)
        {
            strcpy(wholefilepath,path);
            strcpy(wholefilepath+strlen(path),gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[i].FileName[j]);

            qDebug()<<"wholefilepath="<<wholefilepath<<gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[i].FileName[j]<<strlen(path);
            if(access(wholefilepath,F_OK) == 0)
            {
                ReadFromProtoFile(wholefilepath,i,j);
            }
            else
            {
                qDebug()<<gDeviceDataBase.SysCfgData.ProtoList.ProtoArray[i].FileName[j]<<"file not exist";
            }
        }
    }
//    qDebug()<<"end------------------------"<<sizeof(CONFIGURE_FILE_CONTENT)<<sizeof(simpleCONFIGURE_FILE_CONTENT)
//            <<sizeof(simpleTYPE_INFO);
}

void centralCtl::ReadFromProtoFile(char *filename,INT32S ProtoIndex,INT32S ProtoVer)
{
    INT32S pointId = 0;
    INT32S tGroupId = 0;

    INT32S tChannelId = 0;

    XMLDocument doc;
    XMLError ret = doc.LoadFile(filename);
    if(ret != XML_SUCCESS)
    {
        qDebug()<<filename<<"load xml error"<<ret;
        return;
    }
    XMLElement* root  = doc.RootElement();
    if(root)
    {
        XMLElement *subNode = root->FirstChildElement("ProtoName");
        if(subNode)
        {
             strcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ProtoName,subNode->GetText());
            // qDebug()<<subNode->GetText()<<valid_cfg_info.name<<subNode->GetText();
        }
        subNode = root->FirstChildElement("Version");
        if(subNode)
        {
            // qDebug()<<subNode->GetText();
             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Version = atoi(subNode->GetText());
        }

        subNode = root->FirstChildElement("PrivExist");
        if(subNode)
        {
            // qDebug()<<subNode->GetText();
             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].InterExist = atoi(subNode->GetText());
        }

        subNode = root->FirstChildElement("MeasureNum");
        if(subNode)
        {
            // qDebug()<<subNode->GetText();
             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeaureTotalNum = atoi(subNode->GetText());
        }

        subNode = root->FirstChildElement("ConstantNum");
        if(subNode)
        {
            // qDebug()<<subNode->GetText();
             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantTotalNum = atoi(subNode->GetText());
        }

        subNode = root->FirstChildElement("PrivMeasureNum");
        if(subNode)
        {
            // qDebug()<<subNode->GetText();
             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternTotalNum = atoi(subNode->GetText());
        }

        subNode = root->FirstChildElement("FaulCodeGroupNum");
        if(subNode)
        {
            // qDebug()<<subNode->GetText();
             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaulCodeGroupNum = atoi(subNode->GetText());
        }

        //qDebug()<<"public----------------------";
        subNode = root->FirstChildElement("PUBLIC");
        if(subNode)
        {
            XMLElement *SectionNode = subNode->FirstChildElement("TeleAction");
            if(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("FUNCODE");
                if(subSectionNode)
                {
                    // qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TeleActionSection.funcode = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("STARTADDR");
                if(subSectionNode)
                {
                   //  qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TeleActionSection.startaddr = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("CommuTotal");
                if(subSectionNode)
                {
                   //  qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TeleActionSection.CommuTotal = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayTotal");
                if(subSectionNode)
                {
                     qDebug()<<"TeleActionSection DisplayTotal"<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TeleActionSection.DisplayTotal = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayPoint");
                if(subSectionNode)
                {
                     pointId = 0;
                     XMLElement * pointNode = subSectionNode->FirstChildElement("Point");
                     while(pointNode)
                     {
                      //  qDebug()<<pointNode->Attribute("index");
                        if(pointNode->FirstChildElement("Name"))
                        {
                           // qDebug()<<pointNode->FirstChildElement("Name")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TeleActionSection.Point[pointId].name,
                                   pointNode->FirstChildElement("Name")->GetText(),
                                   strlen(pointNode->FirstChildElement("Name")->GetText()));
                        }

                        if(pointNode->FirstChildElement("addr"))
                        {
                           // qDebug()<<pointNode->FirstChildElement("addr")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TeleActionSection.Point[pointId].bitIndex
                                    = atoi(pointNode->FirstChildElement("addr")->GetText());
                        }

                        if(pointNode->FirstChildElement("Comment"))
                        {
                           // qDebug()<<pointNode->FirstChildElement("Comment")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TeleActionSection.Point[pointId].comment,
                                   pointNode->FirstChildElement("Comment")->GetText(),
                                   strlen(pointNode->FirstChildElement("Comment")->GetText()));
                        }

                        pointId++;
                        pointNode = pointNode->NextSiblingElement("Point");
                     }
                }
            }

            SectionNode = subNode->FirstChildElement("MeasureGroup");
            tGroupId = 0;
            while(SectionNode)
            {
                XMLElement * subSectionNode = SectionNode->FirstChildElement("FUNCODE");
                if(subSectionNode)
                {
                    // qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].funcode
                             = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("STARTADDR");
                if(subSectionNode)
                {
                    // qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].startaddr
                             = atoi(subSectionNode->GetText());
                }
                subSectionNode = SectionNode->FirstChildElement("CommuTotal");
                if(subSectionNode)
                {
                   //  qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].CommuTotal
                             = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayTotal");
                if(subSectionNode)
                {
                    // qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].DisplayTotal
                             = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayPoint");
                if(subSectionNode)
                {
                     pointId = 0;
                     XMLElement * pointNode = subSectionNode->FirstChildElement("Point");
                     while(pointNode)
                     {
                      //  qDebug()<<pointNode->Attribute("index");
                        if(pointNode->FirstChildElement("Name"))
                        {
                       //     qDebug()<<pointNode->FirstChildElement("Name")->GetText();
                            strcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].Point[pointId].name,
                                   pointNode->FirstChildElement("Name")->GetText());
                        }

                        if(pointNode->FirstChildElement("Addr"))
                        {
                       //     qDebug()<<pointNode->FirstChildElement("Addr")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].Point[pointId].addr =
                                    atoi(pointNode->FirstChildElement("Addr")->GetText());
                        }

                        if(pointNode->FirstChildElement("Comment"))
                        {
                       //     qDebug()<<pointNode->FirstChildElement("Comment")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].Point[pointId].comment,
                                   pointNode->FirstChildElement("Comment")->GetText(),
                                   strlen(pointNode->FirstChildElement("Comment")->GetText()));
                        }

                        if(pointNode->FirstChildElement("SIZE"))
                        {
                       //     qDebug()<<pointNode->FirstChildElement("SIZE")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].Point[pointId].bitsnum =
                                    atoi(pointNode->FirstChildElement("SIZE")->GetText());
                        }

                        if(pointNode->FirstChildElement("RATIO"))
                        {
                        //    qDebug()<<pointNode->FirstChildElement("RATIO")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].Point[pointId].Ratio =
                                    atof(pointNode->FirstChildElement("RATIO")->GetText());
                        }

                        if(pointNode->FirstChildElement("BitIndex"))
                        {
                        //    qDebug()<<pointNode->FirstChildElement("BitIndex")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureSection[tGroupId].Point[pointId].BitIndex =
                                    atoi(pointNode->FirstChildElement("BitIndex")->GetText());
                        }

                        pointId++;
                        pointNode = pointNode->NextSiblingElement("Point");
                     }
                }
//                qDebug()<<"debug"<<ProtoIndex<<ProtoVer;
//                for(int testcnt=0;testcnt<valid_cfg_info.data[ProtoIndex][ProtoVer].MeasureSection1.DisplayTotal;testcnt++)
//                {
//                       qDebug()<<valid_cfg_info.data[ProtoIndex][ProtoVer].MeasureSection1.Point[testcnt].name
//                               <<valid_cfg_info.data[ProtoIndex][ProtoVer].MeasureSection1.Point[testcnt].addr
//                              <<valid_cfg_info.data[ProtoIndex][ProtoVer].MeasureSection1.Point[testcnt].bitsnum
//                              <<valid_cfg_info.data[ProtoIndex][ProtoVer].MeasureSection1.Point[testcnt].comment
//                               <<valid_cfg_info.data[ProtoIndex][ProtoVer].MeasureSection1.Point[testcnt].BitIndex;

//                }
                tGroupId++;
                SectionNode = SectionNode->NextSiblingElement("MeasureGroup");
            }

            SectionNode = subNode->FirstChildElement("ElecDegree");
            if(SectionNode)
            {
                XMLElement * subSectionNode = SectionNode->FirstChildElement("STARTADDR");
                if(subSectionNode)
                {
                   //  qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeSection.startaddr =
                             atoi(subSectionNode->GetText());

                }

                subSectionNode = SectionNode->FirstChildElement("CommuTotal");
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeSection.CommuTotal =
                             atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayTotal");
                if(subSectionNode)
                {
                    // qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeSection.DisplayTotal =
                             atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayPoint");
                if(subSectionNode)
                {
                     pointId = 0;
                     XMLElement * pointNode = subSectionNode->FirstChildElement("Point");
                     while(pointNode)
                     {
                      //  qDebug()<<pointNode->Attribute("index");
                        if(pointNode->FirstChildElement("Name"))
                        {
                       //     qDebug()<<pointNode->FirstChildElement("Name")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeSection.Point[pointId].name,
                                   pointNode->FirstChildElement("Name")->GetText(),
                                   strlen(pointNode->FirstChildElement("Name")->GetText()));
                        }

                        if(pointNode->FirstChildElement("SIZE"))
                        {
                       //     qDebug()<<pointNode->FirstChildElement("SIZE")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeSection.Point[pointId].bitsnum =
                                    atoi(pointNode->FirstChildElement("SIZE")->GetText());
                        }

                        if(pointNode->FirstChildElement("RATIO"))
                        {
                        //    qDebug()<<pointNode->FirstChildElement("RATIO")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeSection.Point[pointId].ratio =
                                    atoi(pointNode->FirstChildElement("RATIO")->GetText());

                        }

                        if(pointNode->FirstChildElement("Addr"))
                        {
                         //   qDebug()<<pointNode->FirstChildElement("Addr")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeSection.Point[pointId].addr =
                                    atoi(pointNode->FirstChildElement("Addr")->GetText());
                        }

                        if(pointNode->FirstChildElement("Comment"))
                        {
                         //   qDebug()<<pointNode->FirstChildElement("Comment")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeSection.Point[pointId].comment,
                                   pointNode->FirstChildElement("Comment")->GetText(),
                                   strlen(pointNode->FirstChildElement("Comment")->GetText()));
                        }

                        pointId++;
                        pointNode = pointNode->NextSiblingElement("Point");
                     }
                }
            }

            SectionNode = subNode->FirstChildElement("FaultParse");
            if(SectionNode)
            {
                XMLElement * subSectionNode = SectionNode->FirstChildElement("Section");
                if(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->GetText();
                    memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseSection.SectionName,
                           subSectionNode->GetText(),
                           strlen(subSectionNode->GetText()));
                }
                subSectionNode = SectionNode->FirstChildElement("GroupId");
                if(subSectionNode)
                {
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseSection.GroupId = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("Total");
                if(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseSection.Total = atoi(subSectionNode->GetText());
                }

                pointId = 0;
                subSectionNode = SectionNode->FirstChildElement("PointIndex");
                while(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->Attribute("index")
                  //          <<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseSection.MapIndex[pointId] =
                            atoi(subSectionNode->GetText());

                    pointId++;
                    subSectionNode = subSectionNode->NextSiblingElement("PointIndex");
                }
            }

            SectionNode = subNode->FirstChildElement("SwitchStatusParse");
            if(SectionNode)
            {
                XMLElement * subSectionNode = SectionNode->FirstChildElement("Section");
                if(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->GetText();
                    memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchStatusParseSection.SectionName,
                           subSectionNode->GetText(),
                           strlen(subSectionNode->GetText()));
                }
                subSectionNode = SectionNode->FirstChildElement("GroupId");
                if(subSectionNode)
                {
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchStatusParseSection.GroupId = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("PointIndex");
                if(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchStatusParseSection.MapIndex =
                            atoi(subSectionNode->GetText());
                }
            }

            SectionNode = subNode->FirstChildElement("VoltageandCurrentParse");
            if(SectionNode)
            {
                XMLElement * subSectionNode = SectionNode->FirstChildElement("Section");
                if(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->GetText();
                    memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].VoltageCurrentParseSection.SectionName,
                           subSectionNode->GetText(),
                           strlen(subSectionNode->GetText()));
                }

                subSectionNode = SectionNode->FirstChildElement("GroupId");
                if(subSectionNode)
                {
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].VoltageCurrentParseSection.GroupId = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("Voltage");
                if(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].VoltageCurrentParseSection.vindex = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("Current");
                if(subSectionNode)
                {
                 //   qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].VoltageCurrentParseSection.Iindex = atoi(subSectionNode->GetText());
                }
            }
        }

//        qDebug()<<"PRIVATE----------------------";
        subNode = root->FirstChildElement("PRIVATE");

        if(subNode)
        {
          //  qDebug()<<"PRIVATE MeasureGroup1----------------------";
            XMLElement *SectionNode = subNode->FirstChildElement("MeasureGroup");
            tGroupId = 0;
            while(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("STARTADDR");
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].startaddr =
                             atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("CommuTotal");
                if(subSectionNode)
                {
                //    qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].CommuTotal =
                            atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayTotal");
                if(subSectionNode)
                {
                //    qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].DisplayTotal =
                            atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayPoint");
                if(subSectionNode)
                {
                    pointId = 0;
                    XMLElement *PointNode = subSectionNode->FirstChildElement("Point");
                    while(PointNode)
                    {
                   //     qDebug()<<PointNode->Attribute("index");
                        if(PointNode->FirstChildElement("Name"))
                        {
                    //        qDebug()<<PointNode->FirstChildElement("Name")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].Point[pointId].name,
                                   PointNode->FirstChildElement("Name")->GetText(),
                                   strlen(PointNode->FirstChildElement("Name")->GetText()));
                        }

                        if(PointNode->FirstChildElement("SIZE"))
                        {
                     //       qDebug()<<PointNode->FirstChildElement("SIZE")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].Point[pointId].bitsnum =
                                    atoi(PointNode->FirstChildElement("SIZE")->GetText());
                        }

                        if(PointNode->FirstChildElement("RATIO"))
                        {
                      //      qDebug()<<PointNode->FirstChildElement("RATIO")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].Point[pointId].Ratio =
                                    atof(PointNode->FirstChildElement("RATIO")->GetText());
                        }

                        if(PointNode->FirstChildElement("BitIndex"))
                        {
                      //      qDebug()<<PointNode->FirstChildElement("BitIndex")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].Point[pointId].BitIndex =
                                    atoi(PointNode->FirstChildElement("BitIndex")->GetText());
                        }

                        if(PointNode->FirstChildElement("Addr"))
                        {
                       //     qDebug()<<PointNode->FirstChildElement("Addr")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].Point[pointId].addr =
                                    atoi(PointNode->FirstChildElement("Addr")->GetText());
                        }

                        if(PointNode->FirstChildElement("Comment"))
                        {
                      //      qDebug()<<PointNode->FirstChildElement("Comment")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].MeasureInternSection[tGroupId].Point[pointId].comment,
                                   PointNode->FirstChildElement("Comment")->GetText(),
                                   strlen(PointNode->FirstChildElement("Comment")->GetText()));
                        }

                      //  qDebug()<<"1111111111111111####"<<pointId<<ProtoIndex<<ProtoVer;
                        pointId++;
                        PointNode = PointNode->NextSiblingElement("Point");
                    }
                }
                tGroupId++;
                SectionNode = SectionNode->NextSiblingElement("MeasureGroup");
            }

           // qDebug()<<"PRIVATE ElecDegree----------------------";
            SectionNode = subNode->FirstChildElement("ElecDegree");
            if(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("STARTADDR");
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeInternSection.startaddr =
                             atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("CommuTotal");
                if(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeInternSection.CommuTotal =
                            atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayTotal");
                if(subSectionNode)
                {
                  //  qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeInternSection.DisplayTotal =
                            atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayPoint");
                if(subSectionNode)
                {
                    pointId = 0;
                    XMLElement *PointNode = subSectionNode->FirstChildElement("Point");
                    while(PointNode)
                    {
                     //   qDebug()<<PointNode->Attribute("index");

                        if(PointNode->FirstChildElement("Name"))
                        {
                       //     qDebug()<<PointNode->FirstChildElement("Name")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeInternSection.Point[pointId].name,
                                   PointNode->FirstChildElement("Name")->GetText(),
                                   strlen(PointNode->FirstChildElement("Name")->GetText()));
                        }

                        if(PointNode->FirstChildElement("SIZE"))
                        {
                        //    qDebug()<<PointNode->FirstChildElement("SIZE")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeInternSection.Point[pointId].bitsnum =
                                    atoi(PointNode->FirstChildElement("SIZE")->GetText());
                        }

                        if(PointNode->FirstChildElement("RATIOorBITS"))
                        {
                         //   qDebug()<<PointNode->FirstChildElement("RATIOorBITS")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeInternSection.Point[pointId].ratio =
                                    atoi(PointNode->FirstChildElement("RATIOorBITS")->GetText());
                        }

                        if(PointNode->FirstChildElement("Addr"))
                        {
                        //    qDebug()<<PointNode->FirstChildElement("Addr")->GetText();
                            gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeInternSection.Point[pointId].addr =
                                    atoi(PointNode->FirstChildElement("Addr")->GetText());
                        }

                        if(PointNode->FirstChildElement("Comment"))
                        {
                        //    qDebug()<<PointNode->FirstChildElement("Comment")->GetText();
                            memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ElecDegreeInternSection.Point[pointId].comment,
                                   PointNode->FirstChildElement("Comment")->GetText(),
                                   strlen(PointNode->FirstChildElement("Comment")->GetText()));
                        }

                        pointId++;
                        PointNode = PointNode->NextSiblingElement("Point");
                    }
                }
            }

         //   qDebug()<<"PRIVATE FaultParse----------------------";
            SectionNode = subNode->FirstChildElement("FaultParse");
            if(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("Section");
                if(subSectionNode)
                {
                    qDebug()<<"####"<<subSectionNode->GetText();
                    memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseInternSection.SectionName,
                           subSectionNode->GetText(),
                           strlen(subSectionNode->GetText()));

                    qDebug()<<"####"<<gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseInternSection.SectionName;
                }

                subSectionNode = SectionNode->FirstChildElement("GroupId");
                if(subSectionNode)
                {
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseInternSection.GroupId = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("Total");
                if(subSectionNode)
                {
                 //   qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseInternSection.Total =
                            atoi(subSectionNode->GetText());
                }

                pointId = 0;
                subSectionNode = SectionNode->FirstChildElement("PointIndex");
                while(subSectionNode)
                {
                //    qDebug()<<subSectionNode->Attribute("index")
                //            <<subSectionNode->GetText();

                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultParseInternSection.MapIndex[pointId] =
                            atoi(subSectionNode->GetText());



                    pointId++;
                    subSectionNode = subSectionNode->NextSiblingElement("PointIndex");
                }
            }

            SectionNode = subNode->FirstChildElement("SwitchStatusParse");
            if(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("Section");
                if(subSectionNode)
                {
                //    qDebug()<<subSectionNode->GetText();
                    memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchStatusParseInternSection.SectionName,
                           subSectionNode->GetText(),
                           strlen(subSectionNode->GetText()));
                }

                subSectionNode = SectionNode->FirstChildElement("GroupId");
                if(subSectionNode)
                {
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchStatusParseInternSection.GroupId = atoi(subSectionNode->GetText());
                }

                subSectionNode = subSectionNode->NextSiblingElement("PointIndex");
                if(subSectionNode)
                {
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchStatusParseInternSection.MapIndex =
                            atoi(subSectionNode->GetText());
                 //   qDebug()<<subSectionNode->GetText();
                }
            }

            SectionNode = subNode->FirstChildElement("VoltageandCurrentParse");
            if(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("Section");
                if(subSectionNode)
                {
                 //   qDebug()<<subSectionNode->GetText();
                    memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].VoltageCurrentParseInternSection.SectionName,
                           subSectionNode->GetText(),
                           strlen(subSectionNode->GetText()));
                }

                subSectionNode = SectionNode->FirstChildElement("GroupId");
                if(subSectionNode)
                {
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].VoltageCurrentParseInternSection.GroupId = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("Voltage");
                if(subSectionNode)
                {
                 //   qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].VoltageCurrentParseInternSection.vindex =
                            atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("Current");
                if(subSectionNode)
                {
                 //   qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].VoltageCurrentParseInternSection.Iindex =
                            atoi(subSectionNode->GetText());
                }
            }
            SectionNode = subNode->FirstChildElement("PowerDownStamp");
            if(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("Section");
                if(subSectionNode)
                {
                 //   qDebug()<<subSectionNode->GetText();
                    memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].PowerDownStampParseSection.SectionName,
                           subSectionNode->GetText(),
                           strlen(subSectionNode->GetText()));
                }

                subSectionNode = SectionNode->FirstChildElement("GroupId");
                if(subSectionNode)
                {
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].PowerDownStampParseSection.GroupId = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("PointIndex");
                if(subSectionNode)
                {
                 //   qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].PowerDownStampParseSection.MapIndex =
                            atoi(subSectionNode->GetText());
                }
            }
        }

       // qDebug()<<"COMMON----------------------";
        subNode = root->FirstChildElement("COMMON");

        if(subNode)
        {
            XMLElement *SectionNode = subNode->FirstChildElement("ConstantGroup");
            tGroupId = 0;
            while(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("FunCodeW");//FunCodeW
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].funcode =
                             atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("STARTADDR");
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].startaddr =
                             atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("CommuTotal");
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].CommuTotal =
                             atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayTotal");
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].DisplayTotal =
                             atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayPoint");
                if(subSectionNode)
                {
                     pointId = 0;
                     XMLElement * pointNode = subSectionNode->FirstChildElement("Point");
                     while(pointNode)
                     {
                       //  qDebug()<<pointNode->Attribute("index");

                         if(pointNode->FirstChildElement("Name"))
                         {
                       //      qDebug()<<pointNode->FirstChildElement("Name")->GetText();
                             memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].Point[pointId].name,
                                    pointNode->FirstChildElement("Name")->GetText(),
                                    strlen(pointNode->FirstChildElement("Name")->GetText()));
                         }

                         if(pointNode->FirstChildElement("SIZE"))
                         {
                         //    qDebug()<<pointNode->FirstChildElement("SIZE")->GetText();
                             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].Point[pointId].bitsnum =
                                     atoi(pointNode->FirstChildElement("SIZE")->GetText());
                         }

                         if(pointNode->FirstChildElement("RATIO"))
                         {
                         //    qDebug()<<pointNode->FirstChildElement("RATIO")->GetText();
                             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].Point[pointId].Ratio =
                                     atof(pointNode->FirstChildElement("RATIO")->GetText());
                         }

                         if(pointNode->FirstChildElement("BitIndex"))
                         {
                         //    qDebug()<<pointNode->FirstChildElement("BitIndex")->GetText();
                             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].Point[pointId].BitIndex =
                                     atoi(pointNode->FirstChildElement("BitIndex")->GetText());
                         }

                         if(pointNode->FirstChildElement("Addr"))
                         {
                         //    qDebug()<<pointNode->FirstChildElement("Addr")->GetText();
                             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].Point[pointId].addr =
                                     atoi(pointNode->FirstChildElement("Addr")->GetText());
                         }

                         if(pointNode->FirstChildElement("Comment"))
                         {
                          //   qDebug()<<pointNode->FirstChildElement("Comment")->GetText();
                             memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].ConstantSection[tGroupId].Point[pointId].comment,
                                     pointNode->FirstChildElement("Comment")->GetText(),
                                     strlen(pointNode->FirstChildElement("Comment")->GetText()));
                         }

                         pointId++;
                         pointNode = pointNode->NextSiblingElement("Point");
                     }
                 }
                tGroupId++;
                SectionNode = SectionNode->NextSiblingElement("ConstantGroup");
            }

            SectionNode = subNode->FirstChildElement("Fault");
            if(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("STARTADDR");
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Faultsection.startaddr = atoi(subSectionNode->GetText());
                }
                subSectionNode = SectionNode->FirstChildElement("CommuTotal");
                if(subSectionNode)
                {
                  //   qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Faultsection.CommuTotal = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayTotal");
                if(subSectionNode)
                {
                   //  qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Faultsection.DisplayTotal = atoi(subSectionNode->GetText());
                }

                subSectionNode = SectionNode->FirstChildElement("DisplayPoint");
                if(subSectionNode)
                {
                     pointId = 0;
                     XMLElement * pointNode = subSectionNode->FirstChildElement("Point");
                     while(pointNode)
                     {
                     //    qDebug()<<pointNode->Attribute("index");

                         if(pointNode->FirstChildElement("Name"))
                         {
                      //       qDebug()<<pointNode->FirstChildElement("Name")->GetText();
                             memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Faultsection.Point[pointId].name,
                                    pointNode->FirstChildElement("Name")->GetText(),
                                    strlen(pointNode->FirstChildElement("Name")->GetText()));
                         }

                         if(pointNode->FirstChildElement("SIZE"))
                         {
                       //      qDebug()<<pointNode->FirstChildElement("SIZE")->GetText();
                             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Faultsection.Point[pointId].bitsnum =
                                     atoi(pointNode->FirstChildElement("SIZE")->GetText());
                         }

                         if(pointNode->FirstChildElement("RATIO"))
                         {
                       //      qDebug()<<pointNode->FirstChildElement("RATIO")->GetText();
                             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Faultsection.Point[pointId].BitIndex =
                                     atoi(pointNode->FirstChildElement("RATIO")->GetText());
                         }

                         if(pointNode->FirstChildElement("Addr"))
                         {
                        //     qDebug()<<pointNode->FirstChildElement("Addr")->GetText();
                             gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Faultsection.Point[pointId].addr =
                                     atoi(pointNode->FirstChildElement("Addr")->GetText());
                         }

                         if(pointNode->FirstChildElement("Comment"))
                         {
                         //    qDebug()<<pointNode->FirstChildElement("Comment")->GetText();
                             memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].Faultsection.Point[pointId].comment,
                                    pointNode->FirstChildElement("Comment")->GetText(),
                                    strlen(pointNode->FirstChildElement("Comment")->GetText()));
                         }

                         pointId++;
                         pointNode = pointNode->NextSiblingElement("Point");
                     }
                 }
            }
            SectionNode = subNode->FirstChildElement("TimeSync");
            if(SectionNode)
            {
                if(SectionNode->FirstChildElement("ENABLE"))
                {
                //    qDebug()<<SectionNode->FirstChildElement("ENABLE")->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TimeSyncSection.enable = atoi(SectionNode->FirstChildElement("ENABLE")->GetText());
                }

                if(SectionNode->FirstChildElement("YEAR"))
                {
                //    qDebug()<<SectionNode->FirstChildElement("YEAR")->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TimeSyncSection.addr[0] = atoi(SectionNode->FirstChildElement("YEAR")->GetText());
                }

                if(SectionNode->FirstChildElement("MON"))
                {
                //    qDebug()<<SectionNode->FirstChildElement("MON")->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TimeSyncSection.addr[1] = atoi(SectionNode->FirstChildElement("MON")->GetText());
                }

                if(SectionNode->FirstChildElement("DAY"))
                {
                //    qDebug()<<SectionNode->FirstChildElement("DAY")->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TimeSyncSection.addr[2] = atoi(SectionNode->FirstChildElement("DAY")->GetText());
                }

                if(SectionNode->FirstChildElement("HOUR"))
                {
                //    qDebug()<<SectionNode->FirstChildElement("HOUR")->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TimeSyncSection.addr[3] = atoi(SectionNode->FirstChildElement("HOUR")->GetText());
                }

                if(SectionNode->FirstChildElement("MIN"))
                {
                //    qDebug()<<SectionNode->FirstChildElement("MIN")->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TimeSyncSection.addr[4] = atoi(SectionNode->FirstChildElement("MIN")->GetText());
                }

                if(SectionNode->FirstChildElement("SEC"))
                {
                //    qDebug()<<SectionNode->FirstChildElement("SEC")->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].TimeSyncSection.addr[5] = atoi(SectionNode->FirstChildElement("SEC")->GetText());
                }
            }

            SectionNode = subNode->FirstChildElement("SwitchControl");

            if(SectionNode)
            {
                XMLElement *subSectionNode = SectionNode->FirstChildElement("TOTAL");
                if(subSectionNode)
                {
                //    qDebug()<<subSectionNode->GetText();
                    gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchControlSection.total =
                            atoi(subSectionNode->GetText());
                }

                subSectionNode = subSectionNode->NextSiblingElement("CHANNEL");
                tChannelId = 0;
                while(subSectionNode)
                {
                    if(subSectionNode->FirstChildElement("ADDR"))
                    {
                    //     qDebug()<<subSectionNode->FirstChildElement("ADDR")->GetText();
                         gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchControlSection.Point[tChannelId].addr =
                                 atoi(subSectionNode->FirstChildElement("ADDR")->GetText());
                    }

                    if(subSectionNode->FirstChildElement("CMD"))
                    {
                         qDebug()<<subSectionNode->FirstChildElement("CMD")->GetText();
                         gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchControlSection.Point[tChannelId].cmd =
                                 atoi(subSectionNode->FirstChildElement("CMD")->GetText());
                    }

                    if(subSectionNode->FirstChildElement("DESC"))
                    {
                        memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].SwitchControlSection.Point[pointId].Desc,
                               subSectionNode->FirstChildElement("DESC")->GetText(),
                               strlen(subSectionNode->FirstChildElement("DESC")->GetText()));
                    }

                    tChannelId++;
                    subSectionNode = subSectionNode->NextSiblingElement("CHANNEL");
                }                    
            }

            SectionNode = subNode->FirstChildElement("FaultCode");
            tGroupId = 0;
            while(SectionNode)
            {
                 XMLElement * subSectionNode = SectionNode->FirstChildElement("TOTAL");
                 if(subSectionNode)
                 {
                   //  qDebug()<<subSectionNode->GetText();
                     gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultCodeSection[tGroupId].total =
                             atoi(subSectionNode->GetText());
                 }

                 subSectionNode = subSectionNode->NextSiblingElement("CodeDesc");
                 pointId = 0;
                 while(subSectionNode)
                 {
                   //  qDebug()<<subSectionNode->Attribute("index");
                     if(subSectionNode->FirstChildElement("CODE"))
                     {
                     //    qDebug()<<subSectionNode->FirstChildElement("CODE")->GetText();
                         gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultCodeSection[tGroupId].Point[pointId].Code =
                                 atoi(subSectionNode->FirstChildElement("CODE")->GetText());
                     }

                     if(subSectionNode->FirstChildElement("DESC"))
                     {
                     //    qDebug()<<subSectionNode->FirstChildElement("CODE")->GetText();
                         memcpy(gDeviceDataBase.SysCfgData.ProtoCfgData[ProtoIndex][ProtoVer].FaultCodeSection[tGroupId].Point[pointId].name,
                                subSectionNode->FirstChildElement("CODE")->GetText(),
                                strlen(subSectionNode->FirstChildElement("CODE")->GetText()));
                     }
                     pointId++;
                     subSectionNode = subSectionNode->NextSiblingElement("CodeDesc");
                 }

                 tGroupId++;
                 SectionNode = SectionNode->NextSiblingElement("FaultCode");
            }

        }
    }
}




void centralCtl::ReadFromUsrListFile()
{
    gDeviceDataBase.UsrList.adminlvl.level = 0;
    gDeviceDataBase.UsrList.adminlvl.name = 0;
    gDeviceDataBase.UsrList.adminlvl.passwd = 0;

    if(gDeviceDataBase.UsrList.engineerlvl.isEmpty() != 0)
        gDeviceDataBase.UsrList.engineerlvl.clear();
    if(gDeviceDataBase.UsrList.operaterlvl.isEmpty() != 0)
        gDeviceDataBase.UsrList.operaterlvl.clear();

    UsrInfo singleusrinfo;
    memset(&singleusrinfo,0x00,sizeof(UsrInfo));

    XMLDocument doc;
#ifdef HBDEBUG
    XMLError ret = doc.LoadFile("/home/imx6/Desktop/debugFolder/configfiles/usrinfo/usrpwdlist.xml");
#else
    XMLError ret = doc.LoadFile("/waterpump/gaspumpconfigfiles/usrinfo/usrpwdlist.xml");
#endif
    if(ret != XML_SUCCESS)
    {
        qDebug()<<"load usrpwdlist xml error"<<ret;
        return;
    }
    XMLElement* root  = doc.RootElement();
    if(root)
    {
        XMLElement* Node = root->FirstChildElement("admin");
        if(Node)
        {
            XMLElement *subNode = Node->FirstChildElement("name");
            if(subNode)
            {
                gDeviceDataBase.UsrList.adminlvl.name = atoi(subNode->GetText());
            }
            subNode = Node->FirstChildElement("passwd");
            if(subNode)
            {
                gDeviceDataBase.UsrList.adminlvl.passwd = atoi(subNode->GetText());
                gDeviceDataBase.UsrList.adminlvl.level = 1;
            }
        }

        Node = root->FirstChildElement("engineer");
        if(Node)
        {
            XMLElement *subengNode = Node->FirstChildElement("usrinfo");
            while(subengNode)
            {
                XMLElement *usrNode = subengNode->FirstChildElement("name");
                if(usrNode)
                {
                    //memset(&singleusrinfo,0x00,sizeof(usrinfo));
                    singleusrinfo.name = atoi(usrNode->GetText());
                }
                usrNode = subengNode->FirstChildElement("passwd");
                if(usrNode)
                {
                    singleusrinfo.passwd = atoi(usrNode->GetText());
                    singleusrinfo.level = 2;
                    gDeviceDataBase.UsrList.engineerlvl.append(singleusrinfo);
//                    qDebug()<<"usrinfo"<<singleusrinfo.name<<singleusrinfo.passwd;
                }
                subengNode = subengNode->NextSiblingElement("usrinfo");
            }
        }

        Node = root->FirstChildElement("operat");
        if(Node)
        {
            XMLElement *suboptNode = Node->FirstChildElement("usrinfo");
            while(suboptNode)
            {
                XMLElement *usrNode = suboptNode->FirstChildElement("name");
                if(usrNode)
                {
//                    memset(&singleusrinfo,0x00,sizeof(usrinfo));
                    singleusrinfo.name = atoi(usrNode->GetText());
                }
                usrNode = suboptNode->FirstChildElement("passwd");
                if(usrNode)
                {
                    singleusrinfo.passwd = atoi(usrNode->GetText());
                    singleusrinfo.level = 3;

                    gDeviceDataBase.UsrList.operaterlvl.append(singleusrinfo);
                }

                suboptNode = suboptNode->NextSiblingElement("usrinfo");
            }
        }
    }
}

void centralCtl::savetimertimeout()
{
#if 0
  //  qDebug()<<"savetimertimeout"<<faultHisTorylist.count()<<HistoryFaultSummaryIns.copybusyflag;
   // if(HistoryFaultSummaryIns.copybusyflag == 0)
    {
       // HistoryFaultSummaryIns.copybusyflag = 1;
        for(int i=0;i < faultHisTorylist.count();i++)
        {
            faultSavebufferlist.append(faultHisTorylist.at(i));
        }

    //    qDebug()<<"savetimertimeout"<<faultHisTorylist.count()<<HistoryFaultSummaryIns.copybusyflag<<faultSavebufferlist.count();
        faultHisTorylist.clear();
        savefaulttofaultfile();
        savefaultsummaryfaultfile();
      //  HistoryFaultSummaryIns.copybusyflag = 0;
    }
#endif

}
#if 0
void centralCtl::savefaulttofaultfile()
{
    //make sure how many currently
    int SerialCodeFile = HistoryFaultSummaryIns.currentIndex/200;
    int SerialCodeCurrentFaultFile = HistoryFaultSummaryIns.currentIndex%200;
    int newfaultnum = faultSavebufferlist.count();

//    qDebug()<<"savefaulttofaultfile"<<SerialCodeFile<<SerialCodeCurrentFaultFile<<newfaultnum;

    if(newfaultnum)
    {

        if(SerialCodeCurrentFaultFile + newfaultnum < 200)
        {    // save one fault file. FILEINDEX
            savetoonefaultfile(SerialCodeFile,SerialCodeCurrentFaultFile,newfaultnum);

        }
        else //save to two files. fileindex and fileindex+1
        {
            savetodoublefaultfile(SerialCodeFile,SerialCodeCurrentFaultFile,newfaultnum);
            HistoryFaultSummaryIns.filenodenum++;
        }
//        updatefaultlist();
        HistoryFaultSummaryIns.currentIndex += newfaultnum;
    }
}
#endif
void centralCtl::updatefaultlist()
{
//    //HistoryFaultSummaryIns.busyflag = 1;

//  //  int fileSerialCode = HistoryFaultSummaryIns.currentIndex/200;
//    int currentSerialCode = HistoryFaultSummaryIns.currentIndex%200;
//    int newfaultnum = faultSavebufferlist.count();

//  //  faultfilenode onefaultfilenode;
//    HistoryFaultSummaryIns.currentIndex += newfltnum;
//    if(currentSerialCode + newfaultnum >= 200)
//    {
//       HistoryFaultSummaryIns.filenodenum++;
//    }

//    qDebug()<<HistoryFaultSummaryIns.currentIndex<<HistoryFaultSummaryIns.filenodenum<<"updatefaultlist";

}
#if 0
void centralCtl::savefaultsummaryfaultfile()
{
 //   int total = 0;
    XMLDocument doc;
    XMLError ret = doc.LoadFile("/substation/configfiles/historyfault/historyfaultlist.xml");
    XMLElement * root = doc.RootElement();

    if(ret != XML_SUCCESS)
    {
        qDebug()<<"failed"<<ret<<__LINE__;
        return;
    }
   // qDebug()<<"load fault summary success"<<ret<<HistoryFaultSummaryIns.currentIndex<<HistoryFaultSummaryIns.filenodenum<<"savefaultsummaryfaultfile";;

    if(root)
    {
        XMLElement* Node = root->FirstChildElement("currentIndex");
        if(Node)
        {
             Node->SetText(HistoryFaultSummaryIns.currentIndex);
        }
        Node = root->FirstChildElement("filenodenum");
        if(Node)
        {
             Node->SetText(HistoryFaultSummaryIns.filenodenum);
        }
    }
    doc.SaveFile("/substation/configfiles/historyfault/historyfaultlist.xml");
}
#endif
#if 0
void centralCtl::savetoonefaultfile(int fileScode,int currentScode,int newfaultnum)
{
    int total = 0;
    int startindex = 0;
    std::string midstr;

    XMLElement * faultnode;
    XMLElement * timenode;
    XMLElement * devnamenode;
    XMLElement * devtypenode;
    XMLElement * phyaddrnode;
    XMLElement * faultnamenode;
    XMLText    * pText;

    QString srcfilenamestr = tr("/substation/configfiles/historyfault/historyfault%1.xml").arg(fileScode);

    midstr = srcfilenamestr.toStdString();
    const char *descfilenamestr = midstr.c_str();

    XMLDocument doc;
    XMLError ret = doc.LoadFile(descfilenamestr);

    XMLElement * root = doc.RootElement();

    if(ret != XML_SUCCESS)
    {
        qDebug()<<"failed"<<ret<<__LINE__<<descfilenamestr;
        return;
    }
  //  qDebug()<<"load success historyfault"<<ret<<descfilenamestr<<newfaultnum;

    if(root)
    {
        XMLElement* Node = root->FirstChildElement("total");
        if(Node)
        {    total = atoi(Node->GetText());
             startindex = total;
             total += newfaultnum;
             Node->SetText(total);
        }
#if 1
        for(int i=0;i < newfaultnum;i++)
        {

            faultnode = doc.NewElement("faultpoint");
            faultnode->SetAttribute("id",startindex+i);

            timenode = doc.NewElement("time");
            std::string midstr1 = faultSavebufferlist.at(i).time.toStdString();
            const char *descstr1 = midstr1.c_str();
            pText = doc.NewText(descstr1);
//            qDebug()<<descstr1<<"descstr";
            timenode->InsertFirstChild(pText);

            faultnode->InsertFirstChild(timenode);

#if 1
            devnamenode = doc.NewElement("devname");
            midstr1 = faultSavebufferlist.at(i).devname.toStdString();
            const char *descstr2 = midstr1.c_str();
            pText = doc.NewText(descstr2);
            //qDebug()<<descstr<<"descstr";
            devnamenode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(devnamenode);

            devtypenode = doc.NewElement("devtype");
            midstr1 = faultSavebufferlist.at(i).devtype.toStdString();
            const char *descstr3 = midstr1.c_str();
            pText = doc.NewText(descstr3);
           // qDebug()<<descstr<<"descstr";
            devtypenode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(devtypenode);

            phyaddrnode = doc.NewElement("phyaddr");
            midstr1 = QString::number(faultSavebufferlist.at(i).phyaddr).toStdString();
            const char *descstr4 = midstr1.c_str();
            pText = doc.NewText(descstr4);
           // qDebug()<<descstr<<"descstr";
            phyaddrnode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(phyaddrnode);

            faultnamenode = doc.NewElement("faultname");
           // midstr1 = faultSavebufferlist.at(i).faultname.toStdString();
           // const char *descstr5 = midstr1.c_str();
            pText = doc.NewText(faultSavebufferlist.at(i).faultnamearray);
            faultnamenode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(faultnamenode);
#endif
            root->InsertEndChild(faultnode);
        }
#endif
    }

    int result = -1;
    result = doc.SaveFile(descfilenamestr);
    faultSavebufferlist.clear();
   // qDebug()<<"save result to check = "<<result<<descfilenamestr;
}
#endif
#if 0
void centralCtl::savetodoublefaultfile(int fileScode,int currentScode,int newfaultnum)
{
    QString srcstr;
    std::string midstr;
    const char *descstr;
    std::string midfilenamestr;
    const char *descfilenamestr;

    int total = 0;
    int startindex = 0;


    XMLElement * faultnode;
    XMLElement * timenode;
    XMLElement * devnamenode;
    XMLElement * devtypenode;
    XMLElement * phyaddrnode;
    XMLElement * faultnamenode;
    XMLText    * pText;

    QString srcfilenamestr = tr("/substation/configfiles/historyfault/historyfault%1.xml").arg(fileScode);

    midfilenamestr = srcfilenamestr.toStdString();
    descfilenamestr = midfilenamestr.c_str();

    XMLDocument doc;
    XMLError ret = doc.LoadFile(descfilenamestr);
    XMLElement * root = doc.RootElement();

    if(ret != XML_SUCCESS)
    {
        qDebug()<<"failed"<<ret<<__LINE__;
        return;
    }
    qDebug()<<"load double success"<<ret<<descfilenamestr;

    if(root)
    {
        XMLElement* Node = root->FirstChildElement("total");
        if(Node)
        {    total = atoi(Node->GetText());
             startindex = total;
             total = 200;
             Node->SetText(total);
        }

        for(int i=0;i < 200-currentScode;i++) // save currentid ->200 to faultxxx.xml
        {
            faultnode = doc.NewElement("faultpoint");
            faultnode->SetAttribute("id",startindex+i);
            root->InsertEndChild(faultnode);

            timenode = doc.NewElement("time");
            midstr = faultSavebufferlist.at(i).time.toStdString();
            descstr = midstr.c_str();
            pText = doc.NewText(descstr);
            timenode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(timenode);

            devnamenode = doc.NewElement("devname");
            midstr = faultSavebufferlist.at(i).devname.toStdString();
            descstr = midstr.c_str();
            pText = doc.NewText(descstr);
            devnamenode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(devnamenode);

            devtypenode = doc.NewElement("devtype");
            midstr = faultSavebufferlist.at(i).devtype.toStdString();
            descstr = midstr.c_str();
            pText = doc.NewText(descstr);
            devtypenode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(devtypenode);

            phyaddrnode = doc.NewElement("phyaddr");
            midstr = QString::number(faultSavebufferlist.at(i).phyaddr).toStdString();
            descstr = midstr.c_str();
            pText = doc.NewText(descstr);
            phyaddrnode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(phyaddrnode);

            faultnamenode = doc.NewElement("faultname");
         //   midstr = faultSavebufferlist.at(i).faultname.toStdString();
           // descstr = midstr.c_str();
            pText = doc.NewText(faultSavebufferlist.at(i).faultnamearray);
            faultnamenode->InsertFirstChild(pText);
            faultnode->InsertFirstChild(faultnamenode);
        }
    }

    doc.SaveFile(descfilenamestr);

    XMLDocument doc2;
    //1.添加声明
    tinyxml2::XMLDeclaration* declaration = doc2.NewDeclaration();
    doc2.InsertFirstChild(declaration);

    //2.创建根节点
    tinyxml2::XMLElement* root2 = doc2.NewElement("historyfaultlist");
    doc2.InsertEndChild(root2);

    //3.创建子节点
    tinyxml2::XMLElement *totalnode  = doc2.NewElement("total");
    srcstr = QString::number(newfaultnum + startindex - 200);
    midstr = srcstr.toStdString();
    descstr = midstr.c_str();
    pText = doc2.NewText(descstr);
    root2->InsertEndChild(totalnode);

    for(int i=0;i < newfaultnum + startindex - 200;i++) // save leave to faultxxx+1.xml
    {
        faultnode = doc.NewElement("faultpoint");
        faultnode->SetAttribute("id",i+200*currentScode+1);
        root->InsertEndChild(faultnode);

        timenode = doc.NewElement("time");
        midstr = faultSavebufferlist.at(i + 200-currentScode).time.toStdString();
        descstr = midstr.c_str();
        pText = doc.NewText(descstr);
        timenode->InsertFirstChild(pText);
        faultnode->InsertFirstChild(timenode);

        devnamenode = doc.NewElement("devname");
        midstr = faultSavebufferlist.at(i + 200-currentScode).devname.toStdString();
        descstr = midstr.c_str();
        pText = doc.NewText(descstr);
        devnamenode->InsertFirstChild(pText);
        faultnode->InsertFirstChild(devnamenode);

        devtypenode = doc.NewElement("devtype");
        midstr = faultSavebufferlist.at(i + 200-currentScode).devtype.toStdString();
        descstr = midstr.c_str();
        pText = doc.NewText(descstr);
        devtypenode->InsertFirstChild(pText);
        faultnode->InsertFirstChild(devtypenode);

        phyaddrnode = doc.NewElement("phyaddr");
        midstr = QString::number(faultSavebufferlist.at(i + 200-currentScode).phyaddr).toStdString();
        descstr = midstr.c_str();
        pText = doc.NewText(descstr);
        phyaddrnode->InsertFirstChild(pText);
        faultnode->InsertFirstChild(phyaddrnode);

        faultnamenode = doc.NewElement("faultname");
       // midstr = faultSavebufferlist.at(i + 200-currentScode).faultname.toStdString();
       // descstr = midstr.c_str();
        pText = doc.NewText(faultSavebufferlist.at(i + 200-currentScode).faultnamearray);
        faultnamenode->InsertFirstChild(pText);
        faultnode->InsertFirstChild(faultnamenode);
    }

    faultSavebufferlist.clear();

    srcfilenamestr = tr("/substation/configfiles/historyfault/historyfault%1.xml").arg(fileScode+1);
    midfilenamestr = srcfilenamestr.toStdString();
    descfilenamestr = midfilenamestr.c_str();

    doc2.SaveFile(descfilenamestr);
}
#endif
#if 0
void centralCtl::readfaultsummaryfaultfile()
{
    XMLDocument doc;
    XMLError ret = doc.LoadFile("/substation/configfiles/historyfault/historyfaultlist.xml" );

    faultfilenode faultfilenodeIns;
    faultfilenodeIns.filename.clear();
    faultfilenodeIns.maxindex = 0;

    if(ret != XML_SUCCESS)
    {
        qDebug()<<"failed"<<ret<<__LINE__;
        return;
    }
    qDebug()<<"load success"<<ret;

    XMLElement* root = doc.RootElement();

    if(root)
    {
        XMLElement* Node = root->FirstChildElement("currentIndex");
        if(Node)
        {
             HistoryFaultSummaryIns.currentIndex = atoi(Node->GetText());
             qDebug()<<"HistoryFaultSummaryIns.currentIndex"<<HistoryFaultSummaryIns.currentIndex;
        }

        Node = Node->NextSiblingElement("filenodenum");
        if(Node)
        {
             HistoryFaultSummaryIns.filenodenum  = atoi(Node->GetText());
        }

//        if(HistoryFaultSummaryIns.filenodenum)
//        {
//            Node = Node->NextSiblingElement("filenode");
//            while(Node)
//            {
//                XMLElement* subNode = Node->FirstChildElement("maxIndex");
//                if(subNode)
//                {
//                    faultfilenodeIns.maxindex = atoi(subNode->GetText());
//                }

//                subNode = Node->FirstChildElement("filename");

//                if(subNode)
//                {
//                    faultfilenodeIns.filename =  QString(QLatin1String(subNode->GetText()));
//                }

//                HistoryFaultSummaryIns.filenodelist.append(faultfilenodeIns);
//                Node = Node->NextSiblingElement("filenode");
//            }
//        }
    }
    //doc.SaveFile("/substation/configfiles/historyfault/historyfaultlist.xml");
}
#endif
//void centralCtl::readfaultfile(int fileScode)
//{
    //read max 100 to display
   // int curfaultScode = p

//    int total = 0;

//    QString srcfilenamestr = tr("/substation/historyfault/historyfault%1.xml").arg(fileScode);

//    std::string midfilenamestr;
//    const char *descfilenamestr;

//    midfilenamestr = srcfilenamestr.toStdString();
//    descfilenamestr = midfilenamestr.c_str();

//    XMLDocument doc;
//    XMLError ret = doc.LoadFile(descfilenamestr);

//    if(ret != XML_SUCCESS)
//    {
//        qDebug()<<"failed"<<ret<<__LINE__;
//        return;
//    }
//    qDebug()<<"load success"<<ret;

//    XMLElement* root = doc.RootElement();

//    if(root)
//    {
//        XMLElement* Node = root->FirstChildElement("total");
//        if(Node)
//        {
//            total = atoi(Node->GetText());
//        }

//        Node = root->FirstChildElement("faultpoint");
//    }




//}
