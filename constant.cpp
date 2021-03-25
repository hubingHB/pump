#include "constant.h"
#include "ui_constant.h"
#include <qmessagebox.h>
#include <QTableWidgetItem>
#include <QDebug>
#include <QPushButton>
#define CONSTANT_MAX_SELECT 2

#define FLAG_READ           1
#define FLAG_WRITE          2

#define GROUP_INDEX_ONE     0
#define GROUP_INDEX_TWO     1

Constant::Constant(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Constant)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  //  qDebug()<<"Constant";
}

Constant::~Constant()
{
    delete ui;
}

void Constant::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Constant::EnterProcess()
{
    qDebug()<<"constant EnterProcess";

    selectSETS = 0;
    updateSETSUI(); //SET SELECT
    selectDevIndex = 0;
    currentLogicAddr = 0;
    updateDevUI();   //GROUP SELECT
    selectDataGroup =0;
    selectRW = 0;
    updateRWUI();  //READ OR WRITE
    ui->label_pumpgroup1->setStyleSheet("background-color: rgb(0, 255, 0);");
    ui->label_pumproom->setStyleSheet("background-color: rgb(255, 255, 255);");
    ui->label_pumpgroup2->setStyleSheet("background-color: rgb(255, 255, 255);");
    ui->label_pumpgroup3->setStyleSheet("background-color: rgb(255, 255, 255);");
    ui->label_pumpgroup4->setStyleSheet("background-color: rgb(255, 255, 255);");

    DisplayConstantInfo(0,GROUP_INDEX_ONE);
    mEnterFlag = 1;
    mConstantG1ReadStep = READ_CONSTANT_UNDO;
    mConstantG2ReadStep = READ_CONSTANT_UNDO;
    if(pDeviceDataBase->DeviceValue[0].commu.status == COMM_STATUS_ON)
        ReadConstant(0,GROUP_INDEX_ONE);
}

void Constant::DisplayConstantInfo(INT32S LogicAddr, INT32S GroupIndex)
{
    mSelectCode = FLAG_READ;
    mGroupIndex = GroupIndex;
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;

    mMaxGroupNum = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantTotalNum;

//    if(mMaxGroupNum == 2)
//    {
//        ui->label_Group1->setStyleSheet("background-color: rgb(0, 255, 0);");
//        ui->label_Group2->setStyleSheet("background-color: rgb(255, 255, 255);");
//    }
//    else
//    {
//        ui->label_Group1->hide();
//        ui->label_Group2->hide();
//    }

//    ui->read->setStyleSheet("background-color: rgb(0, 255, 0);");
//    ui->write->setStyleSheet("background-color: rgb(255, 255, 255);");

    mLogicAddr = LogicAddr;

    ui->label_addr->setText(QString::number(pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr,10).append("#"));

    QString tNameStr = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ProtoName;
    ui->label_type->setText(tNameStr);
    ui->label_com->setText("COM"+QString::number(pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].com_id + 1));
    /**************************/
    QStringList header;
    header<<"名称"<<"数值"<<"单位及说明";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    INT32S tRowCnt = 0;
    tRowCnt = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[0].DisplayTotal;
    ui->tableWidget->setRowCount(tRowCnt);
    //qDebug()<<__FUNCTION__<<__LINE__<<tRowCnt;
    for(int i = 0;i < tRowCnt; i++)
    {
       ui->tableWidget->setItem(i,0,new QTableWidgetItem(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[0].Point[i].name));
       ui->tableWidget->setItem(i,2,new QTableWidgetItem(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[0].Point[i].comment));
    }

    ui->tableWidget->resizeColumnToContents(0);
}

void Constant::keyPressEvent(QKeyEvent *e)
{
   // qDebug() << __FUNCTION__ << __LINE__ << e->isAutoRepeat() << "\n";
    if(!e->isAutoRepeat())
    {
        switch(e->key())
        {
            case Qt::Key_F1:key_f1_press();break;
            case Qt::Key_F2:key_f2_press();break;
            case Qt::Key_F3:key_f3_press();break;
            case Qt::Key_Return:key_return_press();break;
            case Qt::Key_Escape:key_escape_press();break;
        }
    }
}

void Constant::key_f1_press()
{
    selectSETS++;
    if(selectSETS > 1)
    {
        selectSETS = 0;
    }
    updateSETSUI();
}

void Constant::key_f2_press()
{
     int col = 1;
     int row = ui->tableWidget->currentRow();
     qDebug()<<col<<row<<__FUNCTION__<<__LINE__;
     if(row >= 0)
     {
            qDebug()<<col<<row<<__FUNCTION__<<__LINE__;
            QTableWidgetItem *item = ui->tableWidget->item(row,col);
            qDebug()<<col<<row<<__FUNCTION__<<__LINE__<<item;
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            qDebug()<<col<<row<<__FUNCTION__<<__LINE__;
            ui->tableWidget->editItem(item);
     }
}

void Constant::key_f3_press()
{
    switch(selectSETS)     //dev select or writr/resd
    {
    case 0:
        selectDevProcess();   //pump group select
        switch(selectDevIndex)
        {
        case 0:
            currentLogicAddr = 0;
            break;
        case 1:
            currentLogicAddr = 1;
            break;
        case 2:
            currentLogicAddr = 16;
            break;
        case 3:
            currentLogicAddr = 17;
            break;
        case 4:
            currentLogicAddr = 32;
            break;
        }
        DisplayConstantInfo(currentLogicAddr,GROUP_INDEX_ONE);
        mEnterFlag = 1;
        mConstantG1ReadStep = READ_CONSTANT_UNDO;
        mConstantG2ReadStep = READ_CONSTANT_UNDO;
        if(pDeviceDataBase->DeviceValue[currentLogicAddr].commu.status == COMM_STATUS_ON)
            ReadConstant(currentLogicAddr,GROUP_INDEX_ONE);
        break;
    case 1:
        selectRWProcess();    //write or read
        break;
    }
}

void Constant::selectDevProcess()
{
    selectDevIndex++;
    if(selectDevIndex > 5)
        selectDevIndex = 0;
    ui->tableWidget->clear();
    updateDevUI();
}
void Constant::selectRWProcess()
{
    mSelectCode++;
    if(mSelectCode > CONSTANT_MAX_SELECT)
        mSelectCode = FLAG_READ;
    switch(mSelectCode)
    {
    case FLAG_READ:
            ui->label_write->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_read->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
    case FLAG_WRITE:
            ui->label_read->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_write->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
    }
    updateRWUI();
}

void Constant::key_return_press()
{
    INT8U tConstantStep = 0;
    QPushButton *buttonY = NULL;
    QPushButton *buttonN = NULL;
    QMessageBox *pMsgbox = NULL;

    if(pDeviceDataBase->DeviceValue[mLogicAddr].commu.status == COMM_STATUS_ON)
    {
        switch(mSelectCode)
        {
        case FLAG_READ:
            pMsgbox = new QMessageBox(QMessageBox::Information, tr("提示"), tr("确定是否读定值"));
            buttonY = pMsgbox->addButton(tr("是"), QMessageBox::AcceptRole);
            buttonN = pMsgbox->addButton(tr("否"), QMessageBox::RejectRole);
            buttonN->setFocus();
            pMsgbox->exec();

            if (pMsgbox->clickedButton() == buttonY)
            {
                ReadConstant(mLogicAddr,mGroupIndex);
            }
            break;

        case FLAG_WRITE:
            if(mGroupIndex == 0)
                tConstantStep = mConstantG1ReadStep;
            else
                tConstantStep = mConstantG2ReadStep;

            if(tConstantStep == READ_CONSTANT_UNDO)
            {
                 QMessageBox *m_box = new QMessageBox(QMessageBox::Information,QString(tr("通知")),QString(tr("先读取定值！")));
                 QTimer::singleShot(2000,m_box,SLOT(accept()));
                 m_box->exec();
            }
            else
            {
//               if((pDeviceDataBase->UsrLoginInfo.usrlevel == LVLENGINEER)||(pDeviceDataBase->UsrLoginInfo.usrlevel == LVLMANAGE))
                pMsgbox = new QMessageBox(QMessageBox::Information, tr("提示"), tr("确定是否写定值"));
                buttonY = pMsgbox->addButton(tr("是"), QMessageBox::AcceptRole);
                buttonN = pMsgbox->addButton(tr("否"), QMessageBox::RejectRole);
                buttonN->setFocus();
                pMsgbox->exec();

                if (pMsgbox->clickedButton() == buttonY)
                {
                    WriteConstant(mLogicAddr,mGroupIndex); // 写定值前必须读对应定值
                }
//                 if(1)
//                 {
//                     WriteConstant(mLogicAddr,mGroupIndex); // 写定值前必须读对应定值
//                 }
//                 else
//                 {
//                     QMessageBox *m_box = new QMessageBox(QMessageBox::Information,QString(tr("通知")),QString(tr("操作员账户无法设置定值")));
//                     QTimer::singleShot(2000,m_box,SLOT(accept()));
//                     m_box->exec();
//                 }
            }
            break;
        }
     }
}
void Constant::key_escape_press()
{
    mConstantG1ReadStep = READ_CONSTANT_UNDO;
    mConstantG2ReadStep = READ_CONSTANT_UNDO;
    mEnterFlag = 0;

    ui->tableWidget->clear();
//    switch(mSelectCode)
//    {
//    case 1:
//        ui->read->setStyleSheet("");
//        break;
//    case 2:
//        ui->write->setStyleSheet("");
//        break;
//    }
    this->close();
}

void Constant::ReadConstant(INT32S LogicAddr,INT32S GroupIndex)
{
    INT32S tComPort    = LogicAddr/16;
    INT32S tLogicIndex = LogicAddr%16;

    QUEUE_MSG ConstantReadMsg;
    memset(&ConstantReadMsg,0x00,sizeof(QUEUE_MSG));
    if(tComPort >= 0 && tComPort <= 5) //
    {
        ConstantReadMsg.MsgType    = FRAME_CONSTANT_READ;
        ConstantReadMsg.Source     = SOURCE_HMI;
        ConstantReadMsg.LogicAddr  = LogicAddr;
        ConstantReadMsg.PhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;
        ConstantReadMsg.ComPort    = tComPort;
        ConstantReadMsg.LogicIndex = tLogicIndex;
        ConstantReadMsg.GroupIndex = GroupIndex;

        pMsgQMutex[tComPort]->lock();
        pMsgQueue[tComPort]->enqueue(ConstantReadMsg);
        pMsgQMutex[tComPort]->unlock();
    }
}

void Constant::WriteConstant(INT32S LogicAddr, INT32S GroupIndex)
{
    INT32S tComPort    = LogicAddr/16;
    INT32S tLogicIndex = LogicAddr%16;

    INT32S tIsChanged = 0;

    GetConstantFromUI(LogicAddr,GroupIndex);

    tIsChanged = IsConstantChanged(LogicAddr,GroupIndex);

    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;
    INT8U tTotal = 0;

    mWriteConstantFunCode = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[mGroupIndex].funcode;

    INT8U tValueIndex = 0;
    INT8U tSectionIndex = GroupIndex;
    if((tSectionIndex >= 0)&&(tSectionIndex <= 1))
        tTotal = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tSectionIndex].DisplayTotal;

    if(tSectionIndex == 0)
        tValueIndex = INFO_CONSTANT_1;
    else
        tValueIndex = INFO_CONSTANT_2;

    qDebug()<<__FUNCTION__<<__LINE__<<tIsChanged<<tTotal;

    if(tIsChanged)
    {
        QUEUE_MSG ConstantReadMsg;
        memset(&ConstantReadMsg,0x00,sizeof(QUEUE_MSG));

        if(tComPort >= 0 && tComPort <= 5) //
        {
            if(mWriteConstantFunCode == WriteHoldReg)
            {
                ConstantReadMsg.MsgType    = FRAME_CONSTANT_SINGLE_WRITE;
                ConstantReadMsg.Source     = SOURCE_HMI;
                ConstantReadMsg.LogicAddr  = LogicAddr;
                ConstantReadMsg.PhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;
                ConstantReadMsg.ComPort    = tComPort;
                ConstantReadMsg.LogicIndex = tLogicIndex;
                ConstantReadMsg.GroupIndex = GroupIndex;
                ConstantReadMsg.Length     = 1; //显示个数
                ConstantReadMsg.ConstantValuePointIndex = mChangedPointIndex;
                ConstantReadMsg.ValueData[0] = mCurrentConstantArray[mChangedPointIndex];

                pMsgQMutex[tComPort]->lock();
                pMsgQueue[tComPort]->enqueue(ConstantReadMsg);
                pMsgQMutex[tComPort]->unlock();
            }
            else if(mWriteConstantFunCode == WriteMoreHoldReg)// WriteMoreHoldReg
            {
                ConstantReadMsg.MsgType    = FRAME_CONSTANT_WRITE;
                ConstantReadMsg.Source     = SOURCE_HMI;
                ConstantReadMsg.LogicAddr  = LogicAddr;
                ConstantReadMsg.PhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr;
                ConstantReadMsg.ComPort    = tComPort;
                ConstantReadMsg.LogicIndex = tLogicIndex;
                ConstantReadMsg.GroupIndex = GroupIndex;
                ConstantReadMsg.Length     = tTotal; //显示个数
                memcpy(ConstantReadMsg.ValueData,mCurrentConstantArray,128*4);

                pMsgQMutex[tComPort]->lock();
                pMsgQueue[tComPort]->enqueue(ConstantReadMsg);
                pMsgQMutex[tComPort]->unlock();
            }
        }
    }
}

void Constant::UpdateDevConstantSlot(INT8U LogicAddr, INT8U FrameType, INT8U Source, INT8U GroupIndex)
{
    INT8U tValueIndex = 0;
    QString tDisplayStr;

    if(GroupIndex == 0)
        tValueIndex = INFO_CONSTANT_1;
    else
        tValueIndex = INFO_CONSTANT_2;

    if(Source == SOURCE_TCP)
        tDisplayStr = tr("远程");

    if(FrameType == FRAME_CONSTANT_READ)
    {
        switch(pDeviceDataBase->DeviceValue[LogicAddr].Result)
        {
        case RESULT_SUCCESS:
             if(Source == SOURCE_HMI)
             {
                UpdateConstant(LogicAddr,FrameType,GroupIndex);
                if(GroupIndex == 0)
                    mConstantG1ReadStep = READ_CONSTANT_DONE;
                else
                    mConstantG2ReadStep = READ_CONSTANT_DONE;
             }
             tDisplayStr += tr("读取定值成功");
             break;
        case RESULT_SEND_ERROR:
             tDisplayStr += tr("读取定值发送失败");
             break;
        case RESULT_RECV_TIMEOUT:
             tDisplayStr += tr("读取定值超时");
             break;
        case RESULT_CRC_ERROR:
             tDisplayStr += tr("读取定值CRC校验失败");
             break;
        case RESULT_FAILED:
             tDisplayStr += tr("读取定值失败");
             break;
        }

        QMessageBox *m_box = new QMessageBox(QMessageBox::Information,QString(tr("提示")),QString(tDisplayStr));
        QTimer::singleShot(2000,m_box,SLOT(accept()));
        m_box->exec();
    }
    else if((FrameType == FRAME_CONSTANT_WRITE)||(FrameType == FRAME_CONSTANT_SINGLE_WRITE))
    {
        QString tDisplayStr;
        if(FrameType == FRAME_CONSTANT_SINGLE_WRITE)
           tDisplayStr = tr("写单个定值");
        else
           tDisplayStr = tr("写定值");

        switch(pDeviceDataBase->DeviceValue[LogicAddr].Result)
        {
        case RESULT_SUCCESS:
             ReadConstant(LogicAddr,GroupIndex);  // 写完定值->再次读取定值->最终刷新界面数据
             tDisplayStr += tr("成功");
             break;
        case RESULT_SEND_ERROR:
             tDisplayStr += tr("发送失败");
             break;
        case RESULT_RECV_TIMEOUT:
             tDisplayStr += tr("超时");
             break;
        case RESULT_CRC_ERROR:
             tDisplayStr += tr("CRC校验失败");
             break;
        case RESULT_FAILED:
             tDisplayStr += tr("失败");
             break;
        }

        QMessageBox *m_box = new QMessageBox(QMessageBox::Information,QString(tr("提示")),QString(tDisplayStr));
        QTimer::singleShot(2000,m_box,SLOT(accept()));
        m_box->exec();
    }
}

void Constant::UpdateConstant(INT8U LogicAddr, INT8U FrameType, INT8U GroupIndex)//GroupIndex 0=switch;1-6=measure
{
    INT8U tCol = 1;
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

//    qDebug()<<__FUNCTION__<<__LINE__<<"TOTAL="<<total;

    FP32 tRatio = 1;
    for(INT32S i = 0; i < tTotal; i++)
    {   
        tRatio = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tSectionIndex].Point[i].Ratio;

        tRatio = tRatio != 0 ? tRatio : 1;
        if(tRatio == 1)
        {
            ui->tableWidget->setItem(i,tRatio,new QTableWidgetItem(QString::number(pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i])));
        }
        else
        {
            float tDisplayValue = (float)pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i]/tRatio;
            ui->tableWidget->setItem(i,tCol,new QTableWidgetItem(QString::number(tDisplayValue ,'f',2)));
        }
    }
}
void Constant::GetConstantFromUI(INT8U LogicAddr, INT8U GroupIndex)
{
    INT8U tCol = 1;
    bool  tOK = 0;
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8U tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;
    INT8U tTotal = 0;

    INT8U tValueIndex = 0;
    INT8U tSectionIndex = GroupIndex;
   // if((tSectionIndex >= 0)&&(tSectionIndex <= 1))
    if(tSectionIndex <= 1)
        tTotal = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tSectionIndex].DisplayTotal;

    if(tSectionIndex == 0)
        tValueIndex = INFO_CONSTANT_1;
    else
        tValueIndex = INFO_CONSTANT_2;

    FP32 tRatio = 1;

    memset(mCurrentConstantArray,0x00,128*4);

    for(INT32S i = 0; i < tTotal; i++)
    {
        tRatio = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ConstantSection[tSectionIndex].Point[i].Ratio;

        tRatio = tRatio != 0 ? tRatio : 1;

        float tDisplayValue = ui->tableWidget->item(i,tCol)->text().toFloat(&tOK)*tRatio;

        if(tOK)
        {
            mCurrentConstantArray[i] = (INT32U)tDisplayValue;
        }
        qDebug()<<__FUNCTION__<<__LINE__<<mCurrentConstantArray[i];
    }
}

INT32S Constant::IsConstantChanged(INT8U LogicAddr, INT8U GroupIndex)
{
    INT32S tIsChanged = 0;

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


    for(INT32S i=0;i<MAX_VAL_NUM;i++)
    {
        if(pDeviceDataBase->DeviceValue[LogicAddr].value[tValueIndex].ValueData[i] != mCurrentConstantArray[i])
        {
            tIsChanged = 1;
            mChangedPointIndex = i;
            break;
        }
    }
    return tIsChanged;
}
void Constant::updateSETSUI()
{
    switch(selectSETS)
    {
        case 0:
            ui->label_00->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_01->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_02->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_03->setStyleSheet("background-color: rgb(255, 255, 255);");

            ui->label_20->setStyleSheet("background-color: rgb(100, 100, 100);");
            ui->label_21->setStyleSheet("background-color: rgb(100, 100, 100);");
            ui->label_22->setStyleSheet("background-color: rgb(100, 100, 100);");
            ui->label_23->setStyleSheet("background-color: rgb(100, 100, 100);");
            break;
        case 1:
            ui->label_20->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_21->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_22->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_23->setStyleSheet("background-color: rgb(255, 255, 255);");

            ui->label_00->setStyleSheet("background-color: rgb(100, 100, 100);");
            ui->label_01->setStyleSheet("background-color: rgb(100, 100, 100);");
            ui->label_02->setStyleSheet("background-color: rgb(100, 100, 100);");
            ui->label_03->setStyleSheet("background-color: rgb(100, 100, 100);");
            break;
    }
}
void Constant::updateDevUI()
{
        switch(selectDevIndex)
        {
        case 0:
            ui->label_pumpgroup1->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumproom->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 1:
            ui->label_pumpgroup2->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumpgroup1->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 2:
            ui->label_pumpgroup3->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumpgroup2->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 3:
            ui->label_pumpgroup4->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumpgroup3->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 4:
            ui->label_pumproom->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumpgroup4->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        }
}
void Constant::updateRWUI()
{
    switch(selectRW)
    {
    case FLAG_READ:
            ui->label_write->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_read->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
    case FLAG_WRITE:
            ui->label_read->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_write->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
    }
}

