#ifndef SERIALCOMMU8330_H
#define SERIALCOMMU8330_H

#include "SerialCommuModbus.h"

class SerialCommu8330 : public SerialCommuModbus
{
public:
    SerialCommu8330();

    INT16U EquipCommuCodeMatchingPackagePrepare(INT32S LogicIndex,INT32S ComPort,INT8U *buffer);

    INT32S RecvRemainBytes(INT32U port, INT32U LogicIndex, INT8U *rBuf);

    void HandleEquipCodeMatch(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);

//    INT16U BuildPackage(INT8U *pkt,INT8U dev_addr,INT8U fun,INT16U reg_addr,INT16U reg_num,INT8U *data,INT8U datalen);

    void HandleInterruptControlCoil(INT32U port, INT32U LogicIndex, INT8U *Buffer, INT8U size, DETAIL_VALUE *pCurValue);

};

#endif // SERIALCOMMU8330_H
