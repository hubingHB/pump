#include "widgetmain.h"
#include "ui_widgetmain.h"
#include <QTime>
#include <QNetworkInterface>
#include <QDebug>
#include <qmessagebox.h>

#include <QMessageBox>
#include <QPushButton>

#include <QIcon>
#include <QTimer>

#define max_fun 8
#define DEBUG 1

WidgetMain::WidgetMain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetMain)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    structinitflag = 0;
    ui_init();
}
WidgetMain::~WidgetMain()
{
    delete ui;
}
void WidgetMain::changeEvent(QEvent *e)
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
/******************************************************************************/
void WidgetMain::keyPressEvent(QKeyEvent *e)
{
    if(!e->isAutoRepeat())
    {
        //qDebug()<<e->key();
        switch(e->key())
        {
        case Qt::Key_F1:key_f1_press();break;
            //  case Qt::Key_Up:key_up_press();break;
            //  case Qt::Key_Down:key_down_press();break;
            //  case Qt::Key_Left:key_left_press();break;
            // case Qt::Key_Right:key_right_press();break;
        case Qt::Key_Return:key_return_press();break;
        case Qt::Key_F2:key_f2_press();break;
        case Qt::Key_F3:key_f3_press();break;
            break;
        }
    }
}

void WidgetMain::key_f1_press()
{
    mFunCode++;
    if(mFunCode > max_fun)
        mFunCode = 1;
    switch(mFunCode)
    {
    case 1:
        ui->label_frequencyset_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_login_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 2:
        ui->label_login_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_mode_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 3:
        ui->label_mode_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_operamode_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 4:
        ui->label_operamode_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_measure_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 5:
        ui->label_measure_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_control_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 6:
        ui->label_control_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_constant_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 7:
        ui->label_constant_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_soe_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 8:
        ui->label_soe_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_frequencyset_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    default:
        break;
    }
}
void WidgetMain::key_f2_press()
{
    mFunCode--;
    if(mFunCode < 1)
        mFunCode = max_fun;
    switch(mFunCode)
    {
    case 1:
        ui->label_mode_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_login_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 2:
        ui->label_operamode_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_mode_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 3:
        ui->label_measure_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_operamode_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 4:
        ui->label_control_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_measure_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 5:
        ui->label_constant_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_control_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 6:
        ui->label_soe_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_constant_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 7:
        ui->label_frequencyset_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_soe_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    case 8:
        ui->label_login_2->setStyleSheet("background-color:rgb(255,255,255);");
        ui->label_frequencyset_2->setStyleSheet("background-color:rgb(0,255,0);");
        break;
    default:
        break;
    }
}

void WidgetMain::key_f3_press()
{

}

void WidgetMain::key_return_press()
{
//    int LogicAddr = -1;
//    QString addrstr;

    if(mFunCode != 1) //
    {
        if(pDeviceDataBase->UsrLoginInfo.islogin == LOGOUT)
        {
            QMessageBox::information(this,tr("提示"),tr("用户未登录!"),QMessageBox::Ok);
            return;
        }
    }

    if(mFunCode < 9)
    {
        switch(mFunCode)
        {
        case 1:user_loginprocess();break;
        case 2:modeswitchprocess();break;
        case 3:operamodeswitchprocess();break;
        case 4:ui_measure_show();break;
        case 6:ui_constant_show();break;
        case 5:ui_control_show();break;
        case 7:ui_soe_show();break;
        case 8:ui_autorun_show();break;
        }
    }
    else
    {
        if((pDeviceDataBase->UsrLoginInfo.usrlevel == LVLENGINEER)||(pDeviceDataBase->UsrLoginInfo.usrlevel == LVLMANAGE))
            ui_sys_config_show();
        else
        {
            QMessageBox::information(this,tr("提示"),tr("当前用户无设置权限"),QMessageBox::Ok);
        }
    }

    //    qDebug()<<LogicAddr<<__FUNCTION__<<__LINE__;
    //    currentLogicaddr = LogicAddr;
}

/******************************************************************************/
void WidgetMain::DisplayTimeSlot() //
{
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss");
    ui->time_3->setText(str);
}
void WidgetMain::ui_init()
{
//    ui->time_flag->setVisible(false);
    /*****************************/
  //ui->ip->setText(QNetworkInterface().allAddresses().at(2).toString());//ip地址显示
    /*****************************/
    mClockTimer.start(1000);
    connect(&mClockTimer,SIGNAL(timeout()),this,SLOT(DisplayTimeSlot()));
}
/******************************************************************************/

void WidgetMain::UpdateDevStatusSlot(INT8U LogicAddr,INT8U FrameType,INT8U GroupIndex)
{
    //    qDebug()<<__FUNCTION__<<__LINE__;
    switch (LogicAddr) {
    case 0:    //pumpgroup 1
        displayBZ1();
        break;
    case 1:     //pumpgroup 2
        displayBZ2();
        break;
    case 16:    //pumpgroup 3
        displayBZ3();
        break;
    case 17:    //pumpgroup 4
        displayBZ4();
        break;
    case 32:    //pumproom
        displayBF1();
        break;
    default:
        break;
    }
}

void WidgetMain::displayBZ1()    //LogicAddr = 0
{
    ui->label_bz1_valve1->setText(QString::number(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_1].ValueData[19]) + tr("%")); //jinqi
    ui->label_bz1_valve2->setText(QString::number(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_1].ValueData[20]) + tr("%"));  //chuqi
    ui->label_bz1_valve5->setText(QString::number(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_1].ValueData[21]) + tr("%"));  //jinshui

    ui->label_bz1_display1->setText(QString::number(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_2].ValueData[20]*0.01) + tr("Hz"));
    ui->label_bz1_display2->setText(QString::number(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_2].ValueData[17]) + tr("V"));
    ui->label_bz1_display3->setText(QString::number(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_2].ValueData[19]*0.1) + tr("A"));

        ui->label_bcdf->setText(QString::number(pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_2].ValueData[22]));
    //jinqi chuqi jinshui
    switch (pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_1].ValueData[10]) {
    case 0:
        ui->label_bz1_mod1->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz1_mod1->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz1_mod1->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_1].ValueData[11]) {
    case 0:
        ui->label_bz1_mod2->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz1_mod2->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz1_mod2->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[0].value[INFO_MEASURE_1].ValueData[12]) {
    case 0:
        ui->label_bz1_mod5->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz1_mod5->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz1_mod5->setText(tr("远程"));
        break;
    }
    //up
    if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[0])     //1#jinqi
    {
        if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[2])  //1#chuqi
        {
            if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
    }
    else
    {
        if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[2])  //1#chuqi
        {
            if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz1_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
    }
    //down
    if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[8])  //jinshui
    {
        if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[12])   //wasi
        {

            ui->label_bz1_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-ON-JSQF-ON-W.png);");
            waterout = 1;
        }
        else
        {
            ui->label_bz1_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-OFF-JSQF-ON-W.png);");
            waterout = 0;
        }
    }
    else
    {
        waterout = 0;
        if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[12])  //wasibeng
        {
            ui->label_bz1_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-ON-JSQF-OFF-W.png);");
        }
        else
        {
            ui->label_bz1_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-OFF-JSQF-OFF-W.png);");
        }
    }

    if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[8] &&
            pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[12])
    {
        if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[33])
        {
            if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[4] &&
                    pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[6])
            {
                labeldisplay2_bz1();
            }
            else{
                labeldisplay3_bz1();
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[0] &&
                    pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[2])
            {
                labeldisplay1_bz1();
            }
            else{
                labeldisplay3_bz1();
            }
        }
    }
    else{
        labeldisplay3_bz1();
    }
    if(waterout1 || waterout2 || waterout3 || waterout)
    {
        ui->label_bf_cs1_w->show();
    }
    else
    {
        ui->label_bf_cs1_w->hide();
    }
    if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[14] ||
            pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[15]  )  //jianxiu or jiting
    {
        if(pDeviceDataBase->OneKeySwith ==1 || pDeviceDataBase->OneKeySwith==2)
        {
            pDeviceDataBase->OneKeySwith=0;
        }
    }
    if(pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[13]==0 &&
            pDeviceDataBase->DeviceValue[0].value[INFO_SWITCH].ValueData[12]==1)
    {
        ui->label_watercut1->show();
    }
    else{
        ui->label_watercut1->hide();
    }
}
void WidgetMain::displayBZ2()   //LogicAddr = 1
{
    ui->label_bz2_valve1->setText(QString::number(pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_1].ValueData[19]) + tr("%"));
    ui->label_bz2_valve2->setText(QString::number(pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_1].ValueData[20]) + tr("%"));
    ui->label_bz2_valve5->setText(QString::number(pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_1].ValueData[21]) + tr("%"));

    ui->label_bz2_display1->setText(QString::number(pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_2].ValueData[20]*0.01) + tr("Hz"));
    ui->label_bz2_display2->setText(QString::number(pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_2].ValueData[17]) + tr("V"));
    ui->label_bz2_display3->setText(QString::number(pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_2].ValueData[19]*0.1) + tr("A"));
    //jinqi chuqi jinshui
    switch (pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_1].ValueData[10]) {
    case 0:
        ui->label_bz2_mod1->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz2_mod1->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz2_mod1->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_1].ValueData[11]) {
    case 0:
        ui->label_bz2_mod2->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz2_mod2->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz2_mod2->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[1].value[INFO_MEASURE_1].ValueData[12]) {
    case 0:
        ui->label_bz2_mod5->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz2_mod5->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz2_mod5->setText(tr("远程"));
        break;
    }

    //up
    if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[0])     //1#jinqi
    {
        if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[2])  //1#chuqi
        {
            if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
    }
    else
    {
        if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[2])  //1#chuqi
        {
            if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz2_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
    }
    //down
    if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[8])  //jinshui
    {
        if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12])   //wasi
        {
            ui->label_bz2_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-ON-JSQF-ON-W.png);");
            ui->label_bz_w2->setVisible(true);
            ui->label_bz2_cs_w->setVisible(true);
            waterout1 = 1;
        }
        else
        {
            ui->label_bz2_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-OFF-JSQF-ON-W.png);");
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12]==0 &&
                    pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12]==0)
            {
                ui->label_bz_w2->setVisible(false);
            }
            ui->label_bz2_cs_w->setVisible(false);
            waterout1 = 0;
        }
    }
    else
    {
        if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12])  //wasibeng
        {
            ui->label_bz2_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-ON-JSQF-OFF-W.png);");
        }
        else
        {
            ui->label_bz2_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-OFF-JSQF-OFF-W.png);");
        }
        if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12]==0 &&
                pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12]==0)
        {
            ui->label_bz_w2->setVisible(false);
        }
        ui->label_bz2_cs_w->setVisible(false);
        waterout1 = 0;
    }

    if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[8] && pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12])
    {
        if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[33])
        {
            if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[4] && pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[16])
            {
                labeldisplay2_bz2();
            }
            else{
                labeldisplay3_bz2();
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[0] && pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[2])
            {
                labeldisplay1_bz2();
            }
            else{
                labeldisplay3_bz2();
            }
        }
    }
    else{
        labeldisplay3_bz2();
    }
    if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[14] ||
            pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[15]  )  //jianxiu or jiting
    {
        if(pDeviceDataBase->OneKeySwith ==1 || pDeviceDataBase->OneKeySwith==2)
        {
            pDeviceDataBase->OneKeySwith=0;
        }
    }
    if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[13]==0 &&
            pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12]==1)
    {
        ui->label_watercut1_2->show();
    }
    else{
        ui->label_watercut1_2->hide();
    }
}
void WidgetMain::displayBZ3()   //LogicAddr = 16
{
    ui->label_bz3_valve1->setText(QString::number(pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_1].ValueData[19]) + tr("%"));
    ui->label_bz3_valve2->setText(QString::number(pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_1].ValueData[20]) + tr("%"));
    ui->label_bz3_valve5->setText(QString::number(pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_1].ValueData[21]) + tr("%"));

    ui->label_bz3_display1->setText(QString::number(pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_2].ValueData[20]*0.01) + tr("Hz"));
    ui->label_bz3_display2->setText(QString::number(pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_2].ValueData[17]) + tr("V"));
    ui->label_bz3_display3->setText(QString::number(pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_2].ValueData[19]*0.1) + tr("A"));

    //jinqi chuqi jinshui
    switch (pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_1].ValueData[10]) {
    case 0:
        ui->label_bz3_mod1->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz3_mod1->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz3_mod1->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_1].ValueData[11]) {
    case 0:
        ui->label_bz3_mod2->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz3_mod2->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz3_mod2->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[16].value[INFO_MEASURE_1].ValueData[12]) {
    case 0:
        ui->label_bz3_mod5->setText(tr("停止"));
        break;
    case 4:
        ui->label_bz3_mod5->setText(tr("现场"));
        break;
    case 8:
        ui->label_bz3_mod5->setText(tr("远程"));
        break;
    }
    //up
    if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[0])     //1#jinqi
    {
        if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[2])  //1#chuqi
        {
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
    }
    else
    {
        if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[2])  //1#chuqi
        {
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[4])  //2#jinqi
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_ON_CQ2_ON.png);");
                }
                else{
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_ON_CQ2_OFF.png);");
                }
            }
            else
            {
                if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])  //2#chuqi
                {
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_OFF_CQ2_ON.png);");
                }
                else{
                     ui->label_bz3_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_OFF_CQ2_OFF.png);");
                }
            }
        }
    }
    //down
    if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[8])  //jinshui
    {
        if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12])   //wasi
        {

            ui->label_bz3_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-ON-JSQF-ON-W.png);");
            ui->label_bz_w2->setVisible(true);
            ui->label_bz_w3->setVisible(true);
            ui->label_bz3_cs_w->setVisible(true);
            waterout2 = 1;
        }
        else
        {
            ui->label_bz3_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-OFF-JSQF-ON-W.png);");
            if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12]==0 &&
                    pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12]==0)
            {
                ui->label_bz_w2->setVisible(false);
            }
            ui->label_bz_w3->setVisible(false);
            ui->label_bz3_cs_w->setVisible(false);
            waterout2 = 0;
        }
    }
    else
    {
        if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12])  //wasibeng
        {
            ui->label_bz3_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-ON-JSQF-OFF-W.png);");
        }
        else
        {
            ui->label_bz3_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-OFF-JSQF-OFF-W.png);");
        }
        if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12]==0 &&
                pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12]==0)
        {
            ui->label_bz_w2->setVisible(false);
        }
        ui->label_bz_w3->setVisible(false);
        ui->label_bz3_cs_w->setVisible(false);
        waterout2 = 0;
    }
    if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[8] &&
            pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12])
    {
        if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[33])
        {
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[4] &&
                    pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[6])
            {
                labeldisplay2_bz3();
            }
            else{
                labeldisplay3_bz3();
            }
        }
        else
        {
            if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[0] &&
                    pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[2])
            {
                labeldisplay1_bz3();
            }
            else{
                labeldisplay3_bz3();
            }
        }
    }
    else{
        labeldisplay3_bz3();
    }
    if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[14] ||
            pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[15]  )  //jianxiu or jiting
    {
        if(pDeviceDataBase->OneKeySwith > 2)
        {
            pDeviceDataBase->OneKeySwith=0;
        }
    }
    if(pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[13]==0 &&
            pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12]==1)
    {
        ui->label_watercut1_3->show();
    }
    else{
        ui->label_watercut1_3->hide();
    }
}
void WidgetMain::displayBZ4()   //LogicAddr = 17
{
    ui->label_bz4_valve1->setText(QString::number(pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_1].ValueData[19]) + tr("%"));
    ui->label_bz4_valve2->setText(QString::number(pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_1].ValueData[20]) + tr("%"));
    ui->label_bz4_valve5->setText(QString::number(pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_1].ValueData[21]) + tr("%"));

    ui->label_bz4_display1->setText(QString::number(pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_2].ValueData[20]*0.01) + tr("Hz"));
    ui->label_bz4_display2->setText(QString::number(pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_2].ValueData[17]) + tr("V"));
    ui->label_bz4_display3->setText(QString::number(pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_2].ValueData[19]*0.1) + tr("A"));

   //jinqi chuqi jinshui
   switch (pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_1].ValueData[10]) {
   case 0:
       ui->label_bz4_mod1->setText(tr("停止"));
       break;
   case 4:
       ui->label_bz4_mod1->setText(tr("现场"));
       break;
   case 8:
       ui->label_bz4_mod1->setText(tr("远程"));
       break;
   }
   switch (pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_1].ValueData[11]) {
   case 0:
       ui->label_bz4_mod2->setText(tr("停止"));
       break;
   case 4:
       ui->label_bz4_mod2->setText(tr("现场"));
       break;
   case 8:
       ui->label_bz4_mod2->setText(tr("远程"));
       break;
   }
   switch (pDeviceDataBase->DeviceValue[17].value[INFO_MEASURE_1].ValueData[12]) {
   case 0:
       ui->label_bz4_mod5->setText(tr("停止"));
       break;
   case 4:
       ui->label_bz4_mod5->setText(tr("现场"));
       break;
   case 8:
       ui->label_bz4_mod5->setText(tr("远程"));
       break;
   }

   //up
   if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[0])     //1#jinqi
   {
       if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[2])  //1#chuqi
       {
           if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[4])  //2#jinqi
           {
               if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])  //2#chuqi
               {
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_ON_CQ2_ON.png);");
               }
               else{
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_ON_CQ2_OFF.png);");
               }
           }
           else
           {
               if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])  //2#chuqi
               {
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_OFF_CQ2_ON.png);");
               }
               else{
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_ON_JQ2_OFF_CQ2_OFF.png);");
               }
           }
       }
       else
       {
           if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[4])  //2#jinqi
           {
               if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])  //2#chuqi
               {
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_ON_CQ2_ON.png);");
               }
               else{
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_ON_CQ2_OFF.png);");
               }
           }
           else
           {
               if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])  //2#chuqi
               {
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_OFF_CQ2_ON.png);");
               }
               else{
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_ON_CQ1_OFF_JQ2_OFF_CQ2_OFF.png);");
               }
           }
       }
   }
   else
   {
       if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[2])  //1#chuqi
       {
           if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[4])  //2#jinqi
           {
               if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])  //2#chuqi
               {
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_ON_CQ2_ON.png);");
               }
               else{
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_ON_CQ2_OFF.png);");
               }
           }
           else
           {
               if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])  //2#chuqi
               {
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_OFF_CQ2_ON.png);");
               }
               else{
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_ON_JQ2_OFF_CQ2_OFF.png);");
               }
           }
       }
       else
       {
           if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[4])  //2#jinqi
           {
               if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])  //2#chuqi
               {
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_ON_CQ2_ON.png);");
               }
               else{
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_ON_CQ2_OFF.png);");
               }
           }
           else
           {
               if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])  //2#chuqi
               {
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_OFF_CQ2_ON.png);");
               }
               else{
                    ui->label_bz4_up->setStyleSheet("border-image: url(:/YCP/YCP/JQ1_OFF_CQ1_OFF_JQ2_OFF_CQ2_OFF.png);");
               }
           }
       }
   }
   //down
   if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[8])  //jinshui
   {
       if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12])   //wasi
       {

           ui->label_bz4_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-ON-JSQF-ON-W.png);");
           ui->label_bz_w2->setVisible(true);
           ui->label_bf_w4->setVisible(true);
           ui->label_bz4_cs_w->setVisible(true);
           waterout3 = 1;
       }
       else
       {
           ui->label_bz4_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-OFF-JSQF-ON-W.png);");
           if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12]==0 &&
                  pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12]==0)
           {
               ui->label_bz_w2->setVisible(false);
           }
           ui->label_bf_w4->setVisible(false);
           ui->label_bz4_cs_w->setVisible(false);
           waterout3 = 0;
       }
   }
   else
   {
       if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12])  //wasibeng
       {
           ui->label_bz4_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-ON-JSQF-OFF-W.png);");
       }
       else
       {
           ui->label_bz4_down->setStyleSheet("border-image: url(:/Gas Picture/WSB-OFF-JSQF-OFF-W.png);");
       }
       if(pDeviceDataBase->DeviceValue[1].value[INFO_SWITCH].ValueData[12]==0 &&
               pDeviceDataBase->DeviceValue[16].value[INFO_SWITCH].ValueData[12]==0)
       {
           ui->label_bz_w2->setVisible(false);
       }
       ui->label_bf_w4->setVisible(false);
       ui->label_bz4_cs_w->setVisible(false);
       waterout3 = 0;
   }
   if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[8] && pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12])
   {
       if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[33])
       {
           if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[4] &&
                   pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[6])
           {
               labeldisplay2_bz4();
           }
           else{
               labeldisplay3_bz4();
           }
       }
       else
       {
           if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[0] &&
                   pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[2])
           {
               labeldisplay1_bz4();
           }
           else{
               labeldisplay3_bz4();
           }
       }
   }
   else{
       labeldisplay3_bz4();
   }
   if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[14] ||
           pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[15]  )  //jianxiu or jiting
   {
       if(pDeviceDataBase->OneKeySwith > 2)
       {
           pDeviceDataBase->OneKeySwith=0;
       }
   }
   if(pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[13]==0 &&
           pDeviceDataBase->DeviceValue[17].value[INFO_SWITCH].ValueData[12]==1)
   {
       ui->label_watercut1_4->show();
   }
   else{
       ui->label_watercut1_4->hide();
   }
}
void WidgetMain::displayBF1()   //logicaddr = 32
{
    ui->label_26->setText(QString::number(pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_1].ValueData[2]*0.01)+ tr("米"));
    ui->label_19->setText(QString::number(pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_1].ValueData[3]*0.01)+ tr("米"));


    ui->label_bf1_valve1->setText(QString::number(pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_3].ValueData[16])+ tr("%"));
    ui->label_bf2_valve1->setText(QString::number(pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_3].ValueData[17])+ tr("%"));
    ui->label_bf3_valve1->setText(QString::number(pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_3].ValueData[18])+ tr("%"));
    ui->label_bf4_valve1->setText(QString::number(pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_3].ValueData[19])+ tr("%"));

    switch (pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_1].ValueData[8]) {
    case 0:
        ui->label_bf1_mod1->setText(tr("停止"));
        break;
    case 4:
        ui->label_bf1_mod1->setText(tr("现场"));
        break;
    case 8:
        ui->label_bf1_mod1->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_1].ValueData[9]) {
    case 0:
        ui->label_bf2_mod1->setText(tr("停止"));
        break;
    case 4:
        ui->label_bf2_mod1->setText(tr("现场"));
        break;
    case 8:
        ui->label_bf2_mod1->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_1].ValueData[10]) {
    case 0:
        ui->label_bf3_mod1->setText(tr("停止"));
        break;
    case 4:
        ui->label_bf3_mod1->setText(tr("现场"));
        break;
    case 8:
        ui->label_bf3_mod1->setText(tr("远程"));
        break;
    }
    switch (pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_1].ValueData[11]) {
    case 0:
        ui->label_bf4_mod1->setText(tr("停止"));
        break;
    case 4:
        ui->label_bf4_mod1->setText(tr("现场"));
        break;
    case 8:
        ui->label_bf4_mod1->setText(tr("远程"));
        break;
    }
    ui->label_gd_value1->setText(QString::number(pDeviceDataBase->SafeData[0]*0.01)+ tr("KPa"));
    ui->label_gd_value3->setText(QString::number(pDeviceDataBase->SafeData[1]*0.01)+ tr("%"));
    ui->label_gd_value5->setText(QString::number(pDeviceDataBase->SafeData[2]*0.01)+ tr("m3/h"));
    ui->label_gd_value7->setText(QString::number(pDeviceDataBase->SafeData[3]*0.01)+ tr("℃"));
    ui->label_gd_value9->setText(QString::number(pDeviceDataBase->SafeData[4]*0.01)+ tr("%"));

    ui->label_gd_value2->setText(QString::number(pDeviceDataBase->SafeData[5]*0.01)+ tr("KPa"));
    ui->label_gd_value4->setText(QString::number(pDeviceDataBase->SafeData[6]*0.01)+ tr("%"));
    ui->label_gd_value6->setText(QString::number(pDeviceDataBase->SafeData[7]*0.01)+ tr("m3/h"));
    ui->label_gd_value8->setText(QString::number(pDeviceDataBase->SafeData[8]*0.01)+ tr("℃"));
    ui->label_gd_value10->setText(QString::number(pDeviceDataBase->SafeData[9]*0.01)+ tr("%"));

    //The liquid level of the high pool is not zero
    if(pDeviceDataBase->DeviceValue[32].value[INFO_MEASURE_1].ValueData[10] > 0)
    {
        ui->label_inite3_w->show();
        ui->label_inite1_w->show();
        ui->label_inite2_w_2->show();
        ui->label_inite2_w->show();
    }
    else
    {
        ui->label_inite3_w->hide();
        ui->label_inite1_w->hide();
        ui->label_inite2_w_2->hide();
        ui->label_inite2_w->hide();
    }
    //POOL DISPLAY
    if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[8])  //1#waterpump
    {
        if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[0])  //1#chushuifa
        {
            ui->label_waterA->setStyleSheet("border-image: url(:/Gas Picture/1#SB-ON-CSF-ON.png);");
            pipe = 1;
        }
        else
        {
           ui->label_waterA->setStyleSheet("border-image: url(:/Gas Picture/1#SB-ON-CSF-OFF.png);");
           pipe = 0;
        }
    }
    else
    {
        if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[0])
        {
            ui->label_waterA->setStyleSheet("border-image: url(:/Gas Picture/1#SB-OFF-CSF-ON.png);");
            pipe = 0;
        }
        else
        {
           ui->label_waterA->setStyleSheet("border-image: url(:/Gas Picture/1#SB-OFF-CSF-OFF.png);");
           pipe = 0;
        }
    }
//2#waterpump
    if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[9])
    {
        if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[2])
        {
            ui->label_waterB->setStyleSheet("border-image: url(:/Gas Picture/2#SB-ON-CSF-ON.png);");
            ui->label_pool2->show();
            pipe1 = 1;
        }
        else
        {
           ui->label_waterB->setStyleSheet("border-image: url(:/Gas Picture/2#SB-ON-CSF-OFF.png);");
           ui->label_pool2->hide();
           pipe1 = 0;
        }
    }
    else
    {
         ui->label_pool2->hide();
        if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[2])
        {
            ui->label_waterB->setStyleSheet("border-image: url(:/Gas Picture/2#SB-OFF-CSF-ON.png);");
            pipe1 = 0;
        }
        else
        {
           ui->label_waterB->setStyleSheet("border-image: url(:/Gas Picture/2#SB-OFF-CSF-OFF.png);");
           pipe1 = 0;
        }
    }
    //3#waterpump
    if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[10])
    {
        if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[4])
        {
            ui->label_waterC->setStyleSheet("border-image: url(:/Gas Picture/1#SB-ON-CSF-ON.png);");
            pipe2 = 1;
        }
        else
        {
           ui->label_waterC->setStyleSheet("border-image: url(:/Gas Picture/1#SB-ON-CSF-OFF.png);");
           pipe2 = 0;
        }
    }
    else
    {
        pipe2 = 0;
        if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[4])
        {
            ui->label_waterC->setStyleSheet("border-image: url(:/Gas Picture/1#SB-OFF-CSF-ON.png);");
        }
        else
        {
           ui->label_waterC->setStyleSheet("border-image: url(:/Gas Picture/1#SB-OFF-CSF-OFF.png);");
        }
    }
    //4#waterpump
    if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[11])
    {
        if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[6])
        {
            ui->label_waterD->setStyleSheet("border-image: url(:/Gas Picture/2#SB-ON-CSF-ON.png);");
            ui->label_pool4->show();
            pipe3 = 1;
        }
        else
        {
            ui->label_waterD->setStyleSheet("border-image: url(:/Gas Picture/2#SB-ON-CSF-OFF.png);");
            ui->label_pool4->hide();
            pipe3 = 0;
        }
    }
    else
    {
        pipe3 = 0;
        ui->label_pool4->hide();
        if(pDeviceDataBase->DeviceValue[32].value[INFO_SWITCH].ValueData[6])
        {
            ui->label_waterD->setStyleSheet("border-image: url(:/Gas Picture/2#SB-OFF-CSF-ON.png);");
        }
        else
        {
            ui->label_waterD->setStyleSheet("border-image: url(:/Gas Picture/2#SB-OFF-CSF-OFF.png);");
        }
    }
    if(pipe || pipe1)
    {
        ui->label_pool1_w->show();
    }
    else
        ui->label_pool1_w->hide();
    if(pipe2 || pipe3)
    {
        ui->label_pool3_w->show();
        ui->label_pool5->show();
        ui->label_pool6->show();
    }
    else
        ui->label_pool3_w->hide();
        ui->label_pool5->hide();
        ui->label_pool6->hide();
}
/******************************************************************************/
void WidgetMain::user_loginprocess() //
{
    if(pDeviceDataBase->UsrLoginInfo.islogin == LOGOUT)
        emit usrloginDisplaysignal();
    else
    {
        QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("是否退出登录"));
        QPushButton *buttonY = msgbox.addButton(tr("是"), QMessageBox::RejectRole);
        QPushButton *buttonN = msgbox.addButton(tr("否"), QMessageBox::AcceptRole);
        buttonN->setFocus();
        msgbox.exec();

        if (msgbox.clickedButton() == buttonY)
        {
            pDeviceDataBase->UsrLoginInfo.islogin = LOGOUT;
            pDeviceDataBase->UsrLoginInfo.usrlevel = 0;
            pDeviceDataBase->UsrLoginInfo.usrname = 0;
            usrlogoutslot();
        }
    }
}
void WidgetMain::modeswitchprocess()
{
    QString operatemode;
    if(pDeviceDataBase->StationRLMode == REMOTE_MODE)
    {
        operatemode = QString(tr("就地模式"));
    }
    else
    {
        operatemode = QString(tr("远控模式"));
    }

    QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("是否切换为")+operatemode);
    QPushButton *buttonY = msgbox.addButton(tr("是"), QMessageBox::YesRole);
    QPushButton *buttonN = msgbox.addButton(tr("否"), QMessageBox::NoRole);

    buttonN->setFocus();
    msgbox.exec();

    if(msgbox.clickedButton() == buttonY)
    {
        if(pDeviceDataBase->StationRLMode == REMOTE_MODE)
        {
            pDeviceDataBase->StationRLMode = LOCAL_MODE;//locate
            ui->label_modelocate->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_moderemote->setStyleSheet("background-color: rgb(255, 255, 255);");
        }
        else
        {
            pDeviceDataBase->StationRLMode = REMOTE_MODE;//remote
            ui->label_modelocate->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_moderemote->setStyleSheet("background-color: rgb(0, 255, 0);");
        }
    }
}

void WidgetMain::operamodeswitchprocess()
{
    QString operamode;
    if(pDeviceDataBase->StationAMMode == AUTO_MODE)
    {
        operamode = QString(tr("手动模式"));
    }
    else
    {
        operamode = QString(tr("自动模式"));
    }

    QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("是否切换为")+operamode);
    QPushButton *buttonY = msgbox.addButton(tr("是"), QMessageBox::YesRole);
    QPushButton *buttonN = msgbox.addButton(tr("否"), QMessageBox::NoRole);

    buttonN->setFocus();
    msgbox.exec();

    if(msgbox.clickedButton() == buttonY)
    {
        if(pDeviceDataBase->StationAMMode == AUTO_MODE)
        {
            pDeviceDataBase->StationAMMode = MANUAL_MODE;//locate
            ui->label_modemanual->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_modeauto->setStyleSheet("background-color: rgb(255, 255, 255);");
        }
        else
        {
            pDeviceDataBase->StationRLMode = AUTO_MODE;//remote
            ui->label_modemanual->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_modeauto->setStyleSheet("background-color: rgb(0, 255, 0);");
        }
    }
}
void WidgetMain::ui_measure_show()
{
    emit measureInsDisplaysignal();
}

void WidgetMain::ui_constant_show()
{
    if(pDeviceDataBase->UsrLoginInfo.usrlevel == LVLOPERATOR)
    {
        QMessageBox::information(this,tr("提示"),tr("用户权限不足!"),QMessageBox::Ok);
    }
    else{
        emit constantInsDisplaysignal();
    }
}
void WidgetMain::ui_control_show()
{
    if(pDeviceDataBase->StationRLMode == REMOTE_MODE)//remote mode
    {
        QMessageBox::information(this,tr("提示"),tr("设备处于远控模式，无法进行就地控制!"),QMessageBox::Ok);
    }
    else
    {
        emit controlInsDisplaysignal();
    }
}
void WidgetMain::ui_autorun_show()
{
    if(pDeviceDataBase->UsrLoginInfo.usrlevel == LVLOPERATOR)
    {
        QMessageBox::information(this,tr("提示"),tr("用户权限不足!"),QMessageBox::Ok);
    }
    else{
        emit autorunInsDisplaysignal();
    }
}
void WidgetMain::ui_soe_show()
{

}
void WidgetMain::ui_commu_show(int LogicAddr)
{
    if(LogicAddr != -1)
    {

    //    ui_commu.display_commu(LogicAddr,pvalid_cfg_info,&valid_all_value[LogicAddr].commu);
    //    ui_commu.move((800-400)/2,(480-250)/2);
    //    ui_commu.show();
    }
}
void WidgetMain::ui_commu_update()
{
    //int logic_addr = logic_code - 1;
 //   ui_commu.update_display(&valid_all_value[currentLogicaddr].commu);
}
void WidgetMain::ui_operate_show(int LogicAddr)
{

}

void WidgetMain::ui_sys_config_show()
{
//    if(pDeviceDataBase->StationRLMode == REMOTE_MODE)//remote mode
//    {
//        QMessageBox::information(this,tr("提示"),tr("设备处于远控模式，无法进行就地控制!"),QMessageBox::Ok);
//    }
//    else
//    {
//        emit sysconfigDisplaysignal();
//    }
}


void WidgetMain::usrloginslot()
{
    ui->label_usrname_3->setText(QString(tr("工号")) + QString::number(pDeviceDataBase->UsrLoginInfo.usrname));

    QString UsrName;
    if(pDeviceDataBase->UsrLoginInfo.usrlevel == LVLMANAGE)
    {
        UsrName = QString(tr("管理员"));
    }
    else if(pDeviceDataBase->UsrLoginInfo.usrlevel == LVLENGINEER)
    {
        UsrName = QString(tr("工程师"));
    }
    else
    {
        UsrName = QString(tr("操作员"));
    }

    ui->label_loginstatus_3->setText(UsrName + QString(tr("已登录")));

}
void WidgetMain::usrlogoutslot()
{
    ui->label_usrname_3->clear();
    ui->label_loginstatus_3->setText(tr("未登录"));
}


void WidgetMain::remoteswitch_local_remote_slot(INT8U currentmode)
{
    if(currentmode == LOCAL_MODE)
    {
        ui->label_modelocate->setStyleSheet("background-color: rgb(0, 255, 0);");
        ui->label_moderemote->setStyleSheet("background-color: rgb(255, 255, 255);");
    }
    else
    {
       ui->label_modelocate->setStyleSheet("background-color: rgb(255, 255, 255);");
       ui->label_moderemote->setStyleSheet("background-color: rgb(0, 255, 0);");
    }
}
void WidgetMain::remoteswitch_manual_auto_slot(INT8U currentmode)
{
    if(currentmode == AUTO_MODE)
    {
        ui->label_modeauto->setStyleSheet("background-color: rgb(0, 255, 0);");
        ui->label_modemanual->setStyleSheet("background-color: rgb(255, 255, 255);");
//        autoruntimer.start(1000);
    }
    else
    {
       ui->label_modeauto->setStyleSheet("background-color: rgb(255, 255, 255);");
       ui->label_modemanual->setStyleSheet("background-color: rgb(0, 255, 0);");
//       autoruntimer.stop();
    }
}
void WidgetMain::ui_display_info()
{

}
void WidgetMain::labeldisplay1_bz1()
{
    ui->label_bz1_t1->show();
    ui->label_bz1_t2->show();
    ui->label_bz1_t3->show();
    ui->label_bz1_t4->show();
    ui->label_bz1_t5->show();

    ui->label_bz1_t6->hide();
    ui->label_bz1_t7->hide();

    ui->label_bz1_t8->show();
    ui->label_bz1_t9->show();
    ui->label_bz1_t10->show();
    ui->label_bz1_t11->show();
    ui->label_bz1_t12->show();

    ui->label_bz1_t13->hide();
    ui->label_bz1_t14->hide();
    ui->label_bz1_t15->hide();
}
void WidgetMain::labeldisplay2_bz1()
{
    ui->label_bz1_t1->hide();
    ui->label_bz1_t2->hide();
    ui->label_bz1_t3->hide();
    ui->label_bz1_t4->hide();

    ui->label_bz1_t5->show();
    ui->label_bz1_t6->show();
    ui->label_bz1_t7->show();

    ui->label_bz1_t8->hide();
    ui->label_bz1_t9->hide();

    ui->label_bz1_t10->show();
    ui->label_bz1_t11->show();
    ui->label_bz1_t12->show();
    ui->label_bz1_t13->show();
    ui->label_bz1_t14->show();
    ui->label_bz1_t15->show();
}
void WidgetMain::labeldisplay3_bz1()
{
    ui->label_bz1_t1->hide();
    ui->label_bz1_t2->hide();
    ui->label_bz1_t3->hide();
    ui->label_bz1_t4->hide();
    ui->label_bz1_t5->hide();
    ui->label_bz1_t6->hide();
    ui->label_bz1_t7->hide();
    ui->label_bz1_t8->hide();
    ui->label_bz1_t9->hide();
    ui->label_bz1_t10->hide();
    ui->label_bz1_t11->hide();
    ui->label_bz1_t12->hide();
    ui->label_bz1_t13->hide();
    ui->label_bz1_t14->hide();
    ui->label_bz1_t15->hide();
}
void WidgetMain::labeldisplay1_bz2()
{
    ui->label_bz2_t1->show();
    ui->label_bz2_t2->show();
    ui->label_bz2_t3->show();
    ui->label_bz2_t4->show();
    ui->label_bz2_t5->show();

    ui->label_bz2_t6->hide();
    ui->label_bz2_t7->hide();

    ui->label_bz2_t8->show();
    ui->label_bz2_t9->show();
    ui->label_bz2_t10->show();
    ui->label_bz2_t11->show();
    ui->label_bz2_t12->show();

    ui->label_bz2_t13->hide();
    ui->label_bz2_t14->hide();
    ui->label_bz2_t15->hide();
}
void WidgetMain::labeldisplay2_bz2()
{
    ui->label_bz2_t1->hide();
    ui->label_bz2_t2->hide();
    ui->label_bz2_t3->hide();
    ui->label_bz2_t4->hide();

    ui->label_bz2_t5->show();
    ui->label_bz2_t6->show();
    ui->label_bz2_t7->show();

    ui->label_bz2_t8->hide();
    ui->label_bz2_t9->hide();

    ui->label_bz2_t10->show();
    ui->label_bz2_t11->show();
    ui->label_bz2_t12->show();
    ui->label_bz2_t13->show();
    ui->label_bz2_t14->show();
    ui->label_bz2_t15->show();
}
void WidgetMain::labeldisplay3_bz2()
{
    ui->label_bz2_t1->hide();
    ui->label_bz2_t2->hide();
    ui->label_bz2_t3->hide();
    ui->label_bz2_t4->hide();
    ui->label_bz2_t5->hide();
    ui->label_bz2_t6->hide();
    ui->label_bz2_t7->hide();
    ui->label_bz2_t8->hide();
    ui->label_bz2_t9->hide();
    ui->label_bz2_t10->hide();
    ui->label_bz2_t11->hide();
    ui->label_bz2_t12->hide();
    ui->label_bz2_t13->hide();
    ui->label_bz2_t14->hide();
    ui->label_bz2_t15->hide();
}
void WidgetMain::labeldisplay1_bz3()
{
    ui->label_bz3_t1->show();
    ui->label_bz3_t2->show();
    ui->label_bz3_t3->show();
    ui->label_bz3_t4->show();
    ui->label_bz3_t5->show();

    ui->label_bz3_t6->hide();
    ui->label_bz3_t7->hide();

    ui->label_bz3_t8->show();
    ui->label_bz3_t9->show();
    ui->label_bz3_t10->show();
    ui->label_bz3_t11->show();
    ui->label_bz3_t12->show();

    ui->label_bz3_t13->hide();
    ui->label_bz3_t14->hide();
    ui->label_bz3_t15->hide();
}
void WidgetMain::labeldisplay2_bz3()
{
    ui->label_bz3_t1->hide();
    ui->label_bz3_t2->hide();
    ui->label_bz3_t3->hide();
    ui->label_bz3_t4->hide();

    ui->label_bz3_t5->show();
    ui->label_bz3_t6->show();
    ui->label_bz3_t7->show();

    ui->label_bz3_t8->hide();
    ui->label_bz3_t9->hide();

    ui->label_bz3_t10->show();
    ui->label_bz3_t11->show();
    ui->label_bz3_t12->show();
    ui->label_bz3_t13->show();
    ui->label_bz3_t14->show();
    ui->label_bz3_t15->show();
}
void WidgetMain::labeldisplay3_bz3()
{
    ui->label_bz3_t1->hide();
    ui->label_bz3_t2->hide();
    ui->label_bz3_t3->hide();
    ui->label_bz3_t4->hide();
    ui->label_bz3_t5->hide();
    ui->label_bz3_t6->hide();
    ui->label_bz3_t7->hide();
    ui->label_bz3_t8->hide();
    ui->label_bz3_t9->hide();
    ui->label_bz3_t10->hide();
    ui->label_bz3_t11->hide();
    ui->label_bz3_t12->hide();
    ui->label_bz3_t13->hide();
    ui->label_bz3_t14->hide();
    ui->label_bz3_t15->hide();
}
void WidgetMain::labeldisplay1_bz4()
{
    ui->label_bz4_t1->show();
    ui->label_bz4_t2->show();
    ui->label_bz4_t3->show();
    ui->label_bz4_t4->show();
    ui->label_bz4_t5->show();

    ui->label_bz4_t6->hide();
    ui->label_bz4_t7->hide();

    ui->label_bz4_t8->show();
    ui->label_bz4_t9->show();
    ui->label_bz4_t10->show();
    ui->label_bz4_t11->show();
    ui->label_bz4_t12->show();

    ui->label_bz4_t13->hide();
    ui->label_bz4_t14->hide();
    ui->label_bz4_t15->hide();
}
void WidgetMain::labeldisplay2_bz4()
{
    ui->label_bz4_t1->hide();
    ui->label_bz4_t2->hide();
    ui->label_bz4_t3->hide();
    ui->label_bz4_t4->hide();

    ui->label_bz4_t5->show();
    ui->label_bz4_t6->show();
    ui->label_bz4_t7->show();

    ui->label_bz4_t8->hide();
    ui->label_bz4_t9->hide();

    ui->label_bz4_t10->show();
    ui->label_bz4_t11->show();
    ui->label_bz4_t12->show();
    ui->label_bz4_t13->show();
    ui->label_bz4_t14->show();
    ui->label_bz4_t15->show();
}
void WidgetMain::labeldisplay3_bz4()
{
    ui->label_bz4_t1->hide();
    ui->label_bz4_t2->hide();
    ui->label_bz4_t3->hide();
    ui->label_bz4_t4->hide();
    ui->label_bz4_t5->hide();
    ui->label_bz4_t6->hide();
    ui->label_bz4_t7->hide();
    ui->label_bz4_t8->hide();
    ui->label_bz4_t9->hide();
    ui->label_bz4_t10->hide();
    ui->label_bz4_t11->hide();
    ui->label_bz4_t12->hide();
    ui->label_bz4_t13->hide();
    ui->label_bz4_t14->hide();
    ui->label_bz4_t15->hide();
}
void WidgetMain::setfilerelease()
{
   emit setfileupdate();
}
