#include "SerialCommu8330.h"
#include <QDebug>
SerialCommu8330::SerialCommu8330()
{

}

INT16U SerialCommu8330::EquipCommuCodeMatchingPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer)
{
    INT32S tLogicAddr = ComPort *MAX_COM_RTU_NUM + LogicIndex;

    INT8U tPhyAddr   = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;
    INT16U tDeviceAD = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].ad;

    INT16U tCalcLinkID = Intrinsic_Function_For_ID(tDeviceAD,0);
    INT16U tLength = 0;
    INT16U tCrcRes = 0;

    qDebug()<<__FUNCTION__<<__LINE__<<tPhyAddr<<tLogicAddr<<ComPort;

    buffer[tLength++] = tPhyAddr;                    //
    buffer[tLength++] = InterUserCompare;
//    buffer[tLength++] = 6;                          //字节长度  8330 length not exist
    buffer[tLength++] = (tDeviceAD >> 8) & 0xFF;    //广告编号
    buffer[tLength++] = tDeviceAD & 0xFF;
    buffer[tLength++] = (tCalcLinkID >> 8) & 0xFF;  //计算联网ID
    buffer[tLength++] = tCalcLinkID & 0xFF;
    buffer[tLength++] = 0;
    buffer[tLength++] = 0;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;

    return (INT16U)tLength;
}

INT32S SerialCommu8330::RecvRemainBytes(INT32U port, INT32U LogicIndex, INT8U *rBuf) // ok
{
    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
    INT8U tFrameType = pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType;
    INT32S tRemainBytes = 0;

    if((rBuf[1] >> 7)== 1) //modbus标准协议发生错误，响应帧为5bytes,头收了3bytes还剩2bytes.
    {
        return 2;
    }
    else
    {
        switch(tFrameType)
        {
        case FRAME_SWITCH:
        case FRAME_MEASURE:
        case FRAME_CONSTANT_READ:
        case FRAME_FAULT:
            tRemainBytes = rBuf[2] + 2;
            break;
        case FRAME_CONSTANT_WRITE:
        case FRAME_CONSTANT_SINGLE_WRITE:
            tRemainBytes = 5;
            break;
        case FRAME_CUSTOMER_ID:
            tRemainBytes = 9;
            break;
        default:
            tRemainBytes = 5;
            break;
        }
        return tRemainBytes;
    }
}

void SerialCommu8330::HandleEquipCodeMatch(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT8U tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;
    if(Buffer[7] == 0x55) //
    {
         qDebug()<<__FUNCTION__<<__LINE__<<tLogicAddr;
         pDeviceDataBase->DeviceValue[tLogicAddr].isIntern = PROTO_PRIV_YES;
         pDeviceDataBase->DeviceValue[tLogicAddr].CodeMatchStatus = TO_INSIDE_OK;
    }
    else
    {
        qDebug()<<__FUNCTION__<<__LINE__;
        pDeviceDataBase->DeviceValue[tLogicAddr].CodeMatchStatus = TO_INSIDE_NOK;
        pDeviceDataBase->DeviceValue[tLogicAddr].isIntern = PROTO_PRIV_NO;
    }
}

void SerialCommu8330::HandleInterruptControlCoil(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT8U  tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;
    if((Buffer[1] >> 7) != 1)
    {
        if((Buffer[4] == 0x00)&&(Buffer[5] == 0x55))
            pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_SUCCESS;
        else
            pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_FAILED;
    }
    else
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_FAILED;
    }
}
