#include "modbustcpserver.h"
#include "DefineDb.h"
#include <QDebug>
#include <QSemaphore>

modbustcpserver::modbustcpserver(QObject *parent) :
    QThread(parent)
{
    //this->start();
    memset(mConstantReadFlag,READ_CONSTANT_UNDO,sizeof(INT8U)*MAX_RTU_NUM*2);

}
void modbustcpserver::initserver()
{
    mRes = -1;
    mRes = TCPServerInit(server_port,&serverfd);
    if(mRes == -1)
    {
        printf("server init error = %d!\n",mRes);
    }
    qDebug()<<"server %d init ok!\n" <<serverfd;

    memset(recbytes,0x00,MAX_LEN_TCP_BYTES);
    memset(sendbytes,0x00,MAX_LEN_TCP_BYTES);
    memset(&serverPointTable,0x00,sizeof(PointTableST));
}


/*************************************************************************************
@ 函数功能：  run 线程方法
@ 输入参数：
@ 返回参数：
@ 创建作者：hb
@ 创建时间：2020.07.04
*************************************************************************************/
void modbustcpserver::run()
{
    //qDebug()<<"modbustcp server start1"<<server_port;
#if 1
    mRes = -1;
    while(1)
    {
       // qDebug()<<"modbustcp server start2"<<serverfd;
        if(mRes == -1)
        {
            //qDebug()<<"modbustcp server start3"<<serverfd;
            mRes = TCPServerWaitConnection(serverfd,&clientfd,clientaddr);
            if(mRes == -1)
            {
                qDebug()<<"accept not ok";
                continue;
            }
            qDebug()<<"client %d connect ok,addr!"<<clientfd<<clientaddr<<serverfd;
        }

        memset(recbytes,0,MAX_LEN_TCP_BYTES);
        memset(sendbytes,0,MAX_LEN_TCP_BYTES);
        int bytes = TCPReadData(clientfd,recbytes,MAX_LEN_TCP_BYTES,5000);
        if(bytes > 0)
        {
            int len = handle_rec_to_send(sendbytes,recbytes,bytes);
            if(len > 0)
                TCPWrite(clientfd,sendbytes,len);
        }
        else
        {
            TCPClientClose(clientfd);
            mRes = -1;
        }
        usleep(10000);
    }
 #endif
}

INT32S modbustcpserver::handle_rec_to_send(INT8U *buffer, INT8U *rec_pkt, INT32S rec_size)
{
#if 1
    //qDebug()<<"handle_rec_to_send"<<rec_size<<rec_pkt[7];
    memcpy(tcpbuffer,rec_pkt,rec_size);   // 响应前面的字节和请求相等

    u16 RegStartAddr = ((rec_pkt[8] << 8) | rec_pkt[9]) + 1;
    u16 RegNum = (rec_pkt[10] << 8) | rec_pkt[11];
    int len = 0;
    qDebug()<<"handle_rec_to_send"<<rec_size<<rec_pkt[7]<<RegStartAddr<<RegNum;
    switch(rec_pkt[7])
    {
//    case ReadHoldCoil: // 01
//         len = handleReadHoldCoil(RegStartAddr,RegNum,rec_size);
//         break;
    case ReadInputCoil:  //02 OK
         len=handleReadInputCoil(RegStartAddr,RegNum,rec_size);
         break;
    case ReadHoldReg:    //03
         len=handleReadHoldReg(RegStartAddr,RegNum,rec_size);
         break;
    case ReadInputReg:   //04 OK
         len=handleReadInputReg(RegStartAddr,RegNum,rec_size);
         break;
//    case WriteHoldCoil:  //05
//         len=handleWriteHoldCoil(RegStartAddr,RegNum,rec_size);
//         break;
    case WriteHoldReg:   //06
         len=handleWriteHoldReg(RegStartAddr,RegNum,rec_size); // here RegNum is Value to write
         break;
    default:break;
    }
    memcpy(buffer,tcpbuffer,len);
    return len;
 #endif
}

INT32S modbustcpserver::handle_error(INT32S len)
{
    qDebug()<<"ERROR HAPPEN";
    tcpbuffer[FUNNUM] |= 0x80;  // error happen , fun code + 0x80
    return len;
}
/*************************************************************************************
@ 函数功能：  01功能码 暂时返回值均为0
@ 输入参数：
@ 返回参数：
@ 创建作者：hb
@ 创建时间：2020.07.04
*************************************************************************************/
INT32S modbustcpserver::handleReadHoldCoil(INT16U reg_start, INT16U reg_num, INT32S recv_size)  // 01
{
    if(reg_num%8 == 0)
        tcpbuffer[8]  = reg_num/8;
    else
        tcpbuffer[8]  = reg_num/8 + 1;

    int bytenum = tcpbuffer[8];

    for(INT32S i = 0; i < bytenum;i++)
        tcpbuffer[9 + i] = 0;

    tcpbuffer[5] = tcpbuffer[8] + 3;

    return (tcpbuffer[5] + 6);

}
/*************************************************************************************
@ 函数功能：  02功能码   包含设备通讯状态和各台设备的输入线圈量，其中如果走的是内部协议，该部分直接为0
@ 输入参数：
@ 返回参数：
@ 创建作者：hb
@ 创建时间：2020.07.04
*************************************************************************************/

INT32S modbustcpserver::handleReadInputCoil(INT16U reg_start, INT16U reg_num, INT32S recv_size)//0x02 OK
{
    INT32S len = 0;
    if((reg_start >= InputCoil_02_START)&&((reg_start + reg_num - 1) <= InputCoil_02_END))// 1区
    {
        updateInputCoilPointTable();
        len = handleReadInputCoilProcessing(reg_start,reg_num);
    }
    else
    {
       len = handle_error(recv_size);// 功能码+0x80,其余原样返回
    }
    return len;
}

void modbustcpserver::updateInputCoilPointTable()
{
    INT32S i = 0;
    INT32S j = 0;
    INT32S tPointIndex = 0;

    INT32S tPointNum = 0;
    INT8U  tLogicAddr = 0;
    INT32S tTypeIndex = 0;
    INT32S tVerIndex = 0;

    INT32S tComPort = 0;

    memset(serverPointTable.InputCoilPointTable,0x00,1000);

    DETAIL_VALUE      tDeviceInfo;
    COMMUNICATE_VALUE tCommInfo;

    for(i = 0;i < AllDevNum; i++)
    {
        tLogicAddr = LogicAddrArray[i];
        if(tLogicAddr < MAX_RTU_NUM)
        {
            tComPort = tLogicAddr/MAX_COM_RTU_NUM;

            pRWLocker[tComPort]->lockForRead();
            memcpy(&tCommInfo,&pDeviceDataBase->DeviceValue[tLogicAddr].commu,sizeof(COMMUNICATE_VALUE));
            pRWLocker[tComPort]->unlock();

            qDebug()<<__FUNCTION__<<__LINE__<<tCommInfo.status<<tLogicAddr<<tComPort;

            if(tCommInfo.status == COMM_STATUS_ON)
                serverPointTable.InputCoilPointTable[tPointIndex] = 1;
            else
                serverPointTable.InputCoilPointTable[tPointIndex] = 0;
            tPointIndex++;
        }
    }

    for(i = 0;i < AllDevNum; i++) // attention:内部协议请看模拟量之中的值，线圈这边会一直为0
    {
         tLogicAddr = LogicAddrArray[i];
        if(tLogicAddr < MAX_RTU_NUM)
        {
            tComPort = tLogicAddr/MAX_COM_RTU_NUM;
            pRWLocker[tComPort]->lockForRead();
            memcpy(&tDeviceInfo,&pDeviceDataBase->DeviceValue[tLogicAddr].value[INFO_SWITCH],sizeof(DETAIL_VALUE));
            pRWLocker[tComPort]->unlock();

            tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeIndex;
            tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[tLogicAddr].rtu_typeVer;

            tPointNum = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].TeleActionSection.DisplayTotal;
            qDebug()<<"tPointNum="<<tPointNum;
            for(j = 0;j < tPointNum; j++)
            {
                serverPointTable.InputCoilPointTable[tPointIndex] = (tDeviceInfo.ValueData[j])&0x1;
                tPointIndex++;
            }
        }
    }
}

INT32S modbustcpserver::handleReadInputCoilProcessing(INT16U reg_start, INT16U reg_num)
{
    //字节计数
    INT32S startaddr = 0;
    startaddr = reg_start - InputCoil_02_START;

    if(reg_num%8 == 0)
        tcpbuffer[8]  = reg_num/8;
    else
        tcpbuffer[8]  = reg_num/8 + 1;

    INT32S byteindex = 0;

    INT8U value = 0;

    for(INT32S i = 0;i < reg_num; i++)
    {
        switch(i%8)
        {
        case 7:
            if(serverPointTable.InputCoilPointTable[i + startaddr])
                value = value + 128;
            break;
        case 6:
            if(serverPointTable.InputCoilPointTable[i + startaddr])
                value = value + 64;
            break;
        case 5:
            if(serverPointTable.InputCoilPointTable[i + startaddr])
                value = value + 32;
            break;
        case 4:
            if(serverPointTable.InputCoilPointTable[i + startaddr])
                value = value + 16;
            break;
        case 3:
            if(serverPointTable.InputCoilPointTable[i + startaddr])
                value = value + 8;
            break;
        case 2:
            if(serverPointTable.InputCoilPointTable[i + startaddr])
                value = value + 4;
            break;
        case 1:
            if(serverPointTable.InputCoilPointTable[i + startaddr])
                value = value + 2;
            break;
        case 0:
            if(serverPointTable.InputCoilPointTable[i + startaddr])
                value = value + 1;
            break;
        }

        if((i+1)%8 == 0) // every 8 bits, add one byte
        {
            tcpbuffer[9 + byteindex] = value;
            byteindex += 1;
            value = 0;
        }

        if(reg_num%8 != 0) // last byte
        {
            if(i == reg_num -1)
                tcpbuffer[9+ byteindex] = value;
        }
    }
    qDebug()<<"tcpbuffer[9]="<<tcpbuffer[9]<<tcpbuffer[10];
    tcpbuffer[5] = tcpbuffer[8] + 3;
    return (tcpbuffer[5] + 6);
}

INT32S modbustcpserver::handleReadInputReg(INT16U reg_start, INT16U reg_num, INT32S recv_size)//0x04
{
    //qDebug() << reg_start << reg_num;
    // reg_start
    INT32S len = 0;

    // update 16 and 32
    if((reg_start >= InputReg_04_16_START)&&(reg_start + reg_num -1 <= InputReg_04_16_END))
    {
        updateInputReg16PointTable();
        qDebug()<<__FUNCTION__<<__LINE__;
        len = handleReadInputRegProcessing(reg_start,reg_num,INPUT_REG_04_16);
    }
    else if((reg_start >= InputReg_04_32_START)&&(reg_start + reg_num -1 <= InputReg_04_32_END))
    {
        updateInputReg32PointTable();
        len = handleReadInputRegProcessing(reg_start,reg_num,INPUT_REG_04_32);
    }
    else if((reg_start >= InputReg_Fault_START)&&(reg_start + reg_num -1 <= InputReg_Fault_END))
    {
        updateInputRegFaultPointTable();
        len = handleReadInputRegProcessing(reg_start,reg_num,INPUT_REG_FAULT);
    }
    else if((reg_start >= InputReg_ELEC_START)&&(reg_start + reg_num -1 <= InputReg_ELEC_END))
    {
        updateInputRegElecPointTable();
        len = handleReadInputRegProcessing(reg_start,reg_num,INPUT_REG_ELEC);
    }
    else
    {
        len = handle_error(recv_size);// 功能码+0x80,其余原样返回
    }
    return len;
}

//void modbustcpserver::updateInputRegPointTable() // 04
//{
//    ;
//}

void modbustcpserver::updateInputReg16PointTable()// 16bits
{
    INT32S typeindex = 0;
    INT32S verindex = 0;
    INT32S i = 0;
    INT32S j = 0;
    INT32S k = 0;
    INT32S logicaddr = 0;
    INT32S displayTotal = 0;
//    INT32S displayTotal2 = 0;

    INT32S pointIndex = 0;
//  INT32S interexistFlag = 0;

    INT32S tInternExist = 0;
    INT32S tValueIndex  = 0;

    INT32S tComPort = 0;

    DETAIL_VALUE tDeviceInfo;

    qDebug()<<__FUNCTION__<<__LINE__;
    for(i = 0; i < AllDevNum; i++)
    {
        logicaddr = LogicAddrArray[i];
        typeindex = pDeviceDataBase->SysCfgData.DevInfo[logicaddr].rtu_typeIndex;
        verindex  = pDeviceDataBase->SysCfgData.DevInfo[logicaddr].rtu_typeVer;

        tInternExist = pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].InterExist;

        tComPort = logicaddr/MAX_COM_RTU_NUM;
        qDebug()<<__FUNCTION__<<__LINE__;
        if(tInternExist)
        {
            for(j=0;j<pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureInternTotalNum;j++)
            {
                if(j==0)
                    tValueIndex = INFO_MEASURE_INTER_1;
                else
                    tValueIndex = INFO_MEASURE_INTER_2;

                displayTotal  = pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureInternSection[j].DisplayTotal;
                if(displayTotal > 0)
                {
                     pRWLocker[tComPort]->lockForRead();
                     memcpy(&tDeviceInfo,&pDeviceDataBase->DeviceValue[logicaddr].value[tValueIndex],sizeof(DETAIL_VALUE));
                     pRWLocker[tComPort]->unlock();

                     for(k = 0; k < displayTotal;k++)
                     {
                         if(pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureInternSection[j].Point[k].bitsnum != 32)
                         {
                              serverPointTable.InputReg16Table[pointIndex]   = (tDeviceInfo.ValueData[k]>>8)& 0xFF;
                              serverPointTable.InputReg16Table[pointIndex+1] = (tDeviceInfo.ValueData[k])& 0xFF;
                              pointIndex += 2;
                         }
                     }
                 }
            }
        }
        else
        {
            qDebug()<<__FUNCTION__<<__LINE__<<pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeaureTotalNum;
            for(j = 0;j < pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeaureTotalNum; j++)
            {
                switch(j)
                {
                case 0:
                    tValueIndex = INFO_MEASURE_1;
                    break;
                case 1:
                    tValueIndex = INFO_MEASURE_2;
                    break;
                case 2:
                    tValueIndex = INFO_MEASURE_3;
                    break;
                case 3:
                    tValueIndex = INFO_MEASURE_4;
                    break;
                case 4:
                    tValueIndex = INFO_MEASURE_5;
                    break;
                case 5:
                    tValueIndex = INFO_MEASURE_6;
                    break;
                default:
                    tValueIndex = INFO_MEASURE_1;
                    break;
                }

                displayTotal  = pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureSection[j].DisplayTotal;
                qDebug()<<__FUNCTION__<<__LINE__<<displayTotal;
                if(displayTotal > 0)
                {
                    pRWLocker[tComPort]->lockForRead();
                    memcpy(&tDeviceInfo,&pDeviceDataBase->DeviceValue[logicaddr].value[tValueIndex],sizeof(DETAIL_VALUE));
                    pRWLocker[tComPort]->unlock();
                    qDebug()<<__FUNCTION__<<__LINE__;
                    for(k = 0; k < displayTotal;k++)
                    {
                        if(pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureSection[j].Point[k].bitsnum != 32)
                        {
                              serverPointTable.InputReg16Table[pointIndex]   = (tDeviceInfo.ValueData[k] >> 8)& 0xFF;
                              serverPointTable.InputReg16Table[pointIndex+1] = (tDeviceInfo.ValueData[k]) & 0xFF;
                              pointIndex += 2;
                        }
                    }
                }
            }
         }
    }
    qDebug()<<"test HBHB1111"<<logicaddr<<tInternExist<<__FUNCTION__<<__LINE__;
}

void modbustcpserver::updateInputReg32PointTable() //32bits
{
    //serverPointTable.InputReg32Table[1000];
    INT32S typeindex = 0;
    INT32S  verindex = 0;
    INT32S i = 0;
    INT32S j = 0;
    INT32S k = 0;
    INT32S logicaddr = 0;
    INT32S displayTotal = 0;
//    INT32S displayTotal2 = 0;

    INT32S pointIndex = 0;

 //   INT32S interexistFlag = 0;
    INT32S tInternExist = 0;

    INT32S tValueIndex = 0;
    INT32S tComPort = 0;

    DETAIL_VALUE tDeviceInfo;

    for(i = 0; i < AllDevNum; i++ )
    {
         logicaddr = LogicAddrArray[i];
         typeindex = pDeviceDataBase->SysCfgData.DevInfo[logicaddr].rtu_typeIndex;
         verindex  = pDeviceDataBase->SysCfgData.DevInfo[logicaddr].rtu_typeVer;

         tInternExist = pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].InterExist;
         tComPort =  logicaddr/MAX_COM_RTU_NUM;

         if(tInternExist)
         {
             for(j = 0;j < pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureInternTotalNum;j++)
             {
                 if(j == 0)
                     tValueIndex = INFO_MEASURE_INTER_1;
                 else
                     tValueIndex = INFO_MEASURE_INTER_2;

                 displayTotal  = pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureInternSection[j].DisplayTotal;
                 if(displayTotal > 0)
                 {
                      pRWLocker[tComPort]->lockForRead();
                      memcpy(&tDeviceInfo,&pDeviceDataBase->DeviceValue[logicaddr].value[tValueIndex],sizeof(DETAIL_VALUE));
                      pRWLocker[tComPort]->unlock();

                      for(k = 0; k < displayTotal;k++)
                      {
                          if(pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureInternSection[j].Point[k].bitsnum == 32)
                          {
                              serverPointTable.InputReg32Table[pointIndex] =   (tDeviceInfo.ValueData[k] >>24)&0xFF;
                              serverPointTable.InputReg32Table[pointIndex+1] = (tDeviceInfo.ValueData[k] >>16)&0xFF;
                              serverPointTable.InputReg32Table[pointIndex+2] = (tDeviceInfo.ValueData[k] >>8) &0xFF;
                              serverPointTable.InputReg32Table[pointIndex+3] = (tDeviceInfo.ValueData[k])&0xFF;
                              pointIndex += 4;
                          }
                      }
                  }
             }
         }
         else
         {
             for(j = 0;j < pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeaureTotalNum; j++)
             {
                 switch(j)
                 {
                 case 0:
                     tValueIndex = INFO_MEASURE_1;
                     break;
                 case 1:
                     tValueIndex = INFO_MEASURE_2;
                     break;
                 case 2:
                     tValueIndex = INFO_MEASURE_3;
                     break;
                 case 3:
                     tValueIndex = INFO_MEASURE_4;
                     break;
                 case 4:
                     tValueIndex = INFO_MEASURE_5;
                     break;
                 case 5:
                     tValueIndex = INFO_MEASURE_6;
                     break;
                 default:
                     tValueIndex = INFO_MEASURE_1;
                     break;
                 }

                 displayTotal  = pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureSection[j].DisplayTotal;
                 if(displayTotal > 0)
                 {
                     pRWLocker[tComPort]->lockForRead();
                     memcpy(&tDeviceInfo,&pDeviceDataBase->DeviceValue[logicaddr].value[tValueIndex],sizeof(DETAIL_VALUE));
                     pRWLocker[tComPort]->unlock();

                     for(k = 0; k < displayTotal;k++)
                     {
                         if(pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].MeasureSection[j].Point[k].bitsnum == 32)
                         {
                             serverPointTable.InputReg32Table[pointIndex]   = (tDeviceInfo.ValueData[k] >> 24)&0xFF;
                             serverPointTable.InputReg32Table[pointIndex+1] = (tDeviceInfo.ValueData[k] >> 16)&0xFF;
                             serverPointTable.InputReg32Table[pointIndex+2] = (tDeviceInfo.ValueData[k] >> 8)&0xFF;
                             serverPointTable.InputReg32Table[pointIndex+3] = (tDeviceInfo.ValueData[k])&0xFFFF;
                             pointIndex += 4;
                         }
                     }
                 }
             }
          }
     }
}

void modbustcpserver::updateInputRegFaultPointTable()
{
    INT32S typeindex = 0;
    INT32S  verindex = 0;
    INT32S i = 0;
    INT32S j = 0;
    INT32S logicaddr = 0;
    INT32S displayTotal = 0;

    INT32S pointIndex = 0;
    INT32S tComPort   = 0;
    DETAIL_VALUE tDeviceInfo;

    for(i = 0; i < AllDevNum; i++)
    {
        logicaddr = LogicAddrArray[i];
        typeindex = pDeviceDataBase->SysCfgData.DevInfo[logicaddr].rtu_typeIndex;
        verindex  = pDeviceDataBase->SysCfgData.DevInfo[logicaddr].rtu_typeVer;

        tComPort = logicaddr/MAX_COM_RTU_NUM;

        displayTotal  = pDeviceDataBase->SysCfgData.ProtoCfgData[typeindex][verindex].Faultsection.DisplayTotal;

        if(displayTotal > 0)
        {
            pRWLocker[tComPort]->lockForRead();
            memcpy(&tDeviceInfo,&pDeviceDataBase->DeviceValue[logicaddr].value[INFO_FAULT],sizeof(DETAIL_VALUE));
            pRWLocker[tComPort]->unlock();

            for(j = 0; j < displayTotal;j++)
            {
                 serverPointTable.InputRegFaultTable[pointIndex] =   (tDeviceInfo.ValueData[j] >> 24)&0xFF;
                 serverPointTable.InputRegFaultTable[pointIndex+1] = (tDeviceInfo.ValueData[j] >> 16)&0xFF;
                 serverPointTable.InputRegFaultTable[pointIndex+2] = (tDeviceInfo.ValueData[j] >> 8)&0xFF;
                 serverPointTable.InputRegFaultTable[pointIndex+3] = (tDeviceInfo.ValueData[j])&0xFF;
                 pointIndex += 4;
            }
        }
    }
}

void modbustcpserver::updateInputRegElecPointTable()
{
    //to do;
    memset(serverPointTable.InputRegElecTable,0x00,1000*2);
}

INT32S modbustcpserver::handleReadInputRegProcessing(INT16U reg_start, INT16U reg_num, INT32S pointtypeIndex)
{
    INT32S start = 0;
    INT32S pointIndex = 9;

    switch(pointtypeIndex)
    {
    case INPUT_REG_04_16:
        start = (reg_start - InputReg_04_16_START)*2;
        break;
    case INPUT_REG_04_32:
        start = (reg_start - InputReg_04_32_START)*2;
        break;
    case INPUT_REG_FAULT:
        start = (reg_start - InputReg_Fault_START)*2;
        break;
    case INPUT_REG_ELEC:
        start = (reg_start - InputReg_ELEC_START)*2;
        break;
    }

    pointIndex = 9;

    for(INT32S i=0;i < reg_num*2; i += 2)
    {
        switch(pointtypeIndex)
        {
        case INPUT_REG_04_16:
            tcpbuffer[pointIndex]   = (serverPointTable.InputReg16Table[start+i]);
            tcpbuffer[pointIndex+1] = (serverPointTable.InputReg16Table[start+i+1]);
            pointIndex += 2;
            break;
        case INPUT_REG_04_32:
            tcpbuffer[pointIndex]   = (serverPointTable.InputReg32Table[start+i]);
            tcpbuffer[pointIndex+1] = (serverPointTable.InputReg32Table[start+i+1]);
            pointIndex += 2;
            break;
        case INPUT_REG_FAULT: //bits
            tcpbuffer[pointIndex]   = (serverPointTable.InputRegFaultTable[start+i]);
            tcpbuffer[pointIndex+1] = (serverPointTable.InputRegFaultTable[start+i+1]);
            pointIndex += 2;
            break;
        case INPUT_REG_ELEC: //bits
            tcpbuffer[pointIndex]   = (serverPointTable.InputRegElecTable[start+i]);
            tcpbuffer[pointIndex+1] = (serverPointTable.InputRegElecTable[start+i+1]);
            pointIndex += 2;
            break;
        }
    }

    tcpbuffer[8] = reg_num * 2;
    tcpbuffer[5] = tcpbuffer[8] + 3;
    return (tcpbuffer[5]+6);
}

INT32S modbustcpserver::handleReadHoldReg(INT16U reg_start, INT16U reg_num, INT32S recv_size)//0x03 read
{
    INT32S len = 0;

    if((reg_start >= HoldReg_06_START)&&((reg_start + reg_num - 1) <= HoldReg_06_END))  //1-200
    {
        len = handleReadHoldRegProcessing(reg_start,reg_num);

    }
    else if((reg_start >= HoldReg_06_SINGLE_START)&&((reg_start + reg_num - 1) <= HoldReg_06_SINGLE_END)) //201 -300
    {
        len = handleReadHoldRegSingleProcessing(reg_start,reg_num);
    }
    else if((reg_start >= HoldReg_06_CONTROL_START)&&((reg_start + reg_num - 1) <= HoldReg_06_SUBSTATION_END)) //1001-1100
    {
        serverPointTable.HoldRegControlTable[100] = 0;
        serverPointTable.HoldRegControlTable[101] = pDeviceDataBase->PumpNum;
        serverPointTable.HoldRegControlTable[102] = 0;
        serverPointTable.HoldRegControlTable[103] = pDeviceDataBase->StationRLMode;
        serverPointTable.HoldRegControlTable[104] = 0;
        serverPointTable.HoldRegControlTable[105] = pDeviceDataBase->StationAMMode;

        serverPointTable.HoldRegControlTable[106] = 0;
        serverPointTable.HoldRegControlTable[107] = pDeviceDataBase->OneKeySwith;
        serverPointTable.HoldRegControlTable[108] = 0;
        serverPointTable.HoldRegControlTable[109] = pDeviceDataBase->Mainsumpinterlock;
        serverPointTable.HoldRegControlTable[110] = 0;
        serverPointTable.HoldRegControlTable[111] = pDeviceDataBase->AuxiSumpinterlock;
        serverPointTable.HoldRegControlTable[114] = 0;
        serverPointTable.HoldRegControlTable[115] = pDeviceDataBase->OneKeyRelateCtlPump;
        serverPointTable.HoldRegControlTable[116] = 0;
        serverPointTable.HoldRegControlTable[117] = pDeviceDataBase->PumpStartNo;

        for(int i = 0; i < 20;i++)
        {
            serverPointTable.HoldRegControlTable[118+2*i]   = (pDeviceDataBase->SafeData[i] >>8) & 0xFF;
            serverPointTable.HoldRegControlTable[118+2*i+1] = pDeviceDataBase->SafeData[i] & 0xFF;
        }

        //the other point have been updated when doing write to get the result.
        len = handleReadControlHoldRegProcessing(reg_start,reg_num);
    }
    else
    {
        len = handle_error(recv_size);// 功能码+0x80,其余原样返回
    }
    return len;
}

INT32S modbustcpserver::handleReadControlHoldRegProcessing(INT16U reg_start, INT16U reg_num)
{
    INT32S start = 0;
    INT32S pointIndex = 9;

    start = (reg_start - HoldReg_06_CONTROL_START)*2;

    pointIndex = 9;
    for(INT32S i=0;i < reg_num*2; i+=2)
    {
        tcpbuffer[pointIndex]     = serverPointTable.HoldRegControlTable[start+i];
        tcpbuffer[pointIndex + 1] = serverPointTable.HoldRegControlTable[start+i+1];
        pointIndex +=2;
    }

    tcpbuffer[8] = reg_num * 2;
    tcpbuffer[5] = tcpbuffer[8] + 3;
    return (tcpbuffer[5]+6);
}


INT32S modbustcpserver::handleReadHoldRegProcessing(INT16U reg_start, INT16U reg_num)//0x03 0x06
{
    INT32S start = 0;
    INT32S pointIndex = 9;

    start = (reg_start - HoldReg_06_START)*2;

    pointIndex = 9;
    for(INT32S i=0;i < reg_num*2; i+=2)
    {
        tcpbuffer[pointIndex]   = serverPointTable.HoldRegTable[start+i];
        tcpbuffer[pointIndex + 1] = serverPointTable.HoldRegTable[start+i+1];
        pointIndex +=2;
    }

    tcpbuffer[8] = reg_num * 2;
    tcpbuffer[5] = tcpbuffer[8] + 3;
    return (tcpbuffer[5]+6);
}

INT32S modbustcpserver::handleReadHoldRegSingleProcessing(INT16U reg_start,INT16U reg_num)
{
    INT32S start = 0;
    INT32S pointIndex = 9;

    start = (reg_start - HoldReg_06_SINGLE_START)*2;

    pointIndex = 9;
    for(INT32S i=0;i < reg_num*2; i+=2)
    {
        tcpbuffer[pointIndex]   = serverPointTable.HoldRegSingleWTable[start+i];
        tcpbuffer[pointIndex + 1] = serverPointTable.HoldRegSingleWTable[start+i+1];
        pointIndex +=2;
    }

    tcpbuffer[8] = reg_num * 2;
    tcpbuffer[5] = tcpbuffer[8] + 3;
    return (tcpbuffer[5]+6);
}

INT32S modbustcpserver::handleWriteHoldCoil(INT16U reg_start, INT16U reg_value, INT32S recv_size)//0x05
{
    return 0;
}

INT32S modbustcpserver::handleWriteHoldReg(INT16U reg_start, INT16U reg_num, INT32S recv_size)//0x06
{
    INT32S len = 0;
    INT32S logicIndex = 0;
    INT8U logicaddr = 0;
    INT32S pointIndex = 0;

    INT16U tCMD = 0;
    INT8U tChannelOrGroupId = 0;
    INT16U tStatus = 0;

    if((reg_start >=HoldReg_06_SUBSTATION_START)&&(reg_start <= HoldReg_06_SUBSTATION_END))//1051-1100
    {
        switch(reg_start)
        {
        case HoldReg_06_SUBSTATION_PUMPNUM: //泵组数量
             pDeviceDataBase->PumpNum = reg_num;
             serverPointTable.HoldRegControlTable[100] = 0;
             serverPointTable.HoldRegControlTable[101] = pDeviceDataBase->PumpNum;
             break;
        case HoldReg_06_SUBSTATION_ControlMode:   //0就地；1远控
            if(reg_num == REMOTE_MODE)
                pDeviceDataBase->StationRLMode = REMOTE_MODE;
            else
                pDeviceDataBase->StationRLMode = LOCAL_MODE;
            serverPointTable.HoldRegControlTable[102] = 0;
            serverPointTable.HoldRegControlTable[103] = pDeviceDataBase->StationRLMode;
            emit remote_switch_local_remote_signal(pDeviceDataBase->StationRLMode);
             break;
        case HoldReg_06_SUBSTATION_AMMode:        //0手动；1自动
            if(reg_num == AUTO_MODE)
                pDeviceDataBase->StationAMMode = AUTO_MODE;
            else
                pDeviceDataBase->StationAMMode = MANUAL_MODE;
            serverPointTable.HoldRegControlTable[104] = 0;
            serverPointTable.HoldRegControlTable[105] = pDeviceDataBase->StationAMMode;
            emit remote_auto_manual_signal(pDeviceDataBase->StationAMMode);
            break;
        case HoldReg_06_SUBSTATION_OneKeySwitch:  //一键切换1=1#->2#;2= 2#->1#;3=3#->4#;4=4#->3#;
            if(reg_num != 0)
            {
                pDeviceDataBase->OneKeySwith = reg_num;
                serverPointTable.HoldRegControlTable[106] = 0;
                serverPointTable.HoldRegControlTable[107] = pDeviceDataBase->OneKeySwith;
                emit remote_switch_onekey_switch_signal(pDeviceDataBase->OneKeySwith);
                qDebug()<<"emit remote_switch_onekey_switch_signal"<<reg_num;
            }
            break;
        case HoldReg_06_SUBSTATION_LowPumpCfgOn:  // 低压泵组闭锁投退
            pDeviceDataBase->Mainsumpinterlock = reg_num;
            serverPointTable.HoldRegControlTable[108] = 0;
            serverPointTable.HoldRegControlTable[109] = pDeviceDataBase->Mainsumpinterlock;
            break;
        case HoldReg_06_SUBSTATION_HighPumpCfgOn: // 高压泵组闭锁投退
            pDeviceDataBase->AuxiSumpinterlock = reg_num;
            serverPointTable.HoldRegControlTable[110] = 0;
            serverPointTable.HoldRegControlTable[111] = pDeviceDataBase->AuxiSumpinterlock;
            break;
        case HoldReg_06_SUBSTATION_SaveCfg:       // 保存数据 = 1
            //TODO
            if(reg_num == 0xFF00)
            {
               emit lockfilewriteSIGNAL();
            }
            serverPointTable.HoldRegControlTable[112] = 0;
            serverPointTable.HoldRegControlTable[113] = 0;
            break;
        case HoldReg_06_SUBSTATION_OneKeyUniCtl:  // 一键启停是否联控水泵，1是；0否
            pDeviceDataBase->OneKeyRelateCtlPump = reg_num;
            serverPointTable.HoldRegControlTable[114] = 0;
            serverPointTable.HoldRegControlTable[115] = pDeviceDataBase->OneKeyRelateCtlPump;
            break;
        case HoldReg_06_SUBSTATION_PumpStartNo:   // 瓦斯泵运行时起水泵组 1=1#水泵;2=2#水泵
            pDeviceDataBase->PumpStartNo = reg_num;
            serverPointTable.HoldRegControlTable[116] = 0;
            serverPointTable.HoldRegControlTable[117] = pDeviceDataBase->PumpStartNo;
            break;
        }
        if((reg_start >= HoldReg_06_SUBSTATION_SafeDataStart)&&(reg_start <= HoldReg_06_SUBSTATION_SafeDataEnd)) //安规数据
        {
            INT8U tIndex = reg_start - HoldReg_06_SUBSTATION_SafeDataStart;
            pDeviceDataBase->SafeData[tIndex] = reg_num;
        }
    }
    else if((reg_start >= HoldReg_06_CONTROL_START)&&(reg_start <= HoldReg_06_CONTROL_END)) //1001-1050
    {
        logicIndex = reg_start - HoldReg_06_CONTROL_START;
        logicaddr = LogicAddrArray[logicIndex];

        tCMD = reg_num&0xFF00;
        tChannelOrGroupId = reg_num&0xFF;

        if((tCMD == CMD_CONSTANT_READ)&&(tChannelOrGroupId < 2))
        {
            tStatus = STATUS_DOING + reg_num;
            serverPointTable.HoldRegControlTable[(logicIndex)*2] = (tStatus>>8)&0xFF;
            serverPointTable.HoldRegControlTable[(logicIndex)*2 + 1] = tStatus&0xFF;
            PrepareConstantReadToQueue(logicaddr,tChannelOrGroupId);
        }
        else if((tCMD == CMD_CONSTANT_WRITE)&&(tChannelOrGroupId < 2))
        {
            tStatus = STATUS_DOING+reg_num;
            serverPointTable.HoldRegControlTable[(logicIndex)*2] = (tStatus>>8)&0xFF;
            serverPointTable.HoldRegControlTable[(logicIndex)*2 + 1] = tStatus&0xFF;
            PrepareConstantWriteToQueue(logicaddr,tChannelOrGroupId);
        }
        else if((tCMD == CMD_CONSTANT_SINGLE_WRITE)&&(tChannelOrGroupId < 2))
        {
            tStatus = STATUS_DOING+reg_num;
            serverPointTable.HoldRegControlTable[(logicIndex)*2] = (tStatus>>8)&0xFF;
            serverPointTable.HoldRegControlTable[(logicIndex)*2 + 1] = tStatus&0xFF;
            PrepareSingleConstantWriteToQueue(logicaddr,tChannelOrGroupId);
        }
        else if((tCMD == CMD_CONTROL_COIL)&&(tChannelOrGroupId < MAX_CHANNEL_NUM))
        {
            tStatus = STATUS_DOING+reg_num;
            serverPointTable.HoldRegControlTable[(logicIndex)*2] = (tStatus>>8)&0xFF;
            serverPointTable.HoldRegControlTable[(logicIndex)*2 + 1] = tStatus&0xFF;
            PrepareControlCoilToQueue(logicaddr,tChannelOrGroupId);
        }
        else
        {
            handle_error(recv_size); // 命令不正确
        }
    }
    else if((reg_start >= HoldReg_06_START)&&(reg_start <= HoldReg_06_END)) //将写的定值暂存在这边,一定要先读定值才能写 1-200
    {
        pointIndex = reg_start - HoldReg_06_START;
        serverPointTable.ConstantData[pointIndex] = reg_num;
    }
    else if((reg_start >= HoldReg_06_SINGLE_START)&&(reg_start <= HoldReg_06_SINGLE_END)) //将写的定值暂存在这边,一定要先读定值才能写 201-300
    {
        pointIndex = reg_start - HoldReg_06_SINGLE_START;

        serverPointTable.ConstantDataPointIndex = pointIndex;
        serverPointTable.ConstantData[0] = reg_num;
    }
    else
    {
        len = handle_error(recv_size);
    }

    return 12;
}

INT32S modbustcpserver::PrepareConstantReadToQueue(INT8U LogicAddr, INT8U GroupIndex)
{
    INT32S tComPort    = LogicAddr/16;
    INT32S tLogicIndex = LogicAddr%16;

    QUEUE_MSG ConstantReadMsg;
    memset(&ConstantReadMsg,0x00,sizeof(QUEUE_MSG));
    if(tComPort >= 0 && tComPort <= 5) //
    {
        ConstantReadMsg.MsgType    = FRAME_CONSTANT_READ;
        ConstantReadMsg.Source     = SOURCE_TCP;
        ConstantReadMsg.LogicAddr  = LogicAddr;
        ConstantReadMsg.PhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;
        ConstantReadMsg.ComPort    = tComPort;
        ConstantReadMsg.LogicIndex = tLogicIndex;
        ConstantReadMsg.GroupIndex = GroupIndex;

        pMsgQMutex[tComPort]->lock();
        pMsgQueue[tComPort]->enqueue(ConstantReadMsg);
        pMsgQMutex[tComPort]->unlock();
    }
    return 0;
}
INT32S modbustcpserver::PrepareConstantWriteToQueue(INT8U LogicAddr, INT8U GroupIndex)
{
    INT32S tComPort    = LogicAddr/16;
    INT32S tLogicIndex = LogicAddr%16;

    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;
    INT8U tTotal = 0;

    INT8U tValueIndex = 0;
    INT8U tSectionIndex = GroupIndex;
    if(tSectionIndex <= 1)
        tTotal = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tSectionIndex].DisplayTotal;

    if(tSectionIndex == 0)
        tValueIndex = INFO_CONSTANT_1;
    else
        tValueIndex = INFO_CONSTANT_2;

//    if(tIsChanged) //
    {
        QUEUE_MSG ConstantReadMsg;
        memset(&ConstantReadMsg,0x00,sizeof(QUEUE_MSG));

        if(tComPort >= 0 && tComPort <= 5) //
        {
            ConstantReadMsg.MsgType    = FRAME_CONSTANT_WRITE;
            ConstantReadMsg.Source     = SOURCE_TCP;
            ConstantReadMsg.LogicAddr  = LogicAddr;
            ConstantReadMsg.PhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;
            ConstantReadMsg.ComPort    = tComPort;
            ConstantReadMsg.LogicIndex = tLogicIndex;
            ConstantReadMsg.GroupIndex = GroupIndex;
            ConstantReadMsg.Length     = tTotal;
            memcpy(ConstantReadMsg.ValueData,serverPointTable.ConstantData,sizeof(ConstantReadMsg.ValueData));

            qDebug()<<__FUNCTION__<<__LINE__<<LogicAddr<<GroupIndex<<tComPort;

            pMsgQMutex[tComPort]->lock();
            pMsgQueue[tComPort]->enqueue(ConstantReadMsg);
            pMsgQMutex[tComPort]->unlock();
        }
    }
    return 0;
}

INT32S modbustcpserver::PrepareSingleConstantWriteToQueue(INT8U LogicAddr, INT8U GroupIndex)
{
    INT32S tComPort    = LogicAddr/16;
    INT32S tLogicIndex = LogicAddr%16;

//    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
//    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;

//    INT8U tValueIndex = 0;
//    INT8U tSectionIndex = GroupIndex;
//    if(tSectionIndex <= 1)
//        tTotal = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tSectionIndex].DisplayTotal;

//    if(tSectionIndex == 0)
//        tValueIndex = INFO_CONSTANT_1;
//    else
//        tValueIndex = INFO_CONSTANT_2;

    {
        QUEUE_MSG ConstantReadMsg;
        memset(&ConstantReadMsg,0x00,sizeof(QUEUE_MSG));

        if(tComPort >= 0 && tComPort <= 5) //
        {
            ConstantReadMsg.MsgType    = FRAME_CONSTANT_SINGLE_WRITE;
            ConstantReadMsg.Source     = SOURCE_TCP;
            ConstantReadMsg.LogicAddr  = LogicAddr;
            ConstantReadMsg.PhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;
            ConstantReadMsg.ComPort    = tComPort;
            ConstantReadMsg.LogicIndex = tLogicIndex;
            ConstantReadMsg.GroupIndex = GroupIndex;
            ConstantReadMsg.Length     = 1;
            ConstantReadMsg.ConstantValuePointIndex = serverPointTable.ConstantDataPointIndex;
            ConstantReadMsg.ValueData[0] = serverPointTable.ConstantData[0];

            qDebug()<<__FUNCTION__<<__LINE__<<LogicAddr<<GroupIndex<<tComPort;

            pMsgQMutex[tComPort]->lock();
            pMsgQueue[tComPort]->enqueue(ConstantReadMsg);
            pMsgQMutex[tComPort]->unlock();
        }
    }
    return 0;
}

INT32S modbustcpserver::PrepareControlCoilToQueue(INT8U LogicAddr,INT8U ChannelId)
{
    INT32S tComPort    = LogicAddr/16;
    INT32S tLogicIndex = LogicAddr%16;

    QUEUE_MSG ControlMsg;
    memset(&ControlMsg,0x00,sizeof(QUEUE_MSG));
    if(tComPort >= 0 && tComPort <= 5) //
    {
        switch(ChannelId)
        {
        case CONTROL_RELAY_ON:
            ControlMsg.MsgType = FRAME_RELAYON;
            break;
        case CONTROL_RELAY_OFF:
            ControlMsg.MsgType = FRAME_RELAYOFF;
            break;
        case CONTROL_RESET:
            ControlMsg.MsgType = FRAME_RESET;
            break;
        case CONTROL_TEST:
            ControlMsg.MsgType = FRAME_TEST1;
            break;
        default:
            ControlMsg.MsgType = FRAME_COIL_OPERATION;
            break;
        }

        ControlMsg.Source     = SOURCE_TCP;
        ControlMsg.LogicAddr  = LogicAddr;
        ControlMsg.PhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;
        ControlMsg.ComPort    = tComPort;
        ControlMsg.LogicIndex = tLogicIndex;
        ControlMsg.GroupIndex = ChannelId; //

        pMsgQMutex[tComPort]->lock();
        pMsgQueue[tComPort]->enqueue(ControlMsg);
        pMsgQMutex[tComPort]->unlock();
    }

    return 0;
}
void modbustcpserver::UpdateDevConstantSlot(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U GroupIndex)
{
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8U tVerType   = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;

    if(Source == SOURCE_TCP)
    {
        INT32S i = 0;
        INT8S tPointTableIndex = -1;
        for(i=0; i < AllDevNum; i++)
        {
            if(LogicAddrArray[i] == LogicAddr)
            {
                tPointTableIndex = i;
                break;
            }
        }

        INT16U tStatusCMD = 0;
        INT8U tValueIndex = 0;

        if((tPointTableIndex >= 0)&&(tPointTableIndex < (100-1)))
        {
            //logicaddrIndex  0;  //match 1001  = point table1

            if(FrameType == FRAME_CONSTANT_WRITE)
            {
                 if(pDeviceDataBase->DeviceValue[LogicAddr].Result == RESULT_SUCCESS)
                 {
                    tStatusCMD = STATUS_SUCCESS + CMD_CONSTANT_WRITE + GroupIndex;
                    serverPointTable.HoldRegControlTable[(tPointTableIndex)*2]  = (tStatusCMD>>8)&0xFF;
                    serverPointTable.HoldRegControlTable[(tPointTableIndex)*2+1] = tStatusCMD&0xFF;
                 }
                 else
                 {
                    tStatusCMD = STATUS_FAILED + CMD_CONSTANT_WRITE + GroupIndex;
                    serverPointTable.HoldRegControlTable[(tPointTableIndex)*2] = (tStatusCMD>>8)&0xFF;
                    serverPointTable.HoldRegControlTable[(tPointTableIndex)*2+1] = tStatusCMD&0xFF;
                 }
            }
            else if(FrameType == FRAME_CONSTANT_SINGLE_WRITE)
            {
                 if(pDeviceDataBase->DeviceValue[LogicAddr].Result == RESULT_SUCCESS)
                 {
                    tStatusCMD = STATUS_SUCCESS + CMD_CONSTANT_SINGLE_WRITE + GroupIndex;
                    serverPointTable.HoldRegControlTable[(tPointTableIndex)*2]  = (tStatusCMD>>8)&0xFF;
                    serverPointTable.HoldRegControlTable[(tPointTableIndex)*2+1] = tStatusCMD&0xFF;
                 }
                 else
                 {
                    tStatusCMD = STATUS_FAILED + CMD_CONSTANT_SINGLE_WRITE + GroupIndex;
                    serverPointTable.HoldRegControlTable[(tPointTableIndex)*2] = (tStatusCMD>>8)&0xFF;
                    serverPointTable.HoldRegControlTable[(tPointTableIndex)*2+1] = tStatusCMD&0xFF;
                 }
            }
            else if(FrameType == FRAME_CONSTANT_READ)
            {
                if(pDeviceDataBase->DeviceValue[LogicAddr].Result == RESULT_SUCCESS)
                {
                   tStatusCMD = STATUS_SUCCESS + CMD_CONSTANT_READ + GroupIndex;
                   serverPointTable.HoldRegControlTable[(tPointTableIndex)*2] = (tStatusCMD>>8)&0xFF;
                   serverPointTable.HoldRegControlTable[(tPointTableIndex)*2+1] = tStatusCMD&0xFF;

                   memset(serverPointTable.HoldRegTable,0x00,sizeof(serverPointTable.HoldRegTable));
                   memset(serverPointTable.ConstantData,0x00,sizeof(serverPointTable.ConstantData));

                   if(GroupIndex == 0)
                   {
                        tValueIndex = INFO_CONSTANT_1;
                   }
                   else
                   {
                        tValueIndex = INFO_CONSTANT_2;
                   }

                   printf("#### Debug data:\n");

                   INT8U tWConstantFunCode = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerType].ConstantSection[GroupIndex].funcode;
                   if(tWConstantFunCode == WriteMoreHoldReg)//0x10
                   {
                       for(INT32S i = 0 ; (i < pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueNum)&&(i < 128);i++)
                       {
                           serverPointTable.HoldRegTable[2*(i)]   = (pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]>>8)&0xFF;
                           serverPointTable.HoldRegTable[2*(i)+1] = pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]&0xFF;

                           serverPointTable.ConstantData[i] = pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]; //
                           printf("%d ",pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]);
                       }
                       printf("\n");
                       serverPointTable.constantDataNum = pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueNum;
                   }
                   else//0x06
                   {
                       for(INT32S i = 0 ; (i < pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueNum)&&(i < 128);i++)
                       {
                           serverPointTable.HoldRegSingleWTable[2*(i)]   = (pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]>>8)&0xFF;
                           serverPointTable.HoldRegSingleWTable[2*(i)+1] = pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]&0xFF;

//                           serverPointTable.ConstantData[i] = pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]; //
//                           printf("%d ",pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]);
                       }
                       printf("\n");
                       serverPointTable.constantDataNum = 1;  // 一次写一个
                   }
                }
                else
                {
                   tStatusCMD = STATUS_FAILED + CMD_CONSTANT_READ + GroupIndex;
                   serverPointTable.HoldRegControlTable[(tPointTableIndex)*2] = (tStatusCMD>>8)&0xFF;
                   serverPointTable.HoldRegControlTable[(tPointTableIndex)*2+1] = tStatusCMD&0xFF;
                }
            }
        }
    }
}
void modbustcpserver::UpdateControlCoilSlot(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U ChannelIndex)
{
    if(Source == SOURCE_TCP)
    {
        INT32S tPointTableIndex = -1;
        INT32S i = 0;
        INT16U tStatusCMD = 0;

        for(i=0; i < AllDevNum; i++)
        {
            if(LogicAddrArray[i] == LogicAddr)
            {
                tPointTableIndex = i;
                break;
            }
        }

        if((tPointTableIndex >= 0)&&(tPointTableIndex<(100-1)))
        {
             switch(FrameType)
             {
             case FRAME_COIL_OPERATION: //开关量控制
             case FRAME_RELAYON:
             case FRAME_RELAYOFF:
             case FRAME_TEST1:
             case FRAME_RESET:
                  if(pDeviceDataBase->DeviceValue[LogicAddr].Result == RESULT_SUCCESS)
                  {
                      tStatusCMD = CMD_CONTROL_COIL + STATUS_SUCCESS + ChannelIndex;
                      serverPointTable.HoldRegControlTable[(tPointTableIndex)*2]   = (tStatusCMD>>8)&0xFF;
                      serverPointTable.HoldRegControlTable[(tPointTableIndex)*2+1] = tStatusCMD&0xFF;
                  }
                  else
                  {
                      tStatusCMD = CMD_CONTROL_COIL + STATUS_FAILED + ChannelIndex;
                      serverPointTable.HoldRegControlTable[(tPointTableIndex)*2]   = (tStatusCMD>>8)&0xFF;
                      serverPointTable.HoldRegControlTable[(tPointTableIndex)*2+1] = tStatusCMD&0xFF;
                  }
                  break;
             }
         }
    }
}

void modbustcpserver::initPointEdgeLogicAddrArray()
{
// LogicAddrArray;
    INT32S i = 0;
//    INT32S IndexStart = 0;
    INT32S tPointIndex = 0;
// Init Exist Logicaddr Array
    for(i = 0; i < MAX_COM_NUM;i++)
        AllDevNum +=  pDeviceDataBase->SysCfgData.OneComDevTotal[i];//pvalid_cfg_info->total[i];

    for(i = 0; i < pDeviceDataBase->SysCfgData.OneComDevTotal[PORT1]; i++)
    {
        LogicAddrArray[tPointIndex] = i;
        tPointIndex++;
    }

//    IndexStart = pDeviceDataBase->SysCfgData.OneComDevTotal[PORT1];
    for(i = 0; i < pDeviceDataBase->SysCfgData.OneComDevTotal[PORT2]; i++)
    {
        LogicAddrArray[tPointIndex] = 16 + i;
        tPointIndex++;
    }

//    IndexStart = pDeviceDataBase->SysCfgData.OneComDevTotal[PORT2];
    for(i = 0; i < pDeviceDataBase->SysCfgData.OneComDevTotal[PORT3]; i++)
    {
        LogicAddrArray[tPointIndex] = 32 + i;
        tPointIndex++;
    }

//    IndexStart = pDeviceDataBase->SysCfgData.OneComDevTotal[PORT3];
    for(i = 0; i < pDeviceDataBase->SysCfgData.OneComDevTotal[PORT4]; i++)
    {
        LogicAddrArray[tPointIndex] = 48 + i;
        tPointIndex++;
    }

//    IndexStart = pDeviceDataBase->SysCfgData.OneComDevTotal[PORT4];
    for(i = 0; i < pDeviceDataBase->SysCfgData.OneComDevTotal[PORT5]; i++)
    {
        LogicAddrArray[tPointIndex] = 64 + i;
        tPointIndex++;
    }

//    IndexStart = pDeviceDataBase->SysCfgData.OneComDevTotal[PORT5];
    for(i = 0; i < pDeviceDataBase->SysCfgData.OneComDevTotal[PORT6]; i++)
    {
        LogicAddrArray[tPointIndex] = 80 + i;
        tPointIndex++;
    }
//    qDebug()<<__FUNCTION__<<__LINE__<<LogicAddrArray[0]<<LogicAddrArray[1]<<LogicAddrArray[2]<<LogicAddrArray[3];
}
