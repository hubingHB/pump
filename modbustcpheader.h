#ifndef MODBUSTCPHEADER_H
#define MODBUSTCPHEADER_H
#include "DefineDb.h"
#define ReadHoldCoil        0x01
#define ReadInputCoil       0x02
#define ReadHoldReg         0x03
#define ReadInputReg        0x04
#define WriteHoldCoil       0x05
#define WriteHoldReg        0x06
#define WriteMoreHoldCoil   0x0F
#define WriteMoreHoldReg    0x10

//02
#define InputCoil_02_START 10001
#define InputCoil_02_END   11000

//0x05  Hold Coil

//0X04 InputReg
#define InputReg_04_16_START      40001
#define InputReg_04_16_END        45000

#define InputReg_04_32_START      45001
#define InputReg_04_32_END        50000

#define InputReg_Fault_START      50001
#define InputReg_Fault_END        51000

#define InputReg_ELEC_START       51001
#define InputReg_ELEC_END         52000

//3-6 HoldReg 支持写多个定值区域
#define HoldReg_06_START          1
#define HoldReg_06_END            200

// HoldReg 仅支持写单个定值区域
#define HoldReg_06_SINGLE_START   201
#define HoldReg_06_SINGLE_END     300

#define HoldReg_SUBSTATION_START      301
#define HoldReg_SUBSTATION_END        350

#define HoldReg_06_Group_START    601

#define HoldReg_06_CONTROL_START  1001
#define HoldReg_06_CONTROL_END    1050

#define HoldReg_06_SUBSTATION_START          1051
#define HoldReg_06_SUBSTATION_PUMPNUM        1051  //泵组数量
#define HoldReg_06_SUBSTATION_ControlMode    1052  //0就地；1远控
#define HoldReg_06_SUBSTATION_AMMode         1053  //0手动；1自动
#define HoldReg_06_SUBSTATION_OneKeySwitch   1054  //一键切换1=1#->2#;2= 2#->1#;3=3#->4#;4=4#->3#;
#define HoldReg_06_SUBSTATION_LowPumpCfgOn   1055  // 低压泵组闭锁投退
#define HoldReg_06_SUBSTATION_HighPumpCfgOn  1056  //高压泵组闭锁投退
#define HoldReg_06_SUBSTATION_SaveCfg        1057  //保存数据 = 1
#define HoldReg_06_SUBSTATION_OneKeyUniCtl   1058  // 一键启停是否联控水泵，1是；0否
#define HoldReg_06_SUBSTATION_PumpStartNo    1059  // 瓦斯泵运行时起水泵组 1=1#水泵;2=2#水泵
#define HoldReg_06_SUBSTATION_SafeDataStart  1060  //预留20个通道 1060-1089
#define HoldReg_06_SUBSTATION_SafeDataEnd    1089
#define HoldReg_06_SUBSTATION_END            1100


//#define CMD_CONSTANT_PRE_READ_1           0x1001
//#define CMD_CONSTANT_PRE_READ_2           0x1002

//#define CMD_CONSTANT_EXE_WRITE_1          0x1218
//#define CMD_CONSTANT_EXE_WRITE_2          0x1219

/***********************************************************************
最高字节 0代表下发处理，A代表结果成功；5代表结果运行中；F代表结果失败
次高字节代表命令类型 2代表读定值；6代表写定值；8代表控制线圈
低16位代表控制通道号(最多32路)或者定值组号(最多2组)
************************************************************************/
#define CMD_CONSTANT_READ          0x0200
#define CMD_CONSTANT_SINGLE_WRITE  0x0400
#define CMD_CONSTANT_WRITE         0x0600
#define CMD_CONTROL_COIL           0x0800

#define STATUS_SUCCESS       0xA000  //(A2/A6/A8)=(读定值成功/写定值成功/控制成功)
#define STATUS_DOING         0x5000  //(52/56/58)=(读定值运行中/写定值运行中/控制运行中)
#define STATUS_FAILED        0xF000  //(F2/F6/F8)=(读定值失败/写定值失败/控制失败)

typedef enum _PointTypeIndex
{
    INPUT_REG_04_16 = 0,
    INPUT_REG_04_32,
    INPUT_REG_FAULT,
    INPUT_REG_ELEC,

}PointTypeIndex;

// notice this place suitable for the point table 4 sections. because the table contionous, so it split to 4 sections.
typedef struct _PointTable
{
    INT8U InputCoilPointTable[1000];     // coil

    INT8U  InputReg16Table[2000];           // 04 100*30*2 =
    INT8U  InputReg32Table[2000];
    INT8U  InputRegFaultTable[2000];
    INT8U  InputRegElecTable[2000];

    INT8U  HoldRegTable[400];              // 03-06 Constant  多个寄存器

    INT8U  HoldRegSingleWTable[200];         //Constant 单个寄存器

    INT8U   ConstantDataPointIndex;    // 用于写单个定值时，所写定值的索引
    INT8U   constantDataNum;
    INT32U  ConstantData[128];

    INT8U   HoldRegControlTable[200];       // 03-06 control

    //u8  InputRegTablePointSize[4][256];
}PointTableST;


#endif // MODBUSTCPHEADER_H
