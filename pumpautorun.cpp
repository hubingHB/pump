#include "pumpautorun.h"
#include "ui_pumpautorun.h"
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <math.h>

using namespace tinyxml2;
pumpautorun::pumpautorun(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::pumpautorun)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

pumpautorun::~pumpautorun()
{
    delete ui;
}

void pumpautorun::EnterProcess()
{
    windowdispay = 1;
    labelsettype = 0;
    lockFocus=0;
    widgetdisplay();
    interlockdisplay();
    updateDisplayUIoperate();
    freqdispaly.start(1000);
    connect(&freqdispaly,SIGNAL(timeout()),this,SLOT(frequpdata()));
}
void pumpautorun::widgetdisplay()
{
    if(labelsettype)
    {
        ui->widget_2->show();
        ui->widget->hide();
        lockFocus = 0 ;
        interlockstatuedisplay();
//        setvalue1 = psysStatus1->Mainsumpinterlock;
//        setvalue2 = psysStatus1->AuxiSumpinterlock;
//        setvalue3 = psysStatus1->Relationwaterpump;
//        setvalue4 = psysStatus1->Relationpumpgroup;
        ui->label_6->setText(tr("运行参数设定"));
    }
    else
    {
        ui->widget->show();
        ui->widget_2->hide();
        ui->lineEdit->clear();
        ui->label_freqset->clear();
        lockFocus = 0 ;
        comstadisplay();
        setvalue1 = 0;
        setvalue2 = 0;
        setvalue3 = 0;
        setvalue4 = 0;
        ui->label_6->setText(tr("运行频率设定"));
    }
}
void pumpautorun::interlockdisplay()
{
    switch(labelsettype)
    {
    case 0:
        ui->label_settype1->setStyleSheet("background-color: rgb(0, 255, 0);");
        ui->label_settype2->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 1:
        ui->label_settype1->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_settype2->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    }
}
void pumpautorun::interlockstatuedisplay()
{
//    if(psysStatus1->Mainsumpinterlock)
//    {
//        ui->label_setvalue1->setText(tr("投入"));
//    }
//    else
//    {
//        ui->label_setvalue1->setText(tr("退出"));
//    }
//    if(psysStatus1->AuxiSumpinterlock)
//    {
//        ui->label_setvalue2->setText(tr("投入"));
//    }
//    else
//    {
//        ui->label_setvalue2->setText(tr("退出"));
//    }
//    if(psysStatus1->Relationwaterpump)
//    {
//        ui->label_setvalue3->setText(tr("关联"));
//    }
//    else
//    {
//        ui->label_setvalue3->setText(tr("不关联"));
//    }
//    if(psysStatus1->Relationpumpgroup)
//    {
//        ui->label_setvalue4->setText(tr("2#水泵"));
//    }
//    else
//    {
//        ui->label_setvalue4->setText(tr("1#水泵"));
//    }
}
void pumpautorun::keymovedisplay()
{
    if(setvalue1)
    {
        ui->label_setvalue1->setText(tr("投入"));
    }
    else
    {
        ui->label_setvalue1->setText(tr("退出"));
    }
    if(setvalue2)
    {
        ui->label_setvalue2->setText(tr("投入"));
    }
    else
    {
        ui->label_setvalue2->setText(tr("退出"));
    }
    if(setvalue3)
    {
        ui->label_setvalue3->setText(tr("关联"));
    }
    else
    {
        ui->label_setvalue3->setText(tr("不关联"));
    }
    if(setvalue4)
    {
        ui->label_setvalue4->setText(tr("2#水泵"));
    }
    else
    {
        ui->label_setvalue4->setText(tr("1#水泵"));
    }
}
void pumpautorun::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_F1)
    {
        labelsettype++;
        if(labelsettype > 1)
        {
            labelsettype = 0;
        }
        lockFocus =0;
        interlockdisplay();
        widgetdisplay();
    }
    else if(e->key() == Qt::Key_F2)
    {
            lockFocus++;
            if(lockFocus > 3)
            {
                lockFocus = 0;
            }
            updateDisplayUIoperate();
    }
    if(e->key() == Qt::Key_Left)
    {
        if(labelsettype==0)
        {
            QString str = ui->lineEdit->text();
            if(str.length() > 0)
            {
                str.chop(1);
            }
            ui->lineEdit->setText(str);
        }
    }
    if(e->key() == Qt::Key_Up)
    {
        if(labelsettype)
        {
            switch (lockFocus) {
            case 0:
                   setvalue1++;
                   if(setvalue1 > 1)
                   {
                       setvalue1 = 0;
                   }
                break;
            case 1:
                   setvalue2++;
                   if(setvalue2 > 1)
                   {
                       setvalue2 = 0;
                   }
                break;
            case 2:
                   setvalue3++;
                   if(setvalue3 > 1)
                   {
                       setvalue3 = 0;
                   }
                break;
            case 3:
                   setvalue4++;
                   if(setvalue4 > 1)
                   {
                       setvalue4 = 0;
                   }
                break;
            default:
                break;
            }
            keymovedisplay();
        }
    }
    if(e->key() == Qt::Key_Down)
    {
        if(labelsettype)
        {
            switch (lockFocus) {
            case 0:
                setvalue1--;
                if(setvalue1 < 0)
                {
                    setvalue1 = 1;
                }
                break;
            case 1:
                setvalue2--;
                if(setvalue2 < 0)
                {
                    setvalue2 = 1;
                }
                break;
            case 2:
                setvalue3--;
                if(setvalue3 < 0)
                {
                    setvalue3 = 1;
                }
                break;
            case 3:
                setvalue4--;
                if(setvalue4 < 0)
                {
                    setvalue4 = 1;
                }
                break;
            default:
                break;
            }
            keymovedisplay();
        }
    }
    else if(e->key() == Qt::Key_F3)
    {
        emit freq_set(logicaddr);
    }
    else if(e->key() == Qt::Key_Escape)
    {
        freqdispaly.stop();
        lockFocus =0;
        logicaddr =0;
         windowdispay = 0;
         labelsettype = 0;
        this->hide();
    }
    else if(e->key() == Qt::Key_Return)
    {
        if(labelsettype)
        {
            pDeviceDataBase->Mainsumpinterlock =  setvalue1;
            pDeviceDataBase->AuxiSumpinterlock =  setvalue2;
            pDeviceDataBase->OneKeyRelateCtlPump =  setvalue3;
            pDeviceDataBase->PumpStartNo =  setvalue4;
            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("是否确认修改"));
            QPushButton *buttonY = msgbox.addButton(tr("是"), QMessageBox::YesRole);
            QPushButton *buttonN = msgbox.addButton(tr("否"), QMessageBox::NoRole);
            buttonN->setFocus();
            msgbox.exec();
            if(msgbox.clickedButton() == buttonY)
            {
             lockfilewrite();
            }
        }
        else
        {
            QString str = ui->lineEdit->text();
            if(str.toFloat()<50)
            {
                enterkeypressprocess();
            }
            else
            {
                ui->label_tip->setText(tr("设置范围：0~50HZ！"));
                QMessageBox::information(this,tr("提示"),tr("设置范围：0~50HZ！"),QMessageBox::Ok);
            }
        }
    }
}

void pumpautorun::updateDisplayUIoperate()
{
    if(labelsettype)
    {
        switch(lockFocus)
        {
        case 0:
            ui->label_set1->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_set2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set3->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set4->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 1:
            ui->label_set1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set2->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_set3->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set4->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 2:
            ui->label_set1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set3->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_set4->setStyleSheet("background-color: rgb(255, 255, 255);");
            break;
        case 3:
            ui->label_set1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set3->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_set4->setStyleSheet("background-color: rgb(0, 255, 0);");
            break;
        default:
            break;
        }
    }
    else
    {
        switch(lockFocus)
        {
        case 0:
            ui->label_pumpgroup1->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumpgroup2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup3->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup4->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpname->setText(tr("1#低负压泵给定频率"));
            break;
        case 1:
            ui->label_pumpgroup1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup2->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumpgroup3->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup4->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpname->setText(tr("2#低负压泵给定频率"));
            break;
        case 2:
            ui->label_pumpgroup1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup3->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumpgroup4->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpname->setText(tr("3#高负压泵给定频率"));
            break;
        case 3:
            ui->label_pumpgroup1->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup2->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup3->setStyleSheet("background-color: rgb(255, 255, 255);");
            ui->label_pumpgroup4->setStyleSheet("background-color: rgb(0, 255, 0);");
            ui->label_pumpname->setText(tr("4#高负压泵给定频率"));
            break;
        default:
            break;
        }
        comstadisplay();
    }
}

void pumpautorun::comstadisplay()
{
    switch (lockFocus) {
    case 0:
       logicaddr =0;
        break;
    case 1:
       logicaddr =1;
        break;
    case 2:
       logicaddr =16;
        break;
    case 3:
       logicaddr =17;
        break;
    default:
        break;
    }
//    ALL_TYPE_VALUE *pdata = &(pRealtimeData[logicaddr]);
//    if(pdata->commu.status == STATUS_ON)
//    {
//        emit constant_two_info(logicaddr,CMD_NONE,1,NULL,0);
//        ui->label_commstatus->setText(tr("正常"));
//    }
//    else
//    {
//        ui->label_commstatus->setText(tr("异常"));
//    }
//    ui->label_addr->setText(QString::number(pvalid_cfg_info->info[logicaddr].phy_addr,10).append("#"));
//    ui->label_com->setText("COM"+QString::number(pvalid_cfg_info->info[logicaddr].com_id));

}


void pumpautorun::frequpdata()
{

    switch (lockFocus) {
    case 0:
       logicaddr =0;
        break;
    case 1:
       logicaddr =1;
        break;
    case 2:
       logicaddr =16;
        break;
    case 3:
       logicaddr =17;
        break;
    default:
        break;
    }
//    ALL_TYPE_VALUE *pdata = &(pRealtimeData[logicaddr]);
  
//    ui->label_freqvalue->setText(QString::number(pdata->value[INFO_MEASURE_2].value[22]*0.01)+tr("Hz"));

}

void pumpautorun::enterkeypressprocess()
{
    QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("是否确认修改"));
    QPushButton *buttonY = msgbox.addButton(tr("是"), QMessageBox::YesRole);
    QPushButton *buttonN = msgbox.addButton(tr("否"), QMessageBox::NoRole);
    buttonN->setFocus();
    msgbox.exec();
    if(msgbox.clickedButton() == buttonY)
    {
        QString str = ui->lineEdit->text();
        ui->label_freqset->setText(str);
        float setvalue = ui->label_freqset->text().toFloat();
        writevalid_value1 = (u32)setvalue;
//        if(QMessageBox::Yes == QMessageBox::information(this,tr("警告"),tr("是否要整定值执行?"),QMessageBox::Yes,QMessageBox::No))
//        {
//            emit freq_info(logicaddr,CMD_EXE,1,&writevalid_value1,1);
//        }
    }
}

void pumpautorun::UK4ZcurrentFreqSlot(unsigned int freqvalue)
{
    ui->label_freqset->setText(QString::number(freqvalue));
}
void pumpautorun::lockfileSLOT()
{
    lockfilewrite();
    qDebug()<<"lockfileSLOT";
}
void pumpautorun::lockfileread()
{
    XMLDocument doc;
    XMLError ret = doc.LoadFile( "/waterpump/gaspumpconfigfiles/sysconfigfile.xml" );
    if(ret != XML_SUCCESS)
    {
        qDebug()<<"sysconfigfile.xml failed"<<ret<<__LINE__;
        return;
    }
//    qDebug()<<"load success"<<ret;

    XMLElement* root = doc.RootElement();
//    qDebug()<<"load 1success"<<ret;
    if(root)
    {
        XMLElement* subNodecommu = root->FirstChildElement("LIMITSET");
        if(subNodecommu)
        {
            pDeviceDataBase->Mainsumpinterlock= atoi(subNodecommu->FirstChildElement("Mainsumpinterlock")->GetText());
            // qDebug()<<__LINE__;
            pDeviceDataBase->AuxiSumpinterlock = atoi(subNodecommu->FirstChildElement("AuxiSumpinterlock")->GetText());
            // qDebug()<<__LINE__;
            pDeviceDataBase->OneKeyRelateCtlPump = atoi(subNodecommu->FirstChildElement("WhetherRelationPUMP")->GetText());
            // qDebug()<<__LINE__;
            pDeviceDataBase->PumpStartNo = atoi(subNodecommu->FirstChildElement("RelationPumpGroup")->GetText());
            // qDebug()<<__LINE__;
        }

    }
}
void pumpautorun::lockfilewrite()
{
    XMLDocument doc;
    XMLError ret = doc.LoadFile( "/waterpump/gaspumpconfigfiles/sysconfigfile.xml" );
    XMLElement * root = doc.RootElement();

    if(ret != XML_SUCCESS)
    {
        qDebug()<<"failed"<<ret<<__LINE__;
        return;
    }
    qDebug()<<"successsuccess";
    if(root)
    {
        XMLElement* Node1 = root->FirstChildElement("LIMITSET");
        if(Node1)
        {
            XMLElement* Node2 = Node1->FirstChildElement("Mainsumpinterlock");
            if(Node2)
            {
                Node2->SetText(pDeviceDataBase->Mainsumpinterlock);
            }
            XMLElement* Node3 = Node1->FirstChildElement("AuxiSumpinterlock");
            if(Node3)
            {
                Node3->SetText(pDeviceDataBase->AuxiSumpinterlock);
            }
            XMLElement* Node4 = Node1->FirstChildElement("WhetherRelationPUMP");
            if(Node4)
            {
                Node4->SetText(pDeviceDataBase->OneKeyRelateCtlPump);
            }
            XMLElement* Node5 = Node1->FirstChildElement("RelationPumpGroup");
            if(Node5)
            {
                Node5->SetText(pDeviceDataBase->PumpStartNo);
            }
        }
    }
    doc.SaveFile("/waterpump/gaspumpconfigfiles/sysconfigfile.xml");

    lockfileread();
}
void pumpautorun::setfileupdatesolt()
{
    if(windowdispay==0)
    {
         lockfileread();
    }

}
