#ifndef SERIALCPMMUMODBUS_H
#define SERIALCPMMUMODBUS_H
#include "DefineDb.h"

class SerialCommuModbus
{
public:
    SerialCommuModbus();

    Deivce_DataBase *pDeviceDataBase;
    //common 一般不会变的 internal used
    INT16U CRCCheck(INT8U *data,INT8U len);
    INT16U Intrinsic_Function_For_ID(INT16U ad,INT16U code);

// 对码包准备
    INT16U EquipCommuCodeMatchingPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer);
//准备读线圈包
    INT16U ReadCoilPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer);
//准备读模拟量包
    INT16U ReadMeasurePackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer,INT8U GroupIndex);
//准备读故障包
    INT16U ReadFaultPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer);
//准备读电度包
    INT16U ReadWattHourPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer);
//读内部协议模拟量包准备
    INT16U ReadInterMeasurePackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer,INT8U GroupIndex);

//接收报文处理 线圈
    void HandleReadCoil(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
//接收报文处理 模拟量
    void HandleReadMeasure(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
//接收报文处理  内部协议模拟量
    void HandleReadInternMeasure(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);

//中断请求 读取定值
    INT16U InterruptReadConstantPackagePrepare(QUEUE_MSG *pMsg,INT8U *buffer);
//中断请求  写定值
    INT16U InterruptWriteConstantPackagePrepare(QUEUE_MSG *pMsg,INT8U *buffer);
//中断请求  写单个定值
    INT16U InterruptWriteSingleConstanPackagePrepare(QUEUE_MSG *pMsg,INT8U *buffer);
//中断请求 开关量操作
    INT16U InterruptControlCoilPackagePrepare(QUEUE_MSG *pMsg,INT8U *buffer);

//头文件接收字节数
    INT32S RecvHeadBytes(INT32U port, INT32U LogicIndex);
//接收头字节数等待时间
    INT32S RecvHeadBytesWaitTime(INT32U port, INT32U LogicIndex);
//接收剩下的字节数
    INT32S RecvRemainBytes(INT32U port, INT32U LogicIndex, INT8U *rBuf);

// 接收剩下字节数等待时间
    INT32S RecvRemainBytesWaitTime(INT32U port, INT32U LogicIndex);
// 检查接收报文正确与否
    INT32S CheckFrameOK(INT32U port, INT32U LogicIndex,INT8U *Buffer,INT8U size);
//接收报文处理  读取定值
    void HandleInterruptConstantRead(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
//接收报文处理  写定值
    void HandleInterruptConstantWrite(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);

//接收报文处理  写单个定值
    void HandleInterruptSingleConstantWrite(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);


//接收报文处理  开关量操作
    void HandleInterruptControlCoil(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
//接收报文处理   对码
    void HandleEquipCodeMatch(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
//接收报文处理 电度
    void HandleReadWattHour(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);


};

#endif // SERIALCPMMUMODBUS_H
