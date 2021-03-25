#ifndef CONSTANT_H
#define CONSTANT_H
#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "DefineDb.h"
#include <QReadWriteLock>
#include <QMutex>
#include <QQueue>
namespace Ui {
    class Constant;
}

class Constant : public QWidget {
    Q_OBJECT
public:
    Constant(QWidget *parent = 0);
    ~Constant();

    Deivce_DataBase *pDeviceDataBase;

    QQueue<QUEUE_MSG> *pMsgQueue[MAX_COM_NUM];

    QMutex    *pMsgQMutex[MAX_COM_NUM];

    void EnterProcess();

    void DisplayConstantInfo(INT32S LogicAddr,INT32S GroupIndex);

    void ReadConstant(INT32S LogicAddr,INT32S GroupIndex);

    void WriteConstant(INT32S LogicAddr,INT32S GroupIndex);

    void UpdateConstant(INT8U LogicAddr, INT8U FrameType, INT8U GroupIndex);
    void GetConstantFromUI(INT8U LogicAddr, INT8U GroupIndex);
    INT32S IsConstantChanged(INT8U LogicAddr, INT8U GroupIndex);

    INT32U mCurrentConstantArray[128];

    INT32S constantnum = 0;

    INT32S mLogicAddr;
    INT32S mSelectCode;
    INT32S mGroupIndex;
    INT32S mMaxGroupNum;
    INT8U  mWriteConstantFunCode;
    INT8U  mChangedPointIndex; //当该设备配置为单个定值写时，记录当前设备的修改定值的pointIndex

    INT8U mConstantG1ReadStep;
    INT8U mConstantG2ReadStep;
    INT8U mEnterFlag;

    int selectSETS =0;int selectDevIndex =0;int currentLogicAddr = 0;int selectDataGroup = 0;
    int selectRW =0;

    void updateSETSUI();
    void updateDevUI();
    void updateRWUI();
    void selectRWProcess();
    void selectDevProcess();

public slots:
    void UpdateDevConstantSlot(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U GroupIndex);

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    Ui::Constant *ui;

    void key_f1_press();
    void key_f2_press();
    void key_f3_press();
    void key_return_press();
    void key_escape_press();
};

#endif // CONSTANT_H
