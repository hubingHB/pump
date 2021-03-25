#include "SerialCommu.h"
#include <QDebug>
SerialCommu::SerialCommu()
{
    mNextLogicIndex = 0;
    mLogicIndex = 0;
}
/*thread function*/
void SerialCommu::run()
{
    qDebug()<<"thread "<<mCurrentPort<<" started";
    INT32S tRes = -1;
    INT8U  tSendLength = 0;
    INT32S tSendActualLength = 0;
    INT32S tRecvLength = 0;

    INT8U sendbytes[MAX_LEN_COM_BYTES];
    INT8U recvbytes[MAX_LEN_COM_BYTES];

    tRes = InitSerial(mCurrentPort);
    if(tRes > 0)// means OK
    {
            qDebug()<<"####"<<__FUNCTION__<<__LINE__;

            mLogicIndex  = 0;
            mGroupIndex  = 0;
            mNextGroupIndex = 0;
            mNextLogicIndex = 0;

            while(1)
            {
                qDebug()<<"####"<<__FUNCTION__<<__LINE__;
                // prepare a request  1- interrupt;  2- loop
                    pMsgQueueMutex->lock();
                    if(pMsgQueue->length())
                    {
                        qDebug()<<"####"<<__FUNCTION__<<__LINE__;
                        mCurrentCommandQ = pMsgQueue->dequeue();
                        pMsgQueueMutex->unlock();  //

                        tSendLength = HandleInterruptRequestMsg(&mCurrentCommandQ,sendbytes);
                    }
                    else
                    {
                        qDebug()<<"####"<<__FUNCTION__<<__LINE__<<"mLogicIndex="<<mLogicIndex<<mCurrentPort;
                        pMsgQueueMutex->unlock();

                        tSendLength = EquipCommLooping(mCurrentPort,sendbytes);
                    }
                    //发送之前刷新输入输出缓冲区
                    SerialFlushBuffer(mSerialFd);//

//                    qDebug()<<tSendLength<<sendbytes[0]<<sendbytes[1]<<sendbytes[2]<<sendbytes[3]<<sendbytes[4]
//                            <<sendbytes[5]<<sendbytes[6]<<sendbytes[7]<<sendbytes[8]<<sendbytes[9]<<sendbytes[10];

                    DebugPrintPkt(sendbytes,tSendLength,"MsgSend:");
                    if(tSendLength > 0)
                    {
                        tSendActualLength = SerialWrite(mSerialFd,sendbytes,tSendLength);

                        if(tSendActualLength == tSendLength)
                        {
                            memset(&mDeviceCurrentValue,0x00,sizeof(DETAIL_VALUE));
                            tRecvLength = ReceiveAndProcessMsg(mCurrentPort,mLogicIndex,recvbytes,sendbytes,&mDeviceCurrentValue,mSerialFd);
                            SummaryCommuData(mCurrentPort,mLogicIndex,tSendLength,tRecvLength);
                        }
                        else //send failed
                        {
                            //发送失败刷新输入输出缓冲区
                            handleCommError(mCurrentPort,mLogicIndex,RESULT_SEND_ERROR);
                            SerialFlushBuffer(mSerialFd);
                        }
                    }

                    SerialFlushBuffer(mSerialFd);

                    CommitToDataBase(mCurrentPort,mLogicIndex,&mDeviceCurrentValue);

                    usleep(300000);
                    SerialFlushBuffer(mSerialFd);
            }
     }
}

void SerialCommu::SetCurrentPort(INT32S port)
{
    mCurrentPort = port;
}

INT32S SerialCommu::InitSerial(INT32S ComPort)
{
    INT32S tSpeed    = 9600;
    INT32S tParity   = 0;
    INT32S tDataBits = 0;
    INT32S tStopBits = 0;
    INT32S tRes = -1;

    mSerialFd = SerialOpen(ComPort);

    if(mSerialFd > 0)
    {
        tSpeed    = pDeviceDataBase->SysCfgData.ComInfo[ComPort].speed;
        tParity   = pDeviceDataBase->SysCfgData.ComInfo[ComPort].parity;
        tDataBits = pDeviceDataBase->SysCfgData.ComInfo[ComPort].databits;
        tStopBits = pDeviceDataBase->SysCfgData.ComInfo[ComPort].stopbit;

        //打开串口
        tRes = SerialSetParam(mSerialFd,tSpeed,tParity,tDataBits,tStopBits);
        if(tRes == -1)
        {
            qDebug()<<"Serial Port open error,Port="<<mCurrentPort;
            return -2;
        }
        else
        {
            return mSerialFd;
        }
    }
    else
    {
        return -1;
    }

}

//void SerialCommu::EquipCommLoopIndexManage(INT32S port)
//{
//    mLogicIndex = mNextLogicIndex;
//    mGroupIndex = mNextGroupIndex;

//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + mLogicIndex;
//    INT8U tIsIntern  = pDeviceDataBase->DeviceValue[tLogicAddr].isIntern;

//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

//    if(pDeviceDataBase->DeviceValue[tLogicAddr].CodeMatchStatus == TO_INSIDE_UNDO)
//    {
//        mNextLogicIndex++;
//        if(mNextLogicIndex >= pDeviceDataBase->SysCfgData.OneComDevTotal[tLogicAddr])
//        {
//            mNextLogicIndex = 0;
//        }


//    }
//    else
//    {
//        if(tIsIntern)
//        {
//            mNextGroupIndex++;
//            if(mNextGroupIndex >= pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternTotalNum)
//            {
//                mNextGroupIndex = 0; // 内部协议无线圈

//                mNextLogicIndex++;
//                if(mNextLogicIndex >= pDeviceDataBase->SysCfgData.OneComDevTotal[tLogicAddr])
//                {
//                    mNextLogicIndex = 0;
//                }
//            }
//        }
//        else
//        {
//            mNextGroupIndex++;
//            mNextLogicIndex++;

//            if(mNextGroupIndex >= pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeaureTotalNum+1)
//            {
//                mNextGroupIndex = 0;
//                mNextLogicIndex++;
//                if(mNextLogicIndex >= pDeviceDataBase->SysCfgData.OneComDevTotal[tLogicAddr])
//                {
//                    mNextLogicIndex = 0;
//                }
//            }
//        }
//    }
//}

//void  SerialCommu::EquipCommLoopIndexArrayInit(INT32S port)
//{
//    memset(mLoopIndexArray,0x00,sizeof(Logic_Group_Index_Pair)*LOOP_INDEX_MAX*2);
//    memset(mLoopCodeMatchArray,0x00,MAX_RTU_NUM);

//    INT8U tLogicAddr = 0;
//    INT8U tIsIntern  = pDeviceDataBase->DeviceValue[tLogicAddr].isIntern;
//    INT8U tTypeIndex = 0;//pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = 0;//pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
//    INT8U i = 0;
//    INT8U j = 0;
//    INT8U OneComTotal = pDeviceDataBase->SysCfgData.OneComDevTotal[port];

//    INT8U MeasureInternTotal = 0;
//    INT8U MeasureTotal = 0;

//对码 循环数组初始化
//    mLoppCodeMatchTotal = OneComTotal;
//    for(i=0;i < OneComTotal;i++)
//    {
//        mLoopCodeMatchArray[i] = i;
//    }
// 内部协议循环数组初始化
//    mLoopInternTotal = 0;
//    for(i=0;i< OneComTotal;i++) // intern
//    {
//        tLogicAddr = port * MAX_COM_RTU_NUM + i;
//        tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//        tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
//        MeasureInternTotal = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternTotalNum;
//        for(j=0;j<MeasureInternTotal;j++)
//        {
//            mLoopIndexArray[0][mLoopInternTotal].LogicIndex = i;
//            mLoopIndexArray[0][mLoopInternTotal].GroupIndex = j;
//            mLoopInternTotal++;
//        }
//    }
//公共协议循环数组初始化
//    mLoopTotal = 0;
//    for(i=0;i< OneComTotal;i++) // public
//    {
//        tLogicAddr = port * MAX_COM_RTU_NUM + i;
//        tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//        tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
//        MeasureTotal = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeaureTotalNum;

//        mLoopIndexArray[1][mLoopTotal].LogicIndex = i;
//        mLoopIndexArray[1][mLoopTotal].GroupIndex = LOOP_TELE_ACTION;
//        mLoopTotal++;
//        for(j=0;j<MeasureTotal;j++)
//        {
//            mLoopIndexArray[1][mLoopTotal].LogicIndex = i;
//            mLoopIndexArray[1][mLoopTotal].GroupIndex = j;
//            mLoopTotal++;
//        }
//    }
//}

INT32S SerialCommu::HandleInterruptRequestMsg(QUEUE_MSG *pMsg, INT8U *pBuffer)
{
    INT32S Length = 0;
    INT16U MsgType = pMsg->MsgType;
    mLogicIndex = pMsg->LogicIndex;
    switch(MsgType)
    {
    case FRAME_CONSTANT_READ: // 读定值
         Length = InterruptReadConstantPackagePrepareRouter(pMsg,pBuffer);
         break;
    case FRAME_CONSTANT_SINGLE_WRITE: // 写单个定值
         Length = InterruptWriteSingleConstantPackagePrepareRouter(pMsg,pBuffer);
         break;
    case FRAME_CONSTANT_WRITE: // 写多个定值
         Length = InterruptWriteConstantPackagePrepareRouter(pMsg,pBuffer);
         break;
    case FRAME_COIL_OPERATION: //开关量控制
    case FRAME_RELAYON:
    case FRAME_RELAYOFF:
    case FRAME_TEST1:
    case FRAME_RESET:
         Length = InterruptControlCoilPackagePrepareRouter(pMsg,pBuffer);
         break;
    case FRAME_ELECTRIC: //查询电度
         break;
    case FRAME_FAULT:
         break;

    }
    return Length;
}


INT16U SerialCommu::InterruptReadConstantPackagePrepareRouter(QUEUE_MSG *pMsg,INT8U *buffer)
{
    INT16U tLength = 0;
//  INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[pMsg->LogicAddr].rtu_typeIndex;
//  if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.InterruptReadConstantPackagePrepare(pMsg,buffer);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)
//    {
//        tLength = Serial8330INS.InterruptReadConstantPackagePrepare(pMsg,buffer);

//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {

//    }
//    else
//    {
//         qDebug()<<"111111";
//    }
    return tLength;
}

INT16U SerialCommu::InterruptWriteConstantPackagePrepareRouter(QUEUE_MSG *pMsg,INT8U *buffer)
{
    INT16U tLength = 0;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[pMsg->LogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.InterruptWriteConstantPackagePrepare(pMsg,buffer);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)
//    {
//        tLength = Serial8330INS.InterruptWriteConstantPackagePrepare(pMsg,buffer);

//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {

//    }
//    else
//    {
//         qDebug()<<"111111";
//    }
    return tLength;
}

INT16U SerialCommu::InterruptWriteSingleConstantPackagePrepareRouter(QUEUE_MSG *pMsg, INT8U *buffer)
{
    INT16U tLength = 0;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[pMsg->LogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.InterruptWriteSingleConstanPackagePrepare(pMsg,buffer);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)
//    {
//        tLength = Serial8330INS.InterruptWriteSingleConstanPackagePrepare(pMsg,buffer);

//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {

//    }
//    else
//    {
//         qDebug()<<"111111";
//    }
    return tLength;
}

INT16U SerialCommu::InterruptControlCoilPackagePrepareRouter(QUEUE_MSG *pMsg,INT8U *buffer)
{
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[pMsg->LogicAddr].rtu_typeIndex;
    INT16U tLength = 0;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.InterruptControlCoilPackagePrepare(pMsg,buffer);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)
//    {
//        tLength = Serial8330INS.InterruptControlCoilPackagePrepare(pMsg,buffer);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {

//    }
//    else
//    {
//         qDebug()<<"111111";
//    }
    return tLength;
}

INT32S SerialCommu::EquipCommCodeMatching(INT32S Port,INT32S LogicIndex,INT8U *pBuffer)
{
    INT8U tLogicAddr = Port * MAX_COM_RTU_NUM + LogicIndex;
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT8U tIsSupportPriv = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].InterExist;  // 是否支持内部协议
    INT8U tCommuStatus   = pDeviceDataBase->DeviceValue[tLogicAddr].commu.status; // 当前通讯状态,在线or离线

    INT16U tLength = 0;
    INT8U tCodeMatchStatus = pDeviceDataBase->DeviceValue[tLogicAddr].CodeMatchStatus;

    qDebug()<<__FUNCTION__<<__LINE__<<tLogicAddr<<Port;
    if(tIsSupportPriv == SUPPORT_YES)  // 支持内部协议可能需要对码
    {
        if(tCommuStatus == COMM_STATUS_OFF) //原来离线 现在上线对码
        {
            tLength = EquipCommCodeMatchingPackagePrepareRouter(LogicIndex,Port,pBuffer);
            pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = FRAME_CUSTOMER_ID;
            pDeviceDataBase->DeviceValue[tLogicAddr].commu.CodeMatchingCnt = 0;
            return tLength;
        }
        else
        {
            if(tCodeMatchStatus == TO_INSIDE_UNDO) //设备未进行对码，则进行对码操作
            {
                tLength = EquipCommCodeMatchingPackagePrepareRouter(LogicIndex,Port,pBuffer);
                pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = FRAME_CUSTOMER_ID;
                pDeviceDataBase->DeviceValue[tLogicAddr].commu.CodeMatchingCnt = 0;
                return tLength;
            }
            else if(tCodeMatchStatus == TO_INSIDE_NOK) //设备对码失败，三次尝试超过,使用外部协议
            {
                pDeviceDataBase->DeviceValue[tLogicAddr].commu.CodeMatchingCnt++;
                if(pDeviceDataBase->DeviceValue[tLogicAddr].commu.CodeMatchingCnt < 5)
                {
                    tLength = EquipCommCodeMatchingPackagePrepareRouter(LogicIndex,Port,pBuffer);
                    pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = FRAME_CUSTOMER_ID;
                    return tLength;
                }
                else
                {
                    return 0;
                }
            }
            else // 设备对码成功无须对码
            {
//                if(tCommuStatus == COMM_STATUS_OFF)
//                {
//                    tLength = EquipCommCodeMatchingPackagePrepareRouter(LogicIndex,Port,pBuffer);
//                    pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = FRAME_CUSTOMER_ID;
//                    pDeviceDataBase->DeviceValue[tLogicAddr].commu.CodeMatchingCnt = 0;
//                    return tLength;
//                }
//                else
//                {
                    return 0;
//                }
            }
        }
    }
    else  //不支持内部协议则跳过对码
    {
        return 0;
    }
}

INT32S SerialCommu::EquipCommCodeMatchingPackagePrepareRouter(INT32S LogicIndex, INT32S ComPort, INT8U *buffer)
{
    INT8U tLogicAddr = ComPort * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT32S tLength = 0;

//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.EquipCommuCodeMatchingPackagePrepare(LogicIndex,ComPort,buffer);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        tLength = Serial8330INS.EquipCommuCodeMatchingPackagePrepare(LogicIndex,ComPort,buffer);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
    return tLength;
}

INT32S SerialCommu::EquipCommLooping(INT32S port, INT8U *Buffer)
{
    mLogicIndex = mNextLogicIndex;
    mGroupIndex = mNextGroupIndex;

    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + mLogicIndex;
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT32S tLength = 0;
    qDebug()<<"####"<<__FUNCTION__<<__LINE__<<port;

    mNextGroupIndex++;
    if(mNextGroupIndex > pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeaureTotalNum)
    {
         mNextGroupIndex = 0;
         mNextLogicIndex ++;
         if(mNextLogicIndex >= pDeviceDataBase->SysCfgData.OneComDevTotal[port])
            mNextLogicIndex = 0;
    }

    tLength = EquipCommLoopPackagePrepareRouter(mLogicIndex,port,mGroupIndex,Buffer);
    qDebug()<<"####"<<__FUNCTION__<<__LINE__<<tLength<<mGroupIndex<<port;
    return tLength;
}

INT32S SerialCommu::EquipCommLoopPackagePrepareRouter(INT32S LogicIndex, INT32S ComPort,INT8U GroupIndex, INT8U *buffer)
{
    INT8U tLogicAddr = ComPort*MAX_COM_RTU_NUM + LogicIndex;
    INT16U tLength = 0;

    pDeviceDataBase->DeviceValue[tLogicAddr].LogicIndex = LogicIndex;
    pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex = GroupIndex;

    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    if(pDeviceDataBase->DeviceValue[tLogicAddr].isIntern == PROTO_PRIV_YES)
    {
       qDebug()<<"####"<<__FUNCTION__<<__LINE__;
       pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = FRAME_MEASURE;
//       tLength = ModbusIns.ReadInterMeasurePackagePrepare(LogicIndex,ComPort,buffer,GroupIndex);
       tLength = LoopReadInterMeasurePackagePrepareRouter(LogicIndex,ComPort,buffer,GroupIndex);
    }
    else
    {
       qDebug()<<"####"<<__FUNCTION__<<__LINE__<<pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeaureTotalNum;
       if(GroupIndex == pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeaureTotalNum)
       {
          pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = FRAME_SWITCH;
//          tLength = ModbusIns.ReadCoilPackagePrepare(LogicIndex,ComPort,buffer);
          tLength = LoopReadCoilPackagePrepareRouter(LogicIndex,ComPort,buffer);
       }
       else
       {
          pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = FRAME_MEASURE;
//          tLength = ModbusIns.ReadMeasurePackagePrepare(LogicIndex,ComPort,buffer,GroupIndex);
          tLength = LoopReadMeasurePackagePrepareRouter(LogicIndex,ComPort,buffer,GroupIndex);
       }
     }

    return tLength;
}

//准备读线圈包
INT16U SerialCommu::LoopReadCoilPackagePrepareRouter(INT32S LogicIndex,INT32S ComPort,INT8U *buffer)
{
    INT8U tLogicAddr = ComPort*MAX_COM_RTU_NUM + LogicIndex;
    INT16U tLength = 0;

    pDeviceDataBase->DeviceValue[tLogicAddr].LogicIndex = LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;

//   if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.ReadCoilPackagePrepare(LogicIndex,ComPort,buffer);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)
//    {
//        tLength = Serial8330INS.ReadCoilPackagePrepare(LogicIndex,ComPort,buffer);
//    }
//    else
//    {
//        qDebug()<<__FUNCTION__<<"WARNING: Not Support Proto"<<tTypeIndex;
//    }
    return tLength;
}

//准备读模拟量包
INT16U SerialCommu::LoopReadMeasurePackagePrepareRouter(INT32S LogicIndex,INT32S ComPort,INT8U *buffer,INT8U GroupIndex)
{
    INT8U tLogicAddr = ComPort*MAX_COM_RTU_NUM + LogicIndex;
    INT16U tLength = 0;

    pDeviceDataBase->DeviceValue[tLogicAddr].LogicIndex = LogicIndex;
    pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex = GroupIndex;

//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.ReadMeasurePackagePrepare(LogicIndex,ComPort,buffer,GroupIndex);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)
//    {
//        tLength = Serial8330INS.ReadMeasurePackagePrepare(LogicIndex,ComPort,buffer,GroupIndex);
//    }
//    else
//    {
//        qDebug()<<__FUNCTION__<<"WARNING: Not Support Proto"<<tTypeIndex;
//    }
    return tLength;
}

//读内部协议模拟量包准备
INT16U SerialCommu::LoopReadInterMeasurePackagePrepareRouter(INT32S LogicIndex,INT32S ComPort,INT8U *buffer,INT8U GroupIndex)
{
    INT8U tLogicAddr = ComPort*MAX_COM_RTU_NUM + LogicIndex;
    INT16U tLength = 0;

    pDeviceDataBase->DeviceValue[tLogicAddr].LogicIndex = LogicIndex;
    pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex = GroupIndex;

//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.ReadInterMeasurePackagePrepare(LogicIndex,ComPort,buffer,GroupIndex);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)
//    {
//        tLength = Serial8330INS.ReadInterMeasurePackagePrepare(LogicIndex,ComPort,buffer,GroupIndex);
//    }
//    else
//    {
//        qDebug()<<tTypeIndex<<"WARNING: Not Support Proto"<<tTypeIndex;
//    }
    return tLength;
}

/*
   一般串口接收分为两段式，一种是提前知道大小，
   则直接按照需要的大小接收，还有一种是接收头部,然后根据头部长度进行接收。
   为了统一,这边实现的方式是先根据不同的frame 接收一部分，然后在接收另外一部分。
   这些 对于特定协议，可以根据实际情况在不同的特定类之中指定
*/
INT32S SerialCommu::RecvHeadByteRouter(INT32S port,INT32S LogicIndex)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT32S tLength = 0;

//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.RecvHeadBytes(port,LogicIndex);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        tLength = Serial8330INS.RecvHeadBytes(port,LogicIndex);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
    return tLength;

}

INT32S SerialCommu::RecvHeadBytesWaitTimeRouter(INT32S port,INT32S LogicIndex)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT32S tLength = 0;

//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.RecvHeadBytesWaitTime(port,LogicIndex);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        tLength = Serial8330INS.RecvHeadBytesWaitTime(port,LogicIndex);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
    return tLength;
}

INT32S SerialCommu::RecvRemainBytesRouter(INT32S port, INT32S LogicIndex, INT8U *rBuf)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT32S tLength = 0;

//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.RecvRemainBytes(port,LogicIndex,rBuf);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        tLength = Serial8330INS.RecvRemainBytes(port,LogicIndex,rBuf);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
    return tLength;
}

INT32S SerialCommu::RecvRemainBytesWaitTimeRouter(INT32S port, INT32S LogicIndex)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT32S tLength = 0;

//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.RecvRemainBytesWaitTime(port,LogicIndex);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        tLength = Serial8330INS.RecvRemainBytesWaitTime(port,LogicIndex);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
    return tLength;
}

INT32S SerialCommu::CheckFrameOKRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U *SendBuf, INT8U size)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT32S tLength = 0;

    if(SendBuf[0] != Buffer[0])
    {
        qDebug()<<"ERROR:Send and Recv Addr MisMatch";
        return -1;
    }
    if(SendBuf[1] != Buffer[1])
    {
        qDebug()<<"ERROR:Send and Recv FunCode MisMatch";
        return -1;
    }

//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        tLength = ModbusIns.CheckFrameOK(port,LogicIndex,Buffer,size);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        tLength = Serial8330INS.CheckFrameOK(port,LogicIndex,Buffer,size);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
    return tLength;
}

INT32S SerialCommu::ReceiveAndProcessMsg(INT32S port, INT32S LogicIndex, INT8U* rBuf, INT8U *sBuf, DETAIL_VALUE *pCurValue, INT32S fd)
{
    INT32S tHeadSize = 0;
    INT32S tRemainSize = 0;
    INT32S tHeadTime = 0;
    INT32S tRemainTime = 0;
    INT32S tNpos = 0;
    INT32S tRet;
    INT32S step = 0;
    INT8U tLogicAddr = port *MAX_COM_RTU_NUM + LogicIndex;
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;

    tHeadSize = RecvHeadByteRouter(port,LogicIndex);
    tHeadTime = RecvHeadBytesWaitTimeRouter(port,LogicIndex);

    if(tHeadSize > 0)
    {
        tRet  = SerialReadData(fd,rBuf,tHeadSize,tHeadTime);
        tNpos = tHeadSize;
        if(tRet == tHeadSize)
        {
            step = 1;
        }
        else
        {
            qDebug()<<"####"<<__FUNCTION__<<__LINE__;
            handleCommError(port,LogicIndex,RESULT_RECV_TIMEOUT);
            //接收失败刷新输入输出缓冲区
            SerialFlushBuffer(mSerialFd);
            return -1;
        }
    }

    if(step == 1)
    {
        qDebug()<<"####"<<__FUNCTION__<<__LINE__;
        tRemainSize = RecvRemainBytesRouter(port,LogicIndex,rBuf);
        tRemainTime = RecvRemainBytesWaitTimeRouter(port,LogicIndex);

        while((tRet = SerialReadData(fd,&rBuf[tNpos],tRemainSize,tRemainTime)) > 0)
        {
            tRemainSize -= tRet;
            tNpos += tRet;
            if(tRemainSize <= 0)
            {
                step = 2;
                break;
            }
        }
    }

    if(step == 2)
    {
        DebugPrintPkt(rBuf,tNpos,"MsgRecv:");
        tRet = CheckFrameOKRouter(port,LogicIndex,rBuf,sBuf,tNpos);

        qDebug()<<"####"<<__FUNCTION__<<__LINE__<<tRet<<LogicIndex<<port<<tNpos;
        if(tRet == 0)
        {
            qDebug()<<"####"<<__FUNCTION__<<__LINE__;
            step = 3;
        }
        else
        {
            handleCommError(port,LogicIndex,RESULT_CRC_ERROR);
            //接收失败刷新输入输出缓冲区
            SerialFlushBuffer(mSerialFd);
        }
    }

    if(step == 3)
    {
        HandleRecvFrame(port,LogicIndex,rBuf,tNpos,pCurValue);
        return tNpos;
    }
    else
    {
        handleCommError(port,LogicIndex,RESULT_RECV_TIMEOUT);
        //接收失败刷新输入输出缓冲区
        SerialFlushBuffer(mSerialFd);
        return -1;
    }
}

void SerialCommu::HandleRecvFrame(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
    INT8U tFrameType = pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType;
    INT8U tIsIntern  = pDeviceDataBase->DeviceValue[tLogicAddr].isIntern;

    qDebug()<<__FUNCTION__<<__LINE__<<tIsIntern<<tFrameType;

    switch(tFrameType)
    {
    case FRAME_SWITCH: //
         HandleReadCoilRouter(port,LogicIndex,Buffer,size,pCurValue);
         break;
    case FRAME_MEASURE:
         if(tIsIntern)
             HandleReadInternMeasureRouter(port,LogicIndex,Buffer,size,pCurValue);
         else
             HandleReadMeasureRouter(port,LogicIndex,Buffer,size,pCurValue);
         break;
    case FRAME_CONSTANT_READ:
         HandleInterruptConstantReadRouter(port,LogicIndex,Buffer,size,pCurValue);
         break;
    case FRAME_CONSTANT_WRITE:
         HandleInterruptConstantWriteRouter(port,LogicIndex,Buffer,size,pCurValue);
         break;
    case FRAME_CONSTANT_SINGLE_WRITE:
         HandleInterruptSingleConstantWriteRouter(port,LogicIndex,Buffer,size,pCurValue);
         break;
    case FRAME_CUSTOMER_ID:
         qDebug()<<"####"<<__FUNCTION__<<__LINE__;
         HandleEquipCodeMatchRouter(port,LogicIndex,Buffer,size,pCurValue);
         break;
    case FRAME_FAULT:
         break;
    case FRAME_ELECTRIC: //todo
         HandleReadWattHourRouter(port,LogicIndex,Buffer,size,pCurValue);
         break;
    case FRAME_COIL_OPERATION: //开关量控制
    case FRAME_RELAYON:
    case FRAME_RELAYOFF:
    case FRAME_TEST1:
    case FRAME_RESET:
         HandleInterruptControlCoilRouter(port,LogicIndex,Buffer,size,pCurValue);
         break;
    }
}

void SerialCommu::HandleReadCoilRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleReadCoil(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        Serial8330INS.HandleReadCoil(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}

void SerialCommu::HandleReadMeasureRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleReadMeasure(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        Serial8330INS.HandleReadMeasure(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}

void SerialCommu::HandleReadInternMeasureRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleReadInternMeasure(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        Serial8330INS.HandleReadInternMeasure(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}

void SerialCommu::HandleInterruptConstantReadRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleInterruptConstantRead(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        Serial8330INS.HandleInterruptConstantRead(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}

void SerialCommu::HandleInterruptConstantWriteRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleInterruptConstantWrite(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        Serial8330INS.HandleInterruptConstantWrite(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}

void SerialCommu::HandleInterruptSingleConstantWriteRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleInterruptSingleConstantWrite(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        Serial8330INS.HandleInterruptSingleConstantWrite(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}

void SerialCommu::HandleInterruptControlCoilRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleInterruptControlCoil(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330) //
//    {
//        Serial8330INS.HandleInterruptControlCoil(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}

void SerialCommu::HandleEquipCodeMatchRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleEquipCodeMatch(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)//
//    {
//        Serial8330INS.HandleEquipCodeMatch(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}

void SerialCommu::HandleReadWattHourRouter(INT32S port, INT32S LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
//    if((tTypeIndex == PROTO_TYPE_UK4F)||(tTypeIndex == PROTO_TYPE_UK4Z))
    {
        ModbusIns.HandleReadWattHour(port,LogicIndex,Buffer,size,pCurValue);
    }
//    else if(tTypeIndex == PROTO_TYPE_8330)//
//    {
//        Serial8330INS.HandleReadWattHour(port,LogicIndex,Buffer,size,pCurValue);
//    }
//    else if(tTypeIndex == PROTO_TYPE_800M)
//    {
//        ;
//    }
//    else
//    {
//        qDebug()<<"not support ";
//    }
}


void SerialCommu::SummaryCommuData(INT32S port, INT32S LogicIndex, INT32S sendSize, INT32S recvSize)
{
    INT32S tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
    pDeviceDataBase->DeviceValue[tLogicAddr].commu.tx_frame++;
    pDeviceDataBase->DeviceValue[tLogicAddr].commu.tx_byte+=sendSize;

    if(recvSize > 0)
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].commu.rx_frame++;
        pDeviceDataBase->DeviceValue[tLogicAddr].commu.rx_byte+=recvSize;
        pDeviceDataBase->DeviceValue[tLogicAddr].commu.lost = 0;
        pDeviceDataBase->DeviceValue[tLogicAddr].commu.status = COMM_STATUS_ON;
    }
    else
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].commu.lost++;
        if(pDeviceDataBase->DeviceValue[tLogicAddr].commu.lost > 3)
        {
            pDeviceDataBase->DeviceValue[tLogicAddr].commu.lost = 0;
            pDeviceDataBase->DeviceValue[tLogicAddr].commu.status = COMM_STATUS_OFF;
        }
    }
}

void SerialCommu::DelayToMakeSureAllSendOut(INT8U SendLength,INT32U Speed)
{
    INT32U DelayTime = 1;
    if(Speed == 0)
        Speed = 9600;
    DelayTime = (SendLength*11*1000*1000)/Speed + 5000; // 多加5ms
    usleep(DelayTime);
}

void SerialCommu::CommitToDataBase(INT32S port, INT32S LogicIndex,DETAIL_VALUE *pCurValue)
{
    INT8U  tLogicAddr  = port * MAX_COM_RTU_NUM + LogicIndex;
    INT8U  tFrameType  = pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType;
    INT8U  tIsIntern   = pDeviceDataBase->DeviceValue[tLogicAddr].isIntern;
    INT8U  tGroupIndex = pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex;
    INT8U  tSource     = pDeviceDataBase->DeviceValue[tLogicAddr].Source;

    qDebug()<<__FUNCTION__<<__LINE__<<tFrameType;

    INT8U  tValueIndex = 0;

    switch(tFrameType)
    {
    case FRAME_SWITCH:
        pRWLocker->lockForWrite();
        memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_SWITCH],pCurValue,sizeof(DETAIL_VALUE));
        pRWLocker->unlock();
        emit UpdateDevStatusSignal(tLogicAddr,tFrameType,tGroupIndex);
        break;
    case FRAME_MEASURE:
        switch(pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex)
        {
        case 0:
            pRWLocker->lockForWrite();
            if(tIsIntern)
                memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_MEASURE_INTER_1],pCurValue,sizeof(DETAIL_VALUE));
            else
                memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_MEASURE_1],pCurValue,sizeof(DETAIL_VALUE));
            pRWLocker->unlock();
            break;
        case 1:
            pRWLocker->lockForWrite();
            if(tIsIntern)
                memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_MEASURE_INTER_2],pCurValue,sizeof(DETAIL_VALUE));
            else
                memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_MEASURE_2],pCurValue,sizeof(DETAIL_VALUE));
            pRWLocker->unlock();
            break;
        case 2:
            pRWLocker->lockForWrite();
            memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_MEASURE_3],pCurValue,sizeof(DETAIL_VALUE));
            pRWLocker->unlock();
            break;
        case 3:
            pRWLocker->lockForWrite();
            memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_MEASURE_4],pCurValue,sizeof(DETAIL_VALUE));
            pRWLocker->unlock();
            break;
        case 4:
            pRWLocker->lockForWrite();
            memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_MEASURE_5],pCurValue,sizeof(DETAIL_VALUE));
            pRWLocker->unlock();
            break;
        case 5:
            pRWLocker->lockForWrite();
            memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_MEASURE_6],pCurValue,sizeof(DETAIL_VALUE));
            pRWLocker->unlock();
            break;
        }
        emit UpdateDevStatusSignal(tLogicAddr,tFrameType,tGroupIndex);
        break;
    case FRAME_FAULT:
        break;
    case FRAME_ELECTRIC:
        break;
    case FRAME_CONSTANT_READ: //
        if(tGroupIndex == 0)
            tValueIndex = INFO_CONSTANT_1;
        else
            tValueIndex = INFO_CONSTANT_2;

        pRWLocker->lockForWrite();
        memcpy(&pDeviceDataBase->DeviceValue[tLogicAddr].value[tValueIndex],pCurValue,sizeof(DETAIL_VALUE));
        pRWLocker->unlock();
        emit UpdateDevConstantSignal(tLogicAddr,tFrameType,tSource,tGroupIndex);
        break;
    case FRAME_CONSTANT_WRITE: //
        emit UpdateDevConstantSignal(tLogicAddr,tFrameType,tSource,tGroupIndex);
        break;
    case FRAME_CONSTANT_SINGLE_WRITE:
        emit UpdateDevConstantSignal(tLogicAddr,tFrameType,tSource,tGroupIndex);
        break;
    case FRAME_RESET:
    case FRAME_RELAYOFF:
    case FRAME_RELAYON:
    case FRAME_TEST1:
    case FRAME_COIL_OPERATION:
        emit UpdateControlCoilSignal(tLogicAddr,tFrameType,tSource,tGroupIndex);
        break;
    }
}

void SerialCommu::handleCommError(INT32S port, INT32S LogicIndex, INT8U ErrorType)
{
    INT8U tLogicAddr  = port * MAX_COM_RTU_NUM + LogicIndex;
    INT8U tFrameType  = pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType;
    INT8U tGroupIndex = pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex;
    INT8U tSource     = pDeviceDataBase->DeviceValue[tLogicAddr].Source;

    switch(tFrameType)
    {
    case FRAME_CONSTANT_READ:
    case FRAME_CONSTANT_WRITE:
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = ErrorType;
        emit UpdateDevConstantSignal(tLogicAddr,tFrameType,tSource,tGroupIndex);
        break;

    case FRAME_FAULT:
        break;
    case FRAME_ELECTRIC: //todo
        break;
    case FRAME_RELAYON:
    case FRAME_RELAYOFF:
    case FRAME_RESET:
    case FRAME_TEST1:
    case FRAME_COIL_OPERATION:
        emit UpdateControlCoilSignal(tLogicAddr,tFrameType,tSource,tGroupIndex);
        break;
    case FRAME_CUSTOMER_ID:
        pDeviceDataBase->DeviceValue[tLogicAddr].CodeMatchStatus = TO_INSIDE_NOK;//
        break;
    }
}

void SerialCommu::DebugPrintPkt(INT8U *pData, INT8U len, char *pTitle)
{
        INT32S i;
        printf("%s:",pTitle);
        for(i = 0; i < len; i++)
        {
            printf("%02X ",pData[i]);
        }
        printf("\n");
}
