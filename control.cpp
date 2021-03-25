#include "control.h"
#include "ui_control.h"
#include <qmessagebox.h>
#include <QDebug>

Control::Control(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Control)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

Control::~Control()
{
    delete ui;
}

void Control::EnterProcess()
{
//    mLogicAddr = LogicAddr;
//    DisplayControlUI(LogicAddr);
    ui->label_Jointopera->setStyleSheet("background-color: rgb(255, 255, 255);");
    ui->label_feedopera->setStyleSheet("background-color: rgb(255, 255, 255);");
    ui->label_pumpopera->setStyleSheet("background-color: rgb(255, 0, 0);");
   operatypedisplay();
   mLogicAddr = 0;
}

void Control::changeEvent(QEvent *e)
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
void Control::DisplayControlUI(INT32S LogicAddr)
{
//    ui->label_addr->setText(QString::number(pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].phy_addr,10).append("#"));

    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8S tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;

    QString tNameStr = QLatin1String(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].ProtoName);
//    ui->label_type->setText(tNameStr);
//    ui->label_com->setText("COM"+QString::number(pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].com_id));
 /**************************/

    mDevStatusIndex = 0;
    mDevStatus = 0;
    mErrorStatusIndex = 0;
    mErrorTotal = 0;
    mErrorStatus = 0;
//    mPowerStampStatus=0;
//    mPowerStampIndex=0;


    if(pDeviceDataBase->DeviceValue[LogicAddr].isIntern)
    {
        mDevStatusIndex = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].SwitchStatusParseInternSection.MapIndex;
        mDevStatus = pDeviceDataBase->DeviceValue[mLogicAddr].value[INFO_MEASURE_INTER_1].ValueData[mDevStatusIndex];
        mErrorStatusIndex = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].FaultParseInternSection.MapIndex[0];  // only one
        mErrorStatus = pDeviceDataBase->DeviceValue[mLogicAddr].value[INFO_MEASURE_INTER_1].ValueData[mErrorStatusIndex];
    }
    else
    {
        mDevStatusIndex = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].SwitchStatusParseSection.MapIndex;
        mDevStatus = pDeviceDataBase->DeviceValue[mLogicAddr].value[INFO_MEASURE_1].ValueData[mDevStatusIndex];
        mErrorTotal = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].FaultParseSection.Total;
        for(int i =0;i<mErrorTotal;i++)
        {
            mErrorStatusIndex = pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].FaultParseSection.MapIndex[i];
            mErrorStatus += pDeviceDataBase->DeviceValue[mLogicAddr].value[INFO_SWITCH].ValueData[mErrorStatusIndex];
        }
    }
    qDebug()<<__FUNCTION__<<__LINE__<<pDeviceDataBase->DeviceValue[LogicAddr].isIntern<<mDevStatusIndex<<mDevStatus<<mErrorStatusIndex<<mErrorStatus;

//    if(mErrorStatus)
//    {
//        ui->label_stat1->setText(tr("故障状态"));
//    }
//    else
//    {
//        if(mDevStatus == RELAY_STATUS_ON)
//        {
//            ui->label_stat1->setText(tr("合闸运行"));
//            ui->label_stat1->setStyleSheet("background-color: rgb(255, 0, 0);border-radius:40px;border-color: rgb(0, 0, 0);border-width: 1px;"); //background-color: rgb(0, 255, 0);border-radius:40px;border-color: rgb(0, 0, 0);border-width: 1px;
//        }
//        else
//        {
//            ui->label_stat1->setText(tr("分闸待机"));
//            ui->label_stat1->setStyleSheet("background-color: rgb(0, 255, 0);border-radius:40px;border-color: rgb(0, 0, 0);border-width: 1px;");
//        }
//    }

    mSelectCode = CONTROL_RELAY_ON;
//    ui->relay_on->setStyleSheet("background-color: rgb(0, 255, 0);");
}
void Control::keyPressEvent(QKeyEvent *e)
{
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
void Control::key_f1_press()
{
    selecttype++;
    if(selecttype > 2)
    {
        selecttype=0;
    }
    switch (selecttype) {
    case 0:
        ui->label_Jointopera->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumpopera->setStyleSheet("background-color: rgb(255, 0, 0);");
        ui->label_feedopera->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 1:
        ui->label_Jointopera->setStyleSheet("background-color: rgb(255, 0, 0);");
        ui->label_pumpopera->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_feedopera->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 2:
        ui->label_Jointopera->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumpopera->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_feedopera->setStyleSheet("background-color: rgb(255, 0, 0);");
        selectDevfeeddisplay();
        selectOptfeeddisplay();
        break;
    default:
        break;
    }
    operatypedisplay();
}
void Control::key_f2_press()
{
    selectOpt =0;
    switch (selecttype) {
    case 0:
        selectDev++;
        if(selectDev > 4)
        {
            selectDev = 0;
        }
        display1();
        display2();
        selectOpt =0;
        display3();
        break;
    case 1:
        selectswitch++;
        if(selectswitch > 1)
        {
            selectswitch=0;
        }
        switch (selectswitch) {
        case 0:
            ui->label_One_switch_Tow->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_One_switch_Three->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 1:
            ui->label_One_switch_Tow->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_One_switch_Three->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        }
        break;
    case 2:
        selectswitch = 0;
        feedtest++;
        if(feedtest > 2)
            feedtest = 0;
        selectDevfeeddisplay();
        selectOpt =0;
        selectOptfeeddisplay();
        break;
    default:
        break;
    }
}
void Control::feedsupdate()
{
    ui->label_feedIA->setText(QString::number(pDeviceDataBase->DeviceValue[33].value[INFO_MEASURE_1].ValueData[3]*0.1)+tr("A"));
    ui->label_feedIB->setText(QString::number(pDeviceDataBase->DeviceValue[33].value[INFO_MEASURE_1].ValueData[4]*0.1)+tr("A"));
    ui->label_feedIC->setText(QString::number(pDeviceDataBase->DeviceValue[33].value[INFO_MEASURE_1].ValueData[5]*0.1)+tr("A"));
    ui->label_feedUs->setText(QString::number(pDeviceDataBase->DeviceValue[33].value[INFO_MEASURE_1].ValueData[1]*0.1)+tr("V"));
    if(pDeviceDataBase->DeviceValue[33].value[INFO_MEASURE_1].ValueData[0])
    {
       ui->label_feedStatue->setText(tr("合闸运行"));
    }
    else
    {
       ui->label_feedStatue->setText(tr("分闸待机"));
    }
}
void Control::key_f3_press()
{    
    if(selecttype==0)
    {
        if(selectDev==4)
        {
            selectOpt++;
            if(selectOpt > 7)
            {
                selectOpt = 0;
            }
        }
        else
        {
            selectOpt++;
            if(selectOpt > 3)
            {
                selectOpt = 0;
            }
        }
        display3();
    }
    if(selecttype==2)
    {
        selectOpt++;
        if(selectOpt > 2)
        {
            selectOpt = 0;
        }
        selectOptfeeddisplay();
    }
}
void Control::key_return_press()
{
    if(selecttype==1)
    {
        switch (selectswitch) {
        case 0:
            if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[12]==1 &&
                    pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12]==0)
            {
                pumpswitchnum=1;
                tipstr = tr("是否由1#泵组切换到2#");
                pumpswitchconfim(pumpswitchnum,tipstr);
            }
            if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[12] ==0 &&
                    pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12]==1)
            {
               pumpswitchnum = 2;
               tipstr = tr("是否由2#泵组切换到1#");
               pumpswitchconfim(pumpswitchnum,tipstr);
            }
            break;
        case 1:
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12] ==1 &&
                    pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12]==0)
            {
                pumpswitchnum=3;
                tipstr = tr("是否由3#泵组切换到4#");
                pumpswitchconfim(pumpswitchnum,tipstr);
            }
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12]==0 &&
                    pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12]==1)
            {
               pumpswitchnum = 4;
               tipstr = tr("是否由4#泵组切换到3#");
               pumpswitchconfim(pumpswitchnum,tipstr);
            }
            break;
        default:
            break;
        }
    }
     if(selecttype==0)
    {
        control_step = FRAME_COIL_OPERATION;
        switch(selectDev)
        {
        case 0:
            mLogicAddr = 0;
            if(pDeviceDataBase->Mainsumpinterlock)
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12])
                {
                    QMessageBox::information(this,tr("提示"),tr("1#低压瓦斯泵组闭锁中"),QMessageBox::Ok);
                    return;
                }
                else
                {
                    handleOperation(mLogicAddr,control_step);
                }
            }
            else
            {
                handleOperation(mLogicAddr,control_step);
            }
            break;
        case 1:
            mLogicAddr = 1;
            if(pDeviceDataBase->Mainsumpinterlock)
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[12])
                {
                    QMessageBox::information(this,tr("提示"),tr("2#低压瓦斯泵组闭锁中"),QMessageBox::Ok);
                    return;
                }
                else
                {
                    handleOperation(mLogicAddr,control_step);
                }
            }
            else
            {
                handleOperation(mLogicAddr,control_step);
            }
            break;
        case 2:
            mLogicAddr = 16;
            if(pDeviceDataBase->AuxiSumpinterlock)
            {
                if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12])
                {
                    QMessageBox::information(this,tr("提示"),tr("1#高压瓦斯泵组闭锁中"),QMessageBox::Ok);
                    return;
                }
                else
                {
                    handleOperation(mLogicAddr,control_step);
                }
            }
            else
            {
                handleOperation(mLogicAddr,control_step);
            }
            break;
        case 3:
            mLogicAddr = 17;
            if(pDeviceDataBase->AuxiSumpinterlock)
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12])
                {
                    QMessageBox::information(this,tr("提示"),tr("2#高压瓦斯泵组闭锁中"),QMessageBox::Ok);
                    return;
                }
                else
                {
                    handleOperation(mLogicAddr,control_step);
                }
            }
            else
            {
                handleOperation(mLogicAddr,control_step);
            }
            break;
        case 4:
            mLogicAddr = 32;
            handleOperation(mLogicAddr,control_step);
            break;
        }
    }
     if(selecttype==2)
     {
         control_step = FRAME_COIL_OPERATION;
         mLogicAddr = 33;
         handleOperation(mLogicAddr,control_step);
     }
}
void Control::handleOperation(int logicaddr, int step)
{

    if(selecttype==0)
    {
        if(selectDev==4)
        {
            switch(selectOpt)
            {
            case 0:
                optaddr = 16;
                tipstr = tr("是否一键启动1#水泵");
                break;
            case 1:
                optaddr = 17;
                tipstr = tr("是否一键关闭1#水泵");
                break;
            case 2:
                optaddr = 18;
                tipstr = tr("是否一键启动2#水泵");
                break;
            case 3:
                optaddr = 19;
                tipstr = tr("是否一键关闭2#水泵");
                break;
            case 4:
                optaddr = 24;
                tipstr = tr("是否一键启动3#水泵");
                break;
            case 5:
                optaddr = 25;
                tipstr = tr("是否一键关闭3#水泵");
                break;
            case 6:
                optaddr = 26;
                tipstr = tr("是否一键启动4#水泵");
                break;
            case 7:
                optaddr = 27;
                tipstr = tr("是否一键关闭4#水泵");
                break;
            }
        }
        else
        {
            switch(selectOpt)
            {
            case 0:
                optaddr = 18;
                tipstr = tr("是否一键启动");
                break;
            case 1:
                optaddr = 19;
                tipstr = tr("是否一键关闭");
                break;
            case 2:
                if(pDeviceDataBase->DeviceValue[logicaddr].value[INFO_SWITCH].ValueData[15])
                {
                    optaddr = 22;
                    tipstr = tr("是否退出检修");
                }
                else
                {
                    optaddr = 20;
                    tipstr = tr("是否检修");
                }
                break;
            case 3:
                if(pDeviceDataBase->DeviceValue[logicaddr].value[INFO_SWITCH].ValueData[14])
                {
                    optaddr = 23;
                    tipstr = tr("是否退出急停");
                }
                else
                {
                    optaddr = 21;
                    tipstr = tr("是否急停");
                }
                break;
            }
        }
    }
    if(selecttype == 2)
    {
        switch(selectOpt)
        {
        case 0:
            optaddr = 6;
            tipstr = tr("是否分闸");
            break;
        case 1:
            optaddr = 5;
            tipstr = tr("是否合闸");
            break;
         case 2:
            optaddr = 4;
            tipstr = tr("是否复归");
            break;
         break;
        }
    }
    if(QMessageBox::Yes == QMessageBox::information(this,tr("警告"),tipstr,QMessageBox::Yes,QMessageBox::No))
    {
        HandleControl(optaddr,logicaddr);
    }
}
void Control::pumpswitchconfim(int switchnum, QString tip)
{
    if(QMessageBox::Yes == QMessageBox::information(this,tr("警告"),tip,QMessageBox::Yes,QMessageBox::No))
    {
        pDeviceDataBase->OneKeySwith = switchnum;
        qDebug()<<"pDeviceDataBase->OneKeySwith "<<pDeviceDataBase->OneKeySwith ;
        pumpswitchdeal();
    }
}
void Control::pumpswitchdeal()
{
    switch (pDeviceDataBase->OneKeySwith) {
    case 1:
         HandleControl(19,0);
        break;
    case 2:
        HandleControl(19,1);
        break;
    case 3:
        HandleControl(19,16);
        break;
    case 4:
        HandleControl(19,17);
        break;
    default:
        pDeviceDataBase->OneKeySwith =0;
        break;
    }
}
void Control::UpdateDevStatusSlot(INT8U LogicAddr,INT8U FrameType,INT8U GroupIndex)
{
    if(LogicAddr < 33) //1#:0,2#:1  3#:16,4#:17
    {
//        ui->label_abc->setText(QString::number(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_1].ValueData[10]));
//     qDebug()<<__FUNCTION__<<__LINE__<<pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_2].ValueData[22]
//                <<pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12]
//                <<pDeviceDataBase->OneKeySwith<<LogicAddr<<"qqqqqqqqqqqqqqq";
        if(pDeviceDataBase->OneKeySwith != 0)
        {
            switch (pDeviceDataBase->OneKeySwith) {
            case 1:
                if(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_2].ValueData[22] ==170 &&
                        pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12] ==0)
                {
                    HandleControl(18,1);
                    pDeviceDataBase->OneKeySwith = 0;
                }
                break;
            case 2:
                if(pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_2].ValueData[22] ==170 &&
                        pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[12] ==0)
                {
                    HandleControl(18,0);
                    pDeviceDataBase->OneKeySwith = 0;
                }
                break;
            case 3:
                if(pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_2].ValueData[22] ==170 &&
                        pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12] ==0)
                {
                    HandleControl(18,17);
                    pDeviceDataBase->OneKeySwith = 0;
                }
                break;
            case 4:
                if(pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_2].ValueData[22] ==170 &&
                        pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12] ==0)
                {
                    HandleControl(18,16);
                    pDeviceDataBase->OneKeySwith = 0;
                }
                break;
            default:
                break;
            }
        }
    }
    if(selecttype==2 and LogicAddr==33)
    {
        feedsupdate();
    }
}
void Control::key_escape_press()
{
    selecttype    = 0;
    selectswitch  = 0;
    selectDev     = 0;
    selectOpt     = 0;
    logic_Addr    = 0;
    rewaterpump = 0;
    this->close();
}
void Control::HandleReset()
{
//    if(QMessageBox::Yes == QMessageBox::information(this,tr("警告"),tr("是否要复归?"),QMessageBox::Yes,QMessageBox::No))
//    {
//        HandleControl(CONTROL_RESET);
//    }
}
void Control::HandleRelayOn()
{
//     if(QMessageBox::Yes == QMessageBox::information(this,tr("警告"),tr("是否合闸执行?"),QMessageBox::Yes,QMessageBox::No))
//     {
//         HandleControl(CONTROL_RELAY_ON);
//     }

}
void Control::HandleRelayOff()
{
//     if(QMessageBox::Yes == QMessageBox::information(this,tr("警告"),tr("是否分闸执行?"),QMessageBox::Yes,QMessageBox::No))
//     {
//        HandleControl(CONTROL_RELAY_OFF);
//     }
}

void Control::HandleTest()
{
//    if(QMessageBox::Yes == QMessageBox::information(this,tr("警告"),tr("是否测试?"),QMessageBox::Yes,QMessageBox::No))
//    {
//       HandleControl(CONTROL_TEST);
//    }
}

void Control::HandleControl(INT8U Channel,int logicaddr)
{
    mLogicAddr = logicaddr;
    INT32S tComPort    = mLogicAddr/16;
    INT32S tLogicIndex = mLogicAddr%16;

    QUEUE_MSG ControlMsg;
    memset(&ControlMsg,0x00,sizeof(QUEUE_MSG));
    if(tComPort >= 0 && tComPort <= 5) //
    {
        switch(Channel)
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

        ControlMsg.Source     = SOURCE_HMI;
        ControlMsg.LogicAddr  = mLogicAddr;
        ControlMsg.PhyAddr    = pDeviceDataBase->SysCfgData.DevInfo[mLogicAddr].phy_addr;
        ControlMsg.ComPort    = tComPort;
        ControlMsg.LogicIndex = tLogicIndex;
        ControlMsg.GroupIndex = Channel; //

        pMsgQueueMutex[tComPort]->lock();
        pMsgQueue[tComPort]->enqueue(ControlMsg);
        pMsgQueueMutex[tComPort]->unlock();
    }
}

void Control::UpdateControlCoilSlot(INT8U LogicAddr,INT8U FrameType,INT8U Source,INT8U ChannelIndex)
{
    INT8U tTypeIndex = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeIndex;
    INT8S tVerIndex  = pDeviceDataBase->SysCfgData.DevInfo[LogicAddr].rtu_typeVer;
    QString tDisplayStr;

    if(ChannelIndex < MAX_CHANNEL_NUM)
    {
        if(Source == SOURCE_TCP)
        {
            tDisplayStr = tr("远程");
        }

        switch(FrameType)
        {
        case FRAME_COIL_OPERATION: //开关量控制
             tDisplayStr += tr("开关量操作命令:") + QString(pDeviceDataBase->SysCfgData.ProtoCfgData[tTypeIndex][tVerIndex].SwitchControlSection.Point[ChannelIndex].Desc);
             break;
        case FRAME_RELAYON:
             tDisplayStr += tr("合闸命令");
             break;
        case FRAME_RELAYOFF:
             tDisplayStr += tr("分闸命令");
             break;
        case FRAME_TEST1:
             tDisplayStr += tr("测试命令");
             break;
        case FRAME_RESET:
             tDisplayStr += tr("重置命令");
             break;
        }

        switch(pDeviceDataBase->DeviceValue[LogicAddr].Result)
        {
        case RESULT_SUCCESS:
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
void Control::operatypedisplay()
{
    switch (selecttype)
    {
    case 0:
        ui->widget->setVisible(true);
        ui->widget_2->setVisible(false);
        ui->widget_3->setVisible(false);
        ui->label_waterpumpA->setVisible(false);
        ui->label_waterpumpB->setVisible(false);
        selectDev =0;
        selectswitch =0;
        display1();
        display2();
        display3();
        break;
    case 1:
        ui->widget->setVisible(false);
        ui->widget_2->setVisible(true);
        ui->widget_3->setVisible(false);
        selectDev =0;
        selectOpt =0;
        selectswitch =0;
        ui->label_One_switch_Tow->setStyleSheet("background-color: rgb(0, 255, 0);");
        ui->label_One_switch_Three->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 2:
        ui->widget->setVisible(false);
        ui->widget_3->setVisible(true);
        ui->widget_2->setVisible(false);
        feedtest = 0;
        selectDevfeeddisplay();
        selectOptfeeddisplay();
        break;
    default:
        break;
    }
}
void Control::selectDevfeeddisplay()
{
    switch(feedtest)
    {
    case 0:
        ui->label_feed2->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_feed3->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_feed1->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    case 1:
        ui->label_feed1->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_feed3->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_feed2->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    case 2:
        ui->label_feed1->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_feed2->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_feed3->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    }
}
void Control::selectOptfeeddisplay()
{
    switch(selectOpt)
    {
    case 0:
        ui->label_SwitchOFF->setStyleSheet("background-color: rgb(255, 0, 0);");
        ui->label_SwitchON->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_Reset->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 1:
        ui->label_SwitchOFF->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_SwitchON->setStyleSheet("background-color: rgb(255, 0, 0);");
        ui->label_Reset->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 2:
        ui->label_SwitchOFF->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_SwitchON->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_Reset->setStyleSheet("background-color: rgb(255, 0, 0);");
        break;
    }
}
void Control::display1()
{
    if(selectDev==4)
    {
        ui->label_OverHaul->setVisible(false);
        ui->label_CarshStop->setVisible(false);
        ui->label_OneKeyClose1->setVisible(true);
        ui->label_OneKeyOpen1->setVisible(true);
        ui->label_waterpumpA->setVisible(true);
        ui->label_waterpumpB->setVisible(true);
        ui->label_waterpumpC->setVisible(true);
        ui->label_waterpumpD->setVisible(true);
        ui->label_OneKeyClose2->setVisible(true);
        ui->label_OneKeyOpen2->setVisible(true);
        ui->label_OneKeyClose3->setVisible(true);
        ui->label_OneKeyOpen3->setVisible(true);
    }
    else
    {
        ui->label_OverHaul->setVisible(true);
        ui->label_CarshStop->setVisible(true);
        ui->label_OneKeyClose1->setVisible(false);
        ui->label_OneKeyOpen1->setVisible(false);
        ui->label_waterpumpA->setVisible(false);
        ui->label_waterpumpB->setVisible(false);
        ui->label_waterpumpC->setVisible(false);
        ui->label_waterpumpD->setVisible(false);
        ui->label_OneKeyClose2->setVisible(false);
        ui->label_OneKeyOpen2->setVisible(false);
        ui->label_OneKeyClose3->setVisible(false);
        ui->label_OneKeyOpen3->setVisible(false);
    }
}
void Control::display2()
{
    switch(selectDev)
    {
    case 0:
        ui->label_pumproom->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumpgroup1->setStyleSheet("background-color: rgb(0, 255, 0);");
        ui->label_pumpgroup2->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumpgroup3->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumpgroup4->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 1:
        ui->label_pumpgroup1->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumpgroup2->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    case 2:
        ui->label_pumpgroup2->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumpgroup3->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    case 3:
        ui->label_pumpgroup3->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumpgroup4->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    case 4:
        ui->label_pumpgroup4->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_pumproom->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    }
}
void Control::display3()
{
    if(selectDev==4)
    {
        switch(selectOpt)
        {
        case 0:
            ui->label_OneKeyClose1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyOpen->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_OneKeyClose->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyOpen1->setStyleSheet("background-color: rgb(255, 255, 255);");

            ui->label_OneKeyClose2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyOpen2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyClose3->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyOpen3->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 1:
            ui->label_OneKeyOpen->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyClose->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        case 2:
            ui->label_OneKeyClose->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyOpen1->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        case 3:
            ui->label_OneKeyOpen1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyClose1->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        case 4:
            ui->label_OneKeyClose1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyOpen2->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        case 5:
            ui->label_OneKeyOpen2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyClose2->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        case 6:
            ui->label_OneKeyClose2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyOpen3->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        case 7:
            ui->label_OneKeyOpen3->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyClose3->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        }
    }
    else
    {
        switch(selectOpt)
        {
        case 0:
            ui->label_CarshStop->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyOpen->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_OneKeyClose->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OverHaul->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 1:
            ui->label_OneKeyOpen->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OneKeyClose->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        case 2:
            ui->label_OneKeyClose->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_OverHaul->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        case 3:
            ui->label_OverHaul->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_CarshStop->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        }
    }
}
void Control::remoteoneswitch_signal(INT8U onekeyswitch)
{

    pumpswitchdeal();
    qDebug()<<"aaaaaa pumpswitchdeal"<<onekeyswitch;
}
