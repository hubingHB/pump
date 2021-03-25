#ifndef SERIALCOMMU_H
#define SERIALCOMMU_H
#include <QThread>
#include "SerialCommuBase.h"
#include "DefineDb.h"
#include <QQueue>
#include <QMutex>
#include "SerialCommuModbus.h"
//#include "SerialCommu8330.h"
#include <QReadWriteLock>

//typedef struct _LogicIndex_GroupIndex_st
//{
//    INT8U LogicIndex;
//    INT8U GroupIndex;  // 0-5 is measure  100 is TeleAction
//}Logic_Group_Index_Pair;

//#define LOOP_MEASURE_GROUP_0    0
//#define LOOP_MEASURE_GROUP_1    1
//#define LOOP_MEASURE_GROUP_2    2
//#define LOOP_MEASURE_GROUP_3    3
//#define LOOP_MEASURE_GROUP_4    4
//#define LOOP_MEASURE_GROUP_5    5
//#define LOOP_TELE_ACTION        100
//#define LOOP_INDEX_MAX  600

class SerialCommu : public QThread, SerialCommuBase
{
    Q_OBJECT
public:
    SerialCommu();
    INT32S mSerialFd;

    INT32S mCurrentPort;
    INT32S mLogicIndex;
    INT32S mNextLogicIndex;
    INT8U  mNextGroupIndex;
    INT8U  mGroupIndex;
    INT8U  mIsIntern;

    Deivce_DataBase *pDeviceDataBase;

    SerialCommuModbus ModbusIns;
//    SerialCommu8330   Serial8330INS;

    QQueue<QUEUE_MSG> *pMsgQueue;
    QMutex *pMsgQueueMutex; //
    QReadWriteLock *pRWLocker;

    DETAIL_VALUE  mDeviceCurrentValue;
    QUEUE_MSG mCurrentCommandQ;//

//    INT8U mLoopCodeMatchArray[MAX_RTU_NUM];
//    Logic_Group_Index_Pair mLoopIndexArray[2][LOOP_INDEX_MAX];// 0下标为intern;1下标为public
//    INT16U mLoppCodeMatchTotal;
//    INT16U mLoopInternTotal;
//    INT16U mLoopTotal;

    void SetCurrentPort(INT32S port);
    INT32S InitSerial(INT32S ComPort);

    INT32S HandleInterruptRequestMsg(QUEUE_MSG *pMsg, INT8U *pBuffer);

//    void   EquipCommLoopIndexManage(INT32S port);
//    void   EquipCommLoopIndexArrayInit(INT32S port);

    INT32S EquipCommLooping(INT32S port, INT8U *Buffer);

    INT32S EquipCommCodeMatching(INT32S Port, INT32S LogicIndex, INT8U *pBuffer);

    INT32S ReceiveAndProcessMsg(INT32S port, INT32S LogicIndex, INT8U* rBuf, INT8U *sBuf,DETAIL_VALUE *pCurValue, INT32S fd);

    void SummaryCommuData(INT32S port, INT32S LogicIndex, INT32S sendSize, INT32S recvSize);

    void HandleRecvFrame(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);

    void CommitToDataBase(INT32S port, INT32S LogicIndex, DETAIL_VALUE *pCurValue);

    void handleCommError(INT32S port, INT32S LogicIndex, INT8U ErrorType);

    //routers start
    INT32S EquipCommCodeMatchingPackagePrepareRouter(INT32S LogicIndex, INT32S ComPort, INT8U *buffer);
    INT32S EquipCommLoopPackagePrepareRouter(INT32S LogicIndex, INT32S ComPort,INT8U GroupIndex, INT8U *buffer);

    //准备读线圈包
    INT16U LoopReadCoilPackagePrepareRouter(INT32S LogicIndex,INT32S ComPort,INT8U *buffer);
    //准备读模拟量包
    INT16U LoopReadMeasurePackagePrepareRouter(INT32S LogicIndex,INT32S ComPort,INT8U *buffer,INT8U GroupIndex);
    //读内部协议模拟量包准备
    INT16U LoopReadInterMeasurePackagePrepareRouter(INT32S LogicIndex,INT32S ComPort,INT8U *buffer,INT8U GroupIndex);

    INT32S RecvHeadByteRouter(INT32S port, INT32S LogicIndex);
    INT32S RecvHeadBytesWaitTimeRouter(INT32S port, INT32S LogicIndex);

    INT32S RecvRemainBytesRouter(INT32S port, INT32S LogicIndex, INT8U *rBuf);
    INT32S RecvRemainBytesWaitTimeRouter(INT32S port, INT32S LogicIndex);

    INT32S CheckFrameOKRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer,INT8U *SendBuf, INT8U size);




    void HandleReadCoilRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
    void HandleReadMeasureRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
    void HandleReadInternMeasureRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);

    void HandleInterruptConstantReadRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
    void HandleInterruptConstantWriteRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
    void HandleInterruptSingleConstantWriteRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
    void HandleInterruptControlCoilRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);

    void HandleEquipCodeMatchRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);
    void HandleReadWattHourRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);

    INT16U InterruptReadConstantPackagePrepareRouter(QUEUE_MSG *pMsg,INT8U *buffer);
    INT16U InterruptWriteSingleConstantPackagePrepareRouter(QUEUE_MSG *pMsg,INT8U *buffer);
    INT16U InterruptWriteConstantPackagePrepareRouter(QUEUE_MSG *pMsg,INT8U *buffer);
    INT16U InterruptControlCoilPackagePrepareRouter(QUEUE_MSG *pMsg,INT8U *buffer);
    //router end

    void DebugPrintPkt(INT8U *data, INT8U len, char *pTitle);
    void DelayToMakeSureAllSendOut(INT8U SendLength,INT32U Speed);

signals:
    void UpdateDevStatusSignal(INT8U LogicAddr,INT8U FrameType,INT8U GroupIndex);
    void UpdateDevConstantSignal(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U GroupIndex);
    void UpdateControlCoilSignal(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U ChannelIndex);
protected:
    virtual void run();
};

#endif // SERIALCOMMU_H
