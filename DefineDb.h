#ifndef _DEFINE_DB_H
#define _DEFINE_DB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/shm.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h> 
#include <sys/ipc.h> 
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>

#include <QString>
#include <QList>

#define ARM_PATHFORM 0
//#define HBDEBUG
/*数据定义*/
typedef unsigned char  u8;
typedef short          s16;
typedef unsigned short u16;
typedef int            i32;
typedef unsigned int   u32;

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

/*公用信息*/
typedef enum _GROUP_INFO_TYPE //实时信息
{	
    INFO_SWITCH          = 0,      //遥信
    INFO_MEASURE_1       = 1,      //模拟量1 - 6
    INFO_MEASURE_2       = 2,
    INFO_MEASURE_3       = 3,
    INFO_MEASURE_4       = 4,
    INFO_MEASURE_5       = 5,
    INFO_MEASURE_6       = 6,
    INFO_MEASURE_INTER_1 = 7,      //内部模拟量 1 - 2
    INFO_MEASURE_INTER_2 = 8,
    INFO_CONSTANT_1      = 9,      //定值 1 - 2
    INFO_CONSTANT_2      = 10,
    INFO_FAULT           = 11,     //故障信息
    INFO_ELECTRIC        = 12,     //电度信息
    INFO_OCONTENT        = 13,     //2016/2/23停电票信息
    INFO_MAX_FLAG        = 14,
}GROUP_INFO_TYPE;

typedef enum _GROUP_OPT_TYPE //操作类型
{
    OPT_OPERATE         = 100,     // 2016/2/23停电票操作
    OPT_TIMESYNC        = 101,     // 用于对时操作，广播对时2016/2/29
    OPT_GET_FAULT       = 102,     // 获取故障
    OPT_SETTING_READ    = 103,     // 读定值
    OPT_SETTING_WRITE   = 104,     // 写定值
    OPT_SWITCH_CONTROL  = 105,     // 控制分不同的通道
}GROUP_OPT_TYPE;

typedef enum _DATA_TYPE
{	
    DATA_SINGLE_BIT         = 1,
    DATA_DOUBLE_BIT         = 2,
    DATA_SINGLE_BYTE        = 8,
    DATA_DOUBLE_BYTE        = 16,
	DATA_DOUBLE_DOUBLE_BYTE = 32, 
}DATA_TYPE;

#define CONTROL_MAX_SELECT   4
#define CONTROL_RELAY_ON     0
#define CONTROL_RELAY_OFF    1
#define CONTROL_RESET        2
#define CONTROL_TEST         3

#define RELAY_STATUS_ON  1
#define RELAY_STATUS_OFF 0

/*设备信息*/
#define MAX_COM_NUM          6
#define MAX_COM_RTU_NUM     16
#define MAX_RTU_NUM          (MAX_COM_NUM * MAX_COM_RTU_NUM)
//#define MAX_CONNECT_NUM         2

/*数据信息*/
#define MAX_VAL_NUM    100// 值不超过200
/*串口信息*/
#define PORT1					0
#define PORT2					1
#define PORT3					2
#define PORT4					3
#define PORT5                   4
#define PORT6                   5
#define MAX_LEN_COM_BYTES 256
/*监控信息*/
#define HMI					    0
#define SERVER			        1

/*配置文件信息*/
typedef struct _COM_DATA
{
    INT32S speed;
    INT8U  parity;
    INT8U  databits;
    INT8U  stopbit;
}COM_DATA;

typedef struct _HMI_INFO
{
    INT8U relay;
    INT8U operate;
    INT8U code;
    INT8U voltage;
    INT8U current;
}HMI_INFO;

//typedef struct _FLAG_INFO
//{
//	unsigned _dis:1;//显示
//	unsigned _abs:1;//用于负数取绝对值
//	unsigned _group:3;//组号
//}FLAG_INFO;

/*-------------TeleAction start--------------------*/
typedef struct _TeleActionData
{
	u8 bitIndex;
    char name[50];
    char comment[50];
}TeleActionData;
typedef struct _TeleAction
{
	u8 funcode;
	u16 startaddr;
	u16 CommuTotal;
	u16 DisplayTotal;
	TeleActionData Point[MAX_VAL_NUM];
}TeleAction;

/*-------------Mesure start-----------------------*/
typedef struct _MeasureData
{
    char name[50];
    u8 bitsnum;  // 每个数据点的size 32/16/8/4/2/1
  //  u16 Ratio_BitIndex;
    INT16U BitIndex;
    FP32 Ratio;   // 比例因子，实际值=报文值/Ratio
	u16 addr;
    char comment[50];
}MeasureData;
typedef struct _Measure
{
    INT8U  funcode;
    INT16U startaddr;
    INT16U CommuTotal;
    INT16U DisplayTotal;
    MeasureData Point[MAX_VAL_NUM];
}StMeasure;

/*-------------Mesure stop-----------------------*/

/*-------------Constant start--------------------*/
typedef struct _ConstantData
{
	char name[50];
    u8 bitsnum;    // 每个数据点的size 32/16/8/4/2/1
//    u16 Ratio_BitIndex;
    INT16U BitIndex;
    FP32 Ratio;   // 比例因子，实际值=报文值/Ratio
	u16 addr;
	char comment[50];
}ConstantData;
typedef struct _Constant
{
    u8 funcode;
	u16 startaddr;
	u16 CommuTotal;
	u16 DisplayTotal;
    ConstantData Point[MAX_VAL_NUM];
}StConstant;

/*-------------Fault start----------------------------*/
typedef struct _FaultData
{
	char name[50];
    u8 bitsnum;     // 每个数据点的size 32/16/8/4/2/1
//    u16 Ratio_BitIndex;
    INT16U BitIndex;
    FP32 Ratio;   // 比例因子，实际值=报文值/Ratio
	u16 addr;
	char comment[50];
}FaultData;
typedef struct _Fault
{
	u16 startaddr;
	u16 CommuTotal;
	u16 DisplayTotal;
    FaultData Point[MAX_VAL_NUM];
}StFault;

/*-------------Fault parse start---------------------------*/
typedef struct _FaultParse
{
    char SectionName[50];
    INT8U GroupId;
    INT8U Total;
    INT8U MapIndex[32];
}FaultParse;
/*-------------Fault parse stop---------------------------*/

/*-------------SwitchStatusParse start---------------------------*/
typedef struct _SwitchStatusParse
{
	char SectionName[20];
    INT8U GroupId;
    INT8U Total;
    INT8U MapIndex;
}SwitchStatusParse;
/*-------------SwitchStatusParse stop---------------------------*/

/*-------------PowerDownStampParse start---------------------------*/
typedef struct _PowerDownStampParse
{
	char SectionName[20];
    INT8U GroupId;
    INT8U MapIndex;
}PowerDownStampParse;
/*-------------PowerDownStampParse stop---------------------------*/

/*-------------VoltageAndCurrentParse start-----------------------*/
typedef struct _VoltageCurrentParse
{
	char SectionName[20];
    INT8U GroupId;
    INT8U vindex;
    INT8U Iindex;
}VoltageCurrentParse;
/*-------------VoltageAndCurrentParse stop-----------------------*/

/*-------------ElecDegreeData start------------------------------*/
typedef struct _ElecDegreeData
{
    char name[50];
    u16 ratio;
    u8 bitsnum;      // 每个数据点的size 32/16/8/4/2/1
	u16 addr;
    char comment[50];
}ElecDegreeData;

typedef struct _ElecDegree
{
    u16 startaddr;
	u16 CommuTotal;
	u16 DisplayTotal;
    ElecDegreeData Point[20];
}ElecDegree;


/*-------------ElecDegreeData stop------------------------------*/

/*-------------TimeSync start-----------------------------------*/
typedef struct _TimeSync
{
    INT8U enable;
    INT16U addr[6];
}TimeSync;
/*-------------TimeSync stop-----------------------------------*/

/*-------------SwitchControl start-------------------------*/

#define MAX_CHANNEL_NUM 32
typedef struct _SwitchControlData
{
    INT16U addr;
	char name[50];
    INT16U cmd;
    char Desc[30];
}SwitchControlData;

typedef struct _SwitchControl
{
    INT8U total;
    SwitchControlData Point[MAX_CHANNEL_NUM];
}SwitchControl;

/*-------------SwitchControl stop-------------------------------*/

/*--------------FaultCode start -------no need to share to COM COMMU---------*/
typedef struct _FaultCodeData
{
    INT16U Code;
	char name[50];
}FaultCodeData;

typedef struct _FaultCode
{
    INT8U total;
    FaultCodeData Point[50];
}FaultCode;
/*---------------FaultCode stop-----------------------------------------------*/

typedef struct _TYPE_INFO
{
    char ProtoName[50];
   // INT8U protoType;     //协议索引   1
  //  INT8U isOut;
    INT8U Version;
    INT8U InterExist;
    INT8U MeaureTotalNum;//measure实际帧数
    //public
    TeleAction TeleActionSection; // 线圈
    StMeasure  MeasureSection[6]; // 模拟量
    ElecDegree ElecDegreeSection; // 电度
    FaultParse FaultParseSection; // 故障索引解析
    SwitchStatusParse SwitchStatusParseSection; //开关量索引解析
    VoltageCurrentParse VoltageCurrentParseSection; //电压电流索引解析
    //private
    INT8U MeasureInternTotalNum; // intern measure实际帧数
    StMeasure  MeasureInternSection[2]; //内部模拟量
    ElecDegree ElecDegreeInternSection; // 内部电度
    FaultParse FaultParseInternSection; // 内部故障索引解析
    SwitchStatusParse SwitchStatusParseInternSection; //内部开关量索引解析
    VoltageCurrentParse VoltageCurrentParseInternSection; //内部电压电流索引解析
    PowerDownStampParse PowerDownStampParseSection; //内部停电票索引解析

    //common
    INT8U ConstantTotalNum;//constant实际帧数
    StConstant ConstantSection[2]; //定值
    StFault Faultsection; // 故障区
    TimeSync TimeSyncSection; //对时
    SwitchControl SwitchControlSection; // 开关操作区
    INT8U FaulCodeGroupNum;
    FaultCode FaultCodeSection[10];// 故障代码列表
}TYPE_INFO;

typedef struct _DEV_INFO
{
    INT8U logic_addr;
    INT8U phy_addr;
    INT8U com_id;
    INT8U rtu_typeIndex;
    INT8U rtu_typeVer;
    INT16U ad;
    char devcode[50];//开关编号
}DEV_INFO;

#define MAX_TYPE_NUM         30 // 目前总共可以存20种协议
#define MAX_VER_NUM          5

typedef struct _OneProto
{
    char Icon[10]; // 图标
    char Name[50]; //协议名称
    INT8U VerNum;   //协议版本数目
    INT8U VerArray[MAX_VER_NUM]; //详细版本列表
    //INT8U ProtoPrivEnable[MAX_PROTO_VER]; //
    char FileName[MAX_VER_NUM][30]; // 详细版本对应的文件名
}StOneProto;

typedef struct _protocolist
{
     INT8U ProtoTotal;
     StOneProto ProtoArray[MAX_TYPE_NUM];
}STProtoList;


typedef struct _CONFIGURE_FILE_CONTENT
{
    INT8S StationName[50];
    INT8U Version;
    INT8U    OneComDevTotal[MAX_COM_NUM];      // devicenum per com
    STProtoList ProtoList;
    DEV_INFO DevInfo[MAX_RTU_NUM]; // sys config info   占了1kB
    COM_DATA ComInfo[MAX_COM_NUM]; // com attr              42byte
    TYPE_INFO ProtoCfgData[MAX_TYPE_NUM][MAX_VER_NUM]; // protocol info
}STCFG_FILE_INFO;

#define MAX_LEN_TCP_BYTES 512

#define COUNTNUM 5
#define FUNNUM   7
#define VALNUM   8

#define RES_SUCCESS 0xF000
#define RES_FAIL    0xE000

#define COMM_STATUS_OFF 0
#define COMM_STATUS_ON  1

typedef struct _COMMUNICATE_VALUE
{
    INT8U  CodeMatchingCnt;
    INT8U  status;
    INT8U  lost;
    INT8U  reserved;
    INT32U tx_frame;
    INT32U rx_frame;
    INT32U tx_byte;
    INT32U rx_byte;
}COMMUNICATE_VALUE;


#define RESULT_SEND_ERROR    0xF0
#define RESULT_RECV_TIMEOUT  0xF1
#define RESULT_CRC_ERROR     0xF2
#define RESULT_FAILED        0xF3
#define RESULT_SUCCESS       0xFA


typedef struct _SOE_INFO
{
	u32 code;
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 min;
	u8 sec;
	u16 msec;
}SOE_INFO; //

typedef struct _FaultSOEinfo
{
    u32 code;
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
    u16 msec;
    u8 phyaddr;
    u8 logicaddr;
    QString devtype;
    QString faultname;
    QString devname;
    u32 isconfirmed;
    u32 faultIndex;
}faultSOEinfo;

typedef struct _StoreFaultSOEinfo
{
    u32 code;
    QString time;
    u8 phyaddr;
    u8 logicaddr;
    QString devtype;
    QString faultname;
    QString devname;
    char faultnamearray[50];
    u32 isconfirmed;
    u32 faultIndex;
}storefaultSOEinfo;

typedef struct _faultnode
{
    int maxindex;
    QString filename;
}faultfilenode;

typedef struct _historyfaultsummary
{
    int currentIndex;
    int filenodenum;
    int copybusyflag;
    int savebusyflag;
    QList<faultfilenode> filenodelist;
}HistoryFaultSummary;

typedef struct _OPERATE_INFO
{
	u8 result;
	u8 opt;
	u8 locate;
	u16 pwd;
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 min;
	u8 sec;
	u16 msec;
}OPERATE_INFO;
//是否本公司产品真假protocol//2016/2/18
#define TRUE_PRO        0
#define FALSE_PRO       1

//对码成功和失败 未进行
#define TO_INSIDE_OK        2
#define TO_INSIDE_NOK       1
#define TO_INSIDE_UNDO      0
#define TO_INSIDE_UNSUPPORT 3

typedef struct _DETAIL_VALUE   //
{
    INT8U  ValueNum;
    INT32U ValueData[MAX_VAL_NUM]; // current 200
    INT8U  CurrentConstantTelegramData[256];    //存储当前定值报文数据部分，因为有部分定值不会修改需要原值返回.
}DETAIL_VALUE;

#define MAX_DATA_TYPE       15

#define READ_CONSTANT_UNDO  0
#define READ_CONSTANT_DONE  101

typedef struct _ALL_TYPE_VALUE
{
    INT8U isIntern;         // 当前协议类型 是私有协议还是公共协议

    INT8U CodeMatchStatus;  // 存储当前设备的对码结果
//    INT8U PhyAddr;          //设备的物理地址
    INT8U GroupIndex;       //用于区分 measure constant的各个子帧
    INT8U CurrentFrameType; // 用于当前帧的类型
    INT8U LogicIndex;       //用于每路串口设备索引 0-15

    INT8U  Source;          // CommandSource  such as  read/write_constant___control  from HMI or TCP
    INT8U  Result;          // CommandResult
    INT8U  ConstantReadStep;    // First Read then Write Constant Every Time.
//control related
//    INT8U OperationIndex; // 当前线圈操作的通道号
    COMMUNICATE_VALUE commu; // 当前设备的通讯信息
    DETAIL_VALUE value[MAX_DATA_TYPE];  //下标来确定存储什么数据 INFO_SWITCH/INFO_MEASURE_1/.....
}ALL_TYPE_VALUE;

typedef struct _usrinfo{
    INT32S name;
    INT32S passwd;
    INT32S level;
}UsrInfo;

typedef struct _STUsrList
{
    UsrInfo        adminlvl;
    QList<UsrInfo> engineerlvl;
    QList<UsrInfo> operaterlvl;

}STUsrList;

typedef struct _STsysLoginInfo
{
    INT32S islogin;
    INT32S usrname;
    INT32S usrlevel;
}STsysLoginInfo;


typedef struct _Device_DataBase
{
    INT8U  PumpNum;
    INT8U  StationRLMode; // remote mode= 1; local mode= 0;就地远程
    INT8U  StationAMMode;//auto mode = 1;manual mode = 0;  手动 自动
    INT8U  Mainsumpinterlock;  //低压泵组闭锁投退  cfgfile
    INT8U  AuxiSumpinterlock;  //高压泵组闭锁投退
    INT8U  OneKeySwith;// just a cmd
    //INT8U  SaveToFile; // just a cmd
    INT8U  OneKeyRelateCtlPump;// 一键启停是否联控水泵，1是；0否
    INT8U  PumpStartNo;       //瓦斯泵运行时起水泵组 1=1#水泵;2=2#水泵

    INT16U SafeData[20];   // 安规数据

    STUsrList              UsrList;
    STsysLoginInfo         UsrLoginInfo;
    STCFG_FILE_INFO        SysCfgData;
    ALL_TYPE_VALUE         DeviceValue[MAX_RTU_NUM];
}Deivce_DataBase;

typedef struct _sysStructPoint
{
    u8 level;
    u8 childflag;
    u8 Index1;
    u8 Index2;
    u8 Index3;
    u8 Index4;
    u8 Index5;
    u8 Index6;
    u8 Index7;
    u8 Index8;
    u8 Com;
    u8 logicaddr;
    u8 phyaddr;
    int isEmpty;
}sysStructPoint;

typedef struct _sysStructCon
{
    u8 lvl;
    u8 devtype;
    u8 Com;
    u8 logicaddr;
    u8 phyaddr;
}sysStructConPoint;

typedef struct _sysStruct
{
    u8 totalI;
    u8 totalII;
    u8 totalCon;
    u8 valid;
    sysStructPoint devPoint[MAX_RTU_NUM];
    sysStructPoint IIdevPoint[MAX_RTU_NUM];
    sysStructConPoint ConDevPoint[8];
}sysStruct;




typedef struct _devlocationPoint
{
    INT8U level;
    INT8U Index1;
    INT8U Index2;
    INT8U Index3;
    INT8U Index4;
    INT8U phyaddr;
}devlocationPoint;

#define VALIDSTRUCTINFO 1
#define INVALIDSTRUCTINFO 0



#define REMOTE_MODE   1
#define LOCAL_MODE    0

#define AUTO_MODE      1
#define MANUAL_MODE    0

#define LVLMANAGE     1
#define LVLENGINEER   2
#define LVLOPERATOR   3

#define LOGIN         1
#define LOGOUT        0

typedef enum _keyvalue
{
    KEYVALUE0 = 0,
    KEYVALUE1,
    KEYVALUE2,
    KEYVALUE3,
    KEYVALUE4,
    KEYVALUE5,
    KEYVALUE6,
    KEYVALUE7,
    KEYVALUE8,
    KEYVALUE9,
    KEYVALUELEFT,
}keyvalue;

#define MAX_BITS_USR 6
#define MAX_BITS_PWD 6

#define FOCUSONUSR 0
#define FOCUSONPWD 1

#define MODIFYFOCUSONUSR 0
#define MODIFYFOCUSONOLDPWD 1
#define MODIFYFOCUSONPWD 2
#define MODIFYFOCUSONPWDAGAIN 3

#define KEYPRESSED 1
#define KEYRELEASED 0

#define SOURCE_HMI 10
#define SOURCE_TCP 20

typedef struct _QUEUE_MSG
{
    INT16U   MsgType;
    INT8U    Source; // HMI = 10; Modbus_Tcp = 20;
    INT8U    LogicAddr;
    INT8U    PhyAddr;
    INT8U    ComPort;
    INT8U    LogicIndex;
    INT8U    GroupIndex; //
    INT8U    ConstantValuePointIndex; //写单个定值时，待写的单个定值于配置文件之中DisplayPoint的索引
    INT8U    Length; //
    INT32U   ValueData[128];  // 若只写单个定值时，该值放到ValueData[0]
}QUEUE_MSG;

//SERIAL RELATED
#define PROTO_PRIV_YES 1
#define PROTO_PRIV_NO  0

#define SUPPORT_YES 1
#define SUPPORT_NO  0

typedef enum _FRAME_TYPE
{
    FRAME_SWITCH,         //0开关量   一般用于外部协议
    FRAME_MEASURE,        //1 模拟量 内外均有，根据配置文件解析一个函数
    FRAME_CONSTANT_READ,  //2读定值   协议差不多一样
    FRAME_CONSTANT_SINGLE_WRITE, // 3 写单个定值
    FRAME_CONSTANT_WRITE, //4写定值
    FRAME_FAULT,          //5故障量
    FRAME_ELECTRIC,       //6电度 一样
    FRAME_OPERATE_INFO,   //7停电票信息 内部协议特有
    FRAME_CONFIRM,        //8确认   no use
    FRAME_CUSTOMER_ID,    //9对码   上线的瞬间进行通信
    FRAME_TIME_CHECK,     //10 时间   检查   其他协议的对时
    FRAME_RESET,          //11 重置
    FRAME_RELAYON,        //12 合闸
    FRAME_RELAYOFF,       //13 分闸
    FRAME_OPERATE,        //14 停电票
    FRAME_TIMEALL_CHECK,  //15 对时
    FRAME_TEST1,          //16 测试
    FRAME_COIL_OPERATION, //17 开关操作
}FRAME_TYPE;

#define ReadHoldCoil            0x01
#define ReadInputCoil           0x02
#define ReadHoldReg             0x03
#define ReadInputReg            0x04
#define WriteHoldCoil           0x05
#define WriteHoldReg            0x06
#define WriteMoreHoldReg        0x10
#define ReadFileRecord          0x14
#define InterTimeAdjust          0x41
#define InterUserCompare         0x43 // 对码命令
#define InterOperate             0x44
#define InterOperateInfo         0x45
#define InterReadInputReg        0x64 // 内部读取命令
#define InterWriteHoldCoil       0x65

#define PARAM_NOT_USED           0
#define PARAM_NOT_USED_POINTER   (void *)0
#define NOT_USED_POINTER         (INT8U *)0

#define BLUEICON   100
#define YELLOWICON 101
#define GREENICON  102
#define REDICON    103
#define GREYICON   104

#define PROTO_TYPE_UK4Z  0
#define PROTO_TYPE_UK4F  1

#endif
