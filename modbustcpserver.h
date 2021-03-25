#ifndef MODBUSTCPSERVER_H
#define MODBUSTCPSERVER_H

#include <QThread>
#include "modbustcp_base.h"
#include "DefineDb.h"
#include <QSemaphore>
#include <QReadWriteLock>
#include <QMutex>
#include "modbustcpheader.h"
#include <QQueue>

class modbustcpserver : public QThread,modbustcp_base
{
    Q_OBJECT
public:
    explicit modbustcpserver(QObject *parent = 0);

    QQueue<QUEUE_MSG> *pMsgQueue[MAX_COM_NUM];
    QMutex *pMsgQMutex[MAX_COM_NUM];
    QReadWriteLock *pRWLocker[MAX_COM_NUM];


    INT8U recbytes[MAX_LEN_TCP_BYTES];
    INT8U sendbytes[MAX_LEN_TCP_BYTES];

    Deivce_DataBase *pDeviceDataBase;

    int server_port;
    int serverfd,clientfd;

    char clientaddr[50];

    INT8U tcpbuffer[MAX_LEN_TCP_BYTES];

    INT8U  LogicAddrArray[MAX_RTU_NUM];
    INT8U  AllDevNum;

    INT8U mConstantReadFlag[MAX_RTU_NUM][2];

    void initPointEdgeLogicAddrArray();

    INT32S mRes;

    INT32S handle_rec_to_send(INT8U *buffer,INT8U *rec_pkt,INT32S rec_size);
    INT32S handle_error(INT32S len);

    PointTableST serverPointTable;

    INT32S handleReadHoldCoil(INT16U reg_start,INT16U reg_num,INT32S recv_size); //0x01
    INT32S handleReadInputCoil(INT16U reg_start, INT16U reg_num, INT32S recv_size);//0x02
    void updateInputCoilPointTable();
    INT32S handleReadInputCoilProcessing(INT16U reg_start, INT16U reg_num);

    INT32S handleReadInputReg(INT16U reg_start, INT16U reg_num,INT32S recv_size);//0x04
//    void updateInputRegPointTable();
    void updateInputReg32PointTable();
    void updateInputReg16PointTable();
    void updateInputRegFaultPointTable();
    void updateInputRegElecPointTable();


    INT32S handleReadInputRegProcessing(INT16U reg_start, INT16U reg_num, INT32S InputRegIndex);

    INT32S handleReadHoldReg(INT16U reg_start,INT16U reg_num, INT32S recv_size);//0x03

    INT32S handleReadHoldRegProcessing(INT16U reg_start,INT16U reg_num);
    INT32S handleReadHoldRegSingleProcessing(INT16U reg_start,INT16U reg_num);
    INT32S handleReadControlHoldRegProcessing(INT16U reg_start,INT16U reg_num);

//
    INT32S PrepareConstantReadToQueue(INT8U LogicAddr,INT8U GroupIndex);
    INT32S PrepareConstantWriteToQueue(INT8U LogicAddr,INT8U GroupIndex);
    INT32S PrepareSingleConstantWriteToQueue(INT8U LogicAddr,INT8U GroupIndex);
    INT32S PrepareControlCoilToQueue(INT8U LogicAddr,INT8U ChannelId);


    INT32S handleWriteHoldCoil(INT16U reg_start, INT16U reg_value, INT32S recv_size);//0x05

    INT32S handleWriteHoldReg(INT16U reg_start, INT16U reg_num, INT32S recv_size);//0x06


    void initserver();

signals:
    void remote_switch_local_remote_signal(INT8U mode);
    void remote_auto_manual_signal(INT8U mode);

    void remote_switch_onekey_switch_signal(INT8U Cmd);
    void lockfilewriteSIGNAL();



public slots:
    void UpdateDevConstantSlot(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U GroupIndex);
    void UpdateControlCoilSlot(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U ChannelIndex);



protected:
    void run();
};

#endif
