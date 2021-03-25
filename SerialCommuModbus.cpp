#include "SerialCommuModbus.h"
#include <QDebug>

SerialCommuModbus::SerialCommuModbus()
{


}

INT16U SerialCommuModbus::CRCCheck(INT8U *data, INT8U len)
{
    int i;
    INT16U crc = 0XFFFF;
    while(len--)
    {
        crc ^= *data++;
        for(i=0;i<8;i++)
        {
            if(crc & 0x01)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}


INT16U SerialCommuModbus::EquipCommuCodeMatchingPackagePrepare(INT32S LogicIndex, INT32S ComPort, INT8U *buffer)
{
//    qDebug()<<__FUNCTION__<<__LINE__;
    INT32S LogicAddr = ComPort *MAX_COM_RTU_NUM + LogicIndex;

    INT8U PhyAddr   = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;
    INT16U tDeviceAD = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].ad;
    INT16U tCalcLinkID = Intrinsic_Function_For_ID(tDeviceAD,0);
    INT16U tCrcRes = 0;
    INT8U tLength = 0;
    qDebug()<<__FUNCTION__<<__LINE__<<PhyAddr<<LogicAddr<<ComPort;

    buffer[tLength++] = PhyAddr;                    //
    buffer[tLength++] = InterUserCompare;
    buffer[tLength++] = 6;                          //字节长度
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

INT16U SerialCommuModbus::Intrinsic_Function_For_ID(INT16U ad, INT16U code)
{
    INT16U  adver;
    INT16U  value_ID;
    adver = ad;
    value_ID = adver*4 + code;
    return value_ID;
}

INT16U SerialCommuModbus::ReadCoilPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer)
{
    INT16U tRegAddr  = 0;
    INT16U tRegNum   = 0;
    INT8U  tFunCode  = 0;
    INT8U  tPhyAddr  = 0;

    INT32S tLogicAddr = ComPort *MAX_COM_RTU_NUM + LogicIndex;
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    tPhyAddr  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;
    tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].TeleActionSection.startaddr;
    tRegNum  = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].TeleActionSection.CommuTotal;
    tFunCode = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].TeleActionSection.funcode;

    INT8U tLength = 0;
    INT16U tCrcRes = 0;

    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = tFunCode;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tRegNum >> 8) & 0xFF;
    buffer[tLength++] = tRegNum & 0xFF;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;
    return (INT16U)tLength;
//    return BuildPackage(buffer,PhyAddr,FunCode,RegAddr,RegNum,NOT_USED_POINTER,0);
}

INT16U SerialCommuModbus::ReadMeasurePackagePrepare(INT32S LogicIndex, INT32S ComPort, INT8U *buffer, INT8U GroupIndex)
{
    INT16U tRegAddr = 0;
    INT16U tRegNum  = 0;

    INT8U  tFunCode  = 0;

    INT32S tLogicAddr = ComPort *MAX_COM_RTU_NUM + LogicIndex;
    INT8U  tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U  tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
    INT8U  tPhyAddr   = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;

    INT8U  tSectionIndex = GroupIndex;// from 0 to 5

    tFunCode = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].funcode;

    if(tSectionIndex <= 5)
    {
        tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].startaddr;
        tRegNum  = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].CommuTotal;
    }

    INT8U tLength = 0;
    INT16U tCrcRes = 0;

    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = tFunCode;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tRegNum >> 8) & 0xFF;
    buffer[tLength++] = tRegNum & 0xFF;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;
    return (INT16U)tLength;
}

INT16U SerialCommuModbus::ReadFaultPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer)
{
    INT16U tRegAddr = 0;
    INT16U tRegNum  = 0;

    INT32S tLogicAddr = ComPort *MAX_COM_RTU_NUM + LogicIndex;
    INT8U tTypeIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U tVerIndex   = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
    INT8U tPhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;

    tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].Faultsection.startaddr;
    tRegNum  = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].Faultsection.CommuTotal;

    INT8U tLength = 0;
    INT16U tCrcRes = 0;

    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = ReadInputReg;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tRegNum >> 8) & 0xFF;
    buffer[tLength++] = tRegNum & 0xFF;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;

    return (INT16U)tLength;
}

INT16U SerialCommuModbus::ReadWattHourPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer)
{
    INT32S tLogicAddr = ComPort *MAX_COM_RTU_NUM + LogicIndex;
    INT8U  tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U  tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
    INT8U  tPhyAddr  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;

    INT16U tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ElecDegreeSection.startaddr;
    INT16U tRegNum  = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ElecDegreeSection.CommuTotal;

    INT8U tLength = 0;
    INT16U tCrcRes = 0;

    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = ReadInputReg;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tRegNum >> 8) & 0xFF;
    buffer[tLength++] = tRegNum & 0xFF;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;

    return (INT16U)tLength;
}

INT16U SerialCommuModbus::ReadInterMeasurePackagePrepare(INT32S LogicIndex, INT32S ComPort, INT8U *buffer, INT8U GroupIndex)
{
    INT16U tRegAddr = 0;
    INT16U tRegNum  = 0;

    INT32S tLogicAddr = ComPort *MAX_COM_RTU_NUM + LogicIndex;
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
    INT8U tPhyAddr  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;
    INT8U tSectionIndex = GroupIndex;// from 0 to 5
    qDebug()<<"####"<<__FUNCTION__<<__LINE__<<tSectionIndex;
    if(tSectionIndex <= 1)
    {
        tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].startaddr;
        tRegNum  = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].CommuTotal;
    }

    INT8U tLength = 0;
    INT16U tCrcRes = 0;
    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = InterReadInputReg;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tRegNum >> 8) & 0xFF;
    buffer[tLength++] = tRegNum & 0xFF;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;

    return (INT16U)tLength;
}

INT16U SerialCommuModbus::InterruptReadConstantPackagePrepare(QUEUE_MSG *pMsg, INT8U *buffer)
{
    INT16U tRegAddr = 0;
    INT16U tRegNum  = 0;

    INT32S LogicAddr = pMsg->ComPort * MAX_COM_RTU_NUM + pMsg->LogicIndex;
    INT8U TypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8U VerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;
    INT8U tPhyAddr   = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;

    INT8U tSectionIndex = pMsg->GroupIndex;

    pDeviceDataBase->DeviceValue[LogicAddr].CurrentFrameType = FRAME_CONSTANT_READ;
    pDeviceDataBase->DeviceValue[LogicAddr].GroupIndex = pMsg->GroupIndex;

    pDeviceDataBase->DeviceValue[LogicAddr].Source = pMsg->Source;

    if(tSectionIndex <= 1)
    {
        tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[TypeIndex][VerIndex].ConstantSection[tSectionIndex].startaddr;
        tRegNum  = pDeviceDataBase->SysCfgData.ProtoCfgData[TypeIndex][VerIndex].ConstantSection[tSectionIndex].CommuTotal;
    }

    INT8U  tLength = 0;
    INT16U tCrcRes = 0;
    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = ReadHoldReg;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tRegNum >> 8) & 0xFF;
    buffer[tLength++] = tRegNum & 0xFF;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;

    return (INT16U)tLength;
}

INT16U SerialCommuModbus::InterruptWriteSingleConstanPackagePrepare(QUEUE_MSG *pMsg,INT8U *buffer)
{
    INT16U tRegAddr = 0;
    INT16U tRegValue  = 0;

    INT32S tLogicAddr = pMsg->ComPort * MAX_COM_RTU_NUM + pMsg->LogicIndex;
    INT8U  TypeIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U  VerIndex   = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
    INT8U  tPhyAddr   = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;

    INT8U tSectionIndex = pMsg->GroupIndex;

    pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = FRAME_CONSTANT_SINGLE_WRITE;
    pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex = pMsg->GroupIndex;
    pDeviceDataBase->DeviceValue[tLogicAddr].Source = pMsg->Source;

    INT8U tPointIndex = pMsg->ConstantValuePointIndex;

    tRegAddr  = pDeviceDataBase->SysCfgData.ProtoCfgData[TypeIndex][VerIndex].ConstantSection[tSectionIndex].Point[tPointIndex].addr;
    tRegValue = pMsg->ValueData[0]&0xFFFF;

    INT8U tLength = 0;
    INT16U tCrcRes = 0;

    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = WriteHoldReg;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tRegValue >> 8) & 0xFF;
    buffer[tLength++] = tRegValue & 0xFF;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++] = tCrcRes & 0xFF;
    buffer[tLength++] = (tCrcRes >> 8) & 0xFF;

    return (INT16U)tLength;
}

INT16U SerialCommuModbus::InterruptWriteConstantPackagePrepare(QUEUE_MSG *pMsg,INT8U *buffer)
{
    INT16U tRegAddr = 0;
    INT16U tRegNum  = 0;

    INT32S LogicAddr = pMsg->ComPort * MAX_COM_RTU_NUM + pMsg->LogicIndex;
    INT8U TypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8U VerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;
    INT8U tPhyAddr   = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;

    INT8U tSectionIndex = pMsg->GroupIndex;

    pDeviceDataBase->DeviceValue[LogicAddr].CurrentFrameType = FRAME_CONSTANT_WRITE;
    pDeviceDataBase->DeviceValue[LogicAddr].GroupIndex = pMsg->GroupIndex;

    INT8U tValueIndex = 0;
    if(tSectionIndex == 0)
       tValueIndex = INFO_CONSTANT_1;
    else
       tValueIndex = INFO_CONSTANT_2;
    pDeviceDataBase->DeviceValue[LogicAddr].Source = pMsg->Source;

    INT8U tDataValueArray[256];
    memset(tDataValueArray,0x00,256);
    memcpy(tDataValueArray,pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].CurrentConstantTelegramData,256);  // restore the old value

//    printf("######previous LogicAddr=%d tValueIndex=%d\n",LogicAddr,tValueIndex);
//    for(INT32S i = 0; i < 256;i++)
//    {
//        printf("%02X ",pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].CurrentConstantTelegramData[i]);
//    }
//    printf("\n");

    INT8U tPos = 0;
    for(INT32S i = 0 ; i < pMsg->Length;i++)
    {
        tPos = (pDeviceDataBase->SysCfgData.ProtoCfgData[TypeIndex][VerIndex].ConstantSection[tSectionIndex].Point[i].addr -
                pDeviceDataBase->SysCfgData.ProtoCfgData[TypeIndex][VerIndex].ConstantSection[tSectionIndex].startaddr)*2;
        if(tPos < 255)
        {
            tDataValueArray[tPos]   =  (pMsg->ValueData[i] >>8) & 0xFF;
            tDataValueArray[tPos+1] =  pMsg->ValueData[i] & 0xFF;
        }
        qDebug()<<__FUNCTION__<<__LINE__<<pMsg->ValueData[i]
                <<tDataValueArray[tPos]<<tDataValueArray[tPos+1];
    }

//    printf("######ValueData length=%d\n",pMsg->Length);
//    for(INT32S i = 0; i < pMsg->Length;i++)
//    {
//        printf("%d ",pMsg->ValueData[i]);
//    }
//    printf("\n");

//    printf("######processed\n");
//    for(INT32S i = 0; i < 256;i++)
//    {
//        printf("%02X ",tDataValueArray[i]);
//    }
//    printf("\n");

    tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[TypeIndex][VerIndex].ConstantSection[tSectionIndex].startaddr;
    tRegNum  = pDeviceDataBase->SysCfgData.ProtoCfgData[TypeIndex][VerIndex].ConstantSection[tSectionIndex].CommuTotal;

    INT8U tDataLength = tRegNum *2;
    INT8U tLength = 0;
    INT16U tCrcRes = 0;

//    return BuildPackage(buffer,tPhyAddr,WriteMoreHoldReg,tRegAddr,tRegNum,tDataValueArray,tLength);
//        INT8U *pkt,INT8U dev_addr,INT8U fun,INT16U reg_addr,INT16U reg_num,INT8U *data,INT8U datalen
    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = WriteMoreHoldReg;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tRegNum >> 8) & 0xFF;
    buffer[tLength++] = tRegNum & 0xFF;
    buffer[tLength++] = tRegNum * 2;
    memcpy(&buffer[tLength],tDataValueArray,tDataLength);
    tLength += tDataLength;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;

    return (INT16U)tLength;
}

INT16U SerialCommuModbus::InterruptControlCoilPackagePrepare(QUEUE_MSG *pMsg, INT8U *buffer)
{
    INT16U tRegAddr = 0;

    INT32S tLogicAddr = pMsg->ComPort * MAX_COM_RTU_NUM + pMsg->LogicIndex;
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
    INT8U tPhyAddr   = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;

    INT8U tChannelIndex = pMsg->GroupIndex;
    INT16U tCMD = 0;

    pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType = pMsg->MsgType;
    pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex = pMsg->GroupIndex;

    pDeviceDataBase->DeviceValue[tLogicAddr].Source = pMsg->Source;

    if(tChannelIndex < MAX_CHANNEL_NUM)
    {
        tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].SwitchControlSection.Point[tChannelIndex].addr;
        tCMD     = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].SwitchControlSection.Point[tChannelIndex].cmd;
    }
//    return BuildPackage(buffer,tPhyAddr,WriteHoldCoil,tRegAddr,0xFF00,NULL,0);
//                   INT8U *pkt,INT8U dev_addr,INT8U fun,INT16U reg_addr,INT16U reg_num,INT8U *data,INT8U datalen

    INT8U tLength = 0;
    INT16U tCrcRes = 0;
    buffer[tLength++] = tPhyAddr;
    buffer[tLength++] = WriteHoldCoil;
    buffer[tLength++] = (tRegAddr >> 8) & 0xFF;
    buffer[tLength++] = tRegAddr & 0xFF;
    buffer[tLength++] = (tCMD >> 8)&0xFF;//0xFF;
    buffer[tLength++] = tCMD & 0xFF;
    tCrcRes = CRCCheck(buffer,tLength);
    buffer[tLength++]= tCrcRes & 0xFF;
    buffer[tLength++]= (tCrcRes >> 8) & 0xFF;
    return (INT16U)tLength;
}

/*modbus标准协议*/
INT32S SerialCommuModbus::RecvHeadBytes(INT32U port, INT32U LogicIndex)
{
    return 3;
}

INT32S SerialCommuModbus::RecvHeadBytesWaitTime(INT32U port, INT32U LogicIndex)
{
    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
    INT8U tFrameType = pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType;
    INT32S tDelayTime = 0;
    switch(tFrameType)
    {
    case  FRAME_SWITCH:       //0开关量   一般用于外部协议
    case  FRAME_MEASURE:      //1 模拟量 内外均有，根据配置文件解析一个函数
    case  FRAME_CUSTOMER_ID:
        tDelayTime = 250;
        break;
    default:
        tDelayTime = 4000;
        break;
    }
    return tDelayTime;
}

INT32S SerialCommuModbus::RecvRemainBytes(INT32U port, INT32U LogicIndex,INT8U *rBuf)
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
        case FRAME_CUSTOMER_ID:
            tRemainBytes = rBuf[2] + 2;
            break;
        case FRAME_CONSTANT_WRITE:
        case FRAME_CONSTANT_SINGLE_WRITE:
            tRemainBytes = 5;
            break;
        default:
            tRemainBytes = 5;
            break;
        }
        return tRemainBytes;
    }
}

INT32S SerialCommuModbus::RecvRemainBytesWaitTime(INT32U port, INT32U LogicIndex)
{
//    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
//    INT8U tFrameType = pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType;
    return 4000;
}
INT32S SerialCommuModbus::CheckFrameOK(INT32U port, INT32U LogicIndex,INT8U *Buffer, INT8U size)
{
    INT8U tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;
    INT8U tPhyAddr = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].phy_addr;
    INT8U tFrameType = pDeviceDataBase->DeviceValue[tLogicAddr].CurrentFrameType;

    INT16U tCrcRes = 0;

    qDebug()<<__FUNCTION__<<__LINE__<<Buffer[0]<<tPhyAddr;
    if(Buffer[0] != tPhyAddr)
    {
        return -1;
    }
    else
    {
        tCrcRes = CRCCheck(Buffer,size-2);
        if((Buffer[size-1] != ((tCrcRes>>8) & 0xFF)) || Buffer[size-2] != (tCrcRes&0xFF))
        {
            return -3;
        }
        if((Buffer[1] >> 7) == 1 && tFrameType == FRAME_MEASURE)
        {
            return -4;
        }
        if((Buffer[1] >> 7) == 1 && tFrameType == FRAME_CUSTOMER_ID)
        {
            return -5;
        }
    }
    return 0;
}

void SerialCommuModbus::HandleReadCoil(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size,DETAIL_VALUE *pCurValue)
{
//    INT16U tRegAddr = 0;
//    INT16U tRegNum  = 0;
    INT8U tLogicAddr= port*MAX_COM_RTU_NUM + LogicIndex;

    INT16U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
    INT16U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT8U pos = 0;
    INT8U bitpos = 0;

    pCurValue->ValueNum = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].TeleActionSection.DisplayTotal;

    for(INT32S i = 0; i < pCurValue->ValueNum; i++) // 76543210  = pos(3)        15 14 13 12 11 10 9 8  = pos(4)  0 -43
    {
            pos =   (pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].TeleActionSection.Point[i].bitIndex/8 + 3);
            bitpos = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].TeleActionSection.Point[i].bitIndex%8;
            pCurValue->ValueData[i] = (Buffer[pos]>>bitpos)&0x01;
    }
}

void SerialCommuModbus::HandleReadMeasure(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
     INT16U tRegStartAddr = 0;
     INT16U tRegAddr = 0;
    // INT16U tRegNum  = 0;
     INT8U tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;

     INT16U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;//content.rtu_type[logicIndex];
     INT16U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

     INT8S tSectionIndex = pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex;

     INT8U tPos = 0;
     INT32S i = 0;
     INT32U tBitIndex = 0;

     if(tSectionIndex >= 0 && tSectionIndex <= 5)
     {
          pCurValue->ValueNum = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].DisplayTotal;
          tRegStartAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].startaddr;
          qDebug()<<"processed data:"<<pCurValue->ValueNum<<tRegStartAddr;
          for(i=0;i< pCurValue->ValueNum;i++)
          {
              tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].Point[i].addr;
              tBitIndex = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].Point[i].BitIndex;
              tPos = (tRegAddr - tRegStartAddr)*2 + 3;

              if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].Point[i].bitsnum == 32)
              {
                  pCurValue->ValueData[i] = (Buffer[tPos]<<24) | (Buffer[tPos+1]<<16) | (Buffer[tPos+2]<<8) | (Buffer[tPos+3]);
              }
              else if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].Point[i].bitsnum == 16)
              {
                  pCurValue->ValueData[i] = (Buffer[tPos]<<8) | Buffer[tPos+1];
              }
              else if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].Point[i].bitsnum == 1)
              {
                  pCurValue->ValueData[i] = (((Buffer[tPos]<<8) | Buffer[tPos+1]) >> tBitIndex)&0x1;
              }
              else if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].Point[i].bitsnum == 2)
              {
                  pCurValue->ValueData[i] = (((Buffer[tPos]<<8) | Buffer[tPos+1]) >> tBitIndex)&0x3;
              }
              else if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureSection[tSectionIndex].Point[i].bitsnum == 8) //8 others not support
              {
                  pCurValue->ValueData[i] = (((Buffer[tPos]<<8) | Buffer[tPos+1]) >> tBitIndex)&0xFF;
              }
              qDebug()<<pCurValue->ValueData[i]<<tRegAddr<<tBitIndex<<tPos;
          }
     }
}

void SerialCommuModbus::HandleReadInternMeasure(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT16U tRegStartAddr = 0;
    INT16U tRegAddr = 0;
   // INT16U tRegNum  = 0;
    INT8U tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;

    INT16U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;//content.rtu_type[logicIndex];
    INT16U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    INT8S tSectionIndex = pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex;

    qDebug()<<__FUNCTION__<<__LINE__<<tSectionIndex;

    INT8U tPos = 0;
    INT32S i = 0;

    INT32U tBitIndex = 0;

    if(tSectionIndex >= 0 && tSectionIndex <= 5)
    {
         pCurValue->ValueNum = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].DisplayTotal;
         tRegStartAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].startaddr;

//         qDebug()<<__FUNCTION__<<__LINE__<<pCurValue->ValueNum<<tRegStartAddr;
         for(i=0;i< pCurValue->ValueNum;i++)
         {
             tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].Point[i].addr;
             tBitIndex = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].Point[i].BitIndex;
             tPos = (tRegAddr - tRegStartAddr)*2 + 3;

             if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].Point[i].bitsnum == 32)
             {               
                 pCurValue->ValueData[i] = Buffer[tPos]<<24 | Buffer[tPos+1]<<16 | Buffer[tPos+2]<<8 | Buffer[tPos+3];
             }
             else if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].Point[i].bitsnum == 16)
             {                             
                 pCurValue->ValueData[i] = Buffer[tPos]<<8 | Buffer[tPos+1];
             }
             else if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].Point[i].bitsnum == 1)
             {
                 pCurValue->ValueData[i] = ((Buffer[tPos]<<8 | Buffer[tPos+1]) >> tBitIndex)&0x1;
             }
             else if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].Point[i].bitsnum == 2)
             {
                 pCurValue->ValueData[i] = ((Buffer[tPos]<<8 | Buffer[tPos+1]) >> tBitIndex)&0x3;
             }
             else if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].MeasureInternSection[tSectionIndex].Point[i].bitsnum == 8) //8 others not support
             {                
                 pCurValue->ValueData[i] = ((Buffer[tPos]<<8 | Buffer[tPos+1]) >> tBitIndex)&0xFF;
             }
//             qDebug()<<__FUNCTION__<<__LINE__<<i<<tRegAddr<<tBitIndex<<tPos<<pCurValue->ValueData[i];
         }
    }
}

void SerialCommuModbus::HandleEquipCodeMatch(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT8U tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;
    if(Buffer[8] == 0x55) //
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

void SerialCommuModbus::HandleReadWattHour(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT16U tRegStartAddr = 0;
    INT16U tRegAddr = 0;
   // INT16U tRegNum  = 0;
    INT8U tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;

    INT16U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;//content.rtu_type[logicIndex];
    INT16U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

    pCurValue->ValueNum = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ElecDegreeSection.DisplayTotal;

    INT32S tPos = 3;
    INT32S i = 0;
    if((Buffer[1] >> 7) != 1)
    {
        tRegStartAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ElecDegreeSection.startaddr;
        for(i = 0; i < pCurValue->ValueNum; i++)
        {
            tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ElecDegreeSection.Point[i].addr;

            if(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ElecDegreeSection.Point[i].bitsnum == 32)
            {
                tPos = (tRegAddr - tRegStartAddr)*2 + 3;
                pCurValue->ValueData[i] = Buffer[tPos]<<24 | Buffer[tPos+1]<<16 | Buffer[tPos+2]<<8 | Buffer[tPos+3];
            }
        }
    }
}

void SerialCommuModbus::HandleInterruptConstantRead(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT16U tRegAddr = 0;
    INT16U tRegNum  = 0;
    INT8U  tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;
    INT16U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;//
    INT16U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
    INT8U  tGroupIndex = pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex;

    INT8U  tValueIndex = 0;
    if((Buffer[1] >> 7) != 1)
    {
        tRegAddr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tGroupIndex].startaddr;
        tRegNum  = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tGroupIndex].DisplayTotal;

        if(tGroupIndex == 0)
        {
            tValueIndex = INFO_CONSTANT_1;
        }
        else
        {
            tValueIndex = INFO_CONSTANT_2;
        }

        memset(pCurValue->CurrentConstantTelegramData,0x00,256);
        memcpy(pCurValue->CurrentConstantTelegramData,Buffer+3,size -2 -3);

//        printf("######previous read size=%d tLogicAddr=%d tValueIndex=%d\n",size,tLogicAddr,tValueIndex);
//        for(INT32S i = 0; i < size - 5;i++)
//        {
//            printf("%02X ",pDeviceDataBase->DeviceValue[tLogicAddr].value[tValueIndex].CurrentConstantTelegramData[i]);
//        }
//        printf("\n");

        pCurValue->ValueNum = tRegNum;
        INT32S tPos = 0;
        INT32S i = 0;
        for(i = 0 ;i < pCurValue->ValueNum;i++)
        {
            tPos = 3 + (pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tGroupIndex].Point[i].addr
                        - pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tGroupIndex].startaddr)*2;

            pCurValue->ValueData[i] = Buffer[tPos]<<8 | Buffer[tPos+1];
        }
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_SUCCESS;
    }
}

void SerialCommuModbus::HandleInterruptConstantWrite(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
//    INT16U tRegAddr = 0;
//    INT16U tRegNum  = 0;
    INT8U  tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;
//    INT16U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;//
//    INT16U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;
//    INT8U  tGroupIndex = pDeviceDataBase->DeviceValue[tLogicAddr].GroupIndex;

//    INT8U  tValueIndex = 0;
    if((Buffer[1] >> 7) != 1)
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_SUCCESS;
    }
    else
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_FAILED;
    }
}

void SerialCommuModbus::HandleInterruptSingleConstantWrite(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT8U  tLogicAddr = port * MAX_COM_RTU_NUM + LogicIndex;

    if((Buffer[1] >> 7) != 1)
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_SUCCESS;
    }
    else
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_FAILED;
    }
}

void SerialCommuModbus::HandleInterruptControlCoil(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue)
{
    INT8U  tLogicAddr = port*MAX_COM_RTU_NUM + LogicIndex;
    if((Buffer[1] >> 7) != 1)
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_SUCCESS;
    }
    else
    {
        pDeviceDataBase->DeviceValue[tLogicAddr].Result = RESULT_FAILED;
    }
}
