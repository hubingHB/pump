#include "fault.h"
#include "ui_fault.h"
#include <QDebug>
#include "tinyxml2.h"

using namespace tinyxml2;

Fault::Fault(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Fault)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

Fault::~Fault()
{
    delete ui;
}

//void Fault::changeEvent(QEvent *e)
//{
//    QWidget::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
//}
void Fault::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Escape)
    {
        ui->tableWidget->clear();
        this->close();
    }
    else if(e->key() == Qt::Key_F1)
    {
        f1_key_process();
    }
    else if(e->key() == Qt::Key_Return)
    {
        enter_key_process();
    }
}

void Fault::f1_key_process()
{
    selectedoption++;
    if(selectedoption > 3)
        selectedoption = 0;
    displayselectedoption();
}

void Fault::enter_key_process()
{
    int fileSerialCode = 0;
    switch(selectedoption)
    {
    case 0:
        currentstart = 0;
        fileSerialCode = currentstart/200;
        readhistoryfaultfile(fileSerialCode);
        break;
    case 1:
        if(currentstart >= 10)
            currentstart -= 10;
        if(((currentstart+10)%200 == 0)&&(currentstart != 0))
        {
            fileSerialCode = currentstart/200;
            readhistoryfaultfile(fileSerialCode);
        }
        break;
    case 2:
        if(currentstart < ((pHistoryFaultSummaryIns->currentIndex/10)*10))
            currentstart += 10;
        if(currentstart%200 == 0)
        {
            fileSerialCode = currentstart/200;
            readhistoryfaultfile(fileSerialCode);
        }
        break;
    case 3:
    default:
        currentstart = (pHistoryFaultSummaryIns->currentIndex/10)*10;
        fileSerialCode = currentstart/200;
        readhistoryfaultfile(fileSerialCode);
        break;
    }

    //int index = currentstart%200;
    //qDebug()<<index<<currentstart<<"currentstart";
    displayFaultlist(currentstart);

}

void Fault::displayselectedoption()
{
    switch(selectedoption)
    {
    case 0:
        ui->label_head->setStyleSheet("background-color: rgb(0, 255, 0);");
        ui->label_prev->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_next->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_tail->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 1:
        ui->label_head->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_prev->setStyleSheet("background-color: rgb(0, 255, 0);");
        ui->label_next->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_tail->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 2:
        ui->label_head->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_prev->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_next->setStyleSheet("background-color: rgb(0, 255, 0);");
        ui->label_tail->setStyleSheet("background-color: rgb(255, 255, 255);");
        break;
    case 3:
    default:
        ui->label_head->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_prev->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_next->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->label_tail->setStyleSheet("background-color: rgb(0, 255, 0);");
        break;
    }
}

void Fault::EntryProcess()
{
    selectedoption = 0;
    currentstart   = 0;

//    int fileSerialCode = currentstart/200;
   // readhistoryfaultfile(fileSerialCode);

//    qDebug()<<"fault num="<<pfaultDisplaylist->count()
//            <<pfaultDisplaylist->at(0).time<<pfaultDisplaylist->at(0).phyaddr
//            <<pfaultDisplaylist->at(0).devname<<pfaultDisplaylist->at(0).devtype
//            <<pfaultDisplaylist->at(0).faultnamearray;

    displayselectedoption();
    displayFaultlist(0);
}

void Fault::displayFaultlist(int start)
{
    storefaultSOEinfo sFaultinfo;
    //ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setRowCount(10);
    int maxcount= 0;

    if(start > (pHistoryFaultSummaryIns->currentIndex/200)*200) // tail
    {
        maxcount = pHistoryFaultSummaryIns->currentIndex - (pHistoryFaultSummaryIns->currentIndex/200)*200;
    }
    else
    {
        maxcount = 200;
    }

    for(int i=0;i< 10;i++)
    {
        if((start%200 + i) < maxcount)
            sFaultinfo = pfaultDisplaylist->at(start%200 + i);
        else
            break;
        ui->tableWidget->setItem(i,0,new QTableWidgetItem(QString::number(currentstart + i+1)));
        ui->tableWidget->setItem(i,1,new QTableWidgetItem(sFaultinfo.time));
        ui->tableWidget->setItem(i,2,new QTableWidgetItem(QString::number(sFaultinfo.phyaddr)));
        ui->tableWidget->setItem(i,3,new QTableWidgetItem(sFaultinfo.devname));
        ui->tableWidget->setItem(i,4,new QTableWidgetItem(sFaultinfo.devtype));
        ui->tableWidget->setItem(i,5,new QTableWidgetItem(sFaultinfo.faultnamearray));
    }

    int currentNumStart = currentstart+1;
    int currentNumEnd   = currentstart+10;
    ui->label_page->setText(QString(tr("%1-%2")).arg(currentNumStart).arg(currentNumEnd));

    qDebug()<<pHistoryFaultSummaryIns->currentIndex<<"pHistoryFaultSummaryIns->currentIndex";
    ui->label_total->setText(QString::number(pHistoryFaultSummaryIns->currentIndex));
}


void Fault::readhistoryfaultfile(int fileScode)
{
    pfaultDisplaylist->clear();

    storefaultSOEinfo sFaultinfo;
    int total = 0;

    QString srcfilenamestr = tr("/substation/configfiles/historyfault/historyfault%1.xml").arg(fileScode);

    std::string midfilenamestr;
    const char *descfilenamestr;

    midfilenamestr = srcfilenamestr.toStdString();
    descfilenamestr = midfilenamestr.c_str();

    XMLElement* subNode;

    XMLDocument doc;
    XMLError ret = doc.LoadFile(descfilenamestr);

    if(ret != XML_SUCCESS)
    {
        qDebug()<<"failed"<<ret;
        return;
    }
    qDebug()<<"load success"<<ret<<__LINE__;

    XMLElement* root = doc.RootElement();

    if(root)
    {
        XMLElement* Node = root->FirstChildElement("total");
        if(Node)
        {
            total = atoi(Node->GetText());

        }

        Node = root->FirstChildElement("faultpoint");
        while(Node)
        {

            subNode = Node->FirstChildElement("time");
            if(subNode)
                sFaultinfo.time = QString(QLatin1String(subNode->GetText()));

            subNode = Node->FirstChildElement("devname");
            if(subNode)
                sFaultinfo.devname = QString(QLatin1String(subNode->GetText()));

            subNode = Node->FirstChildElement("devtype");
            if(subNode)
                sFaultinfo.devtype = QString(QLatin1String(subNode->GetText()));

            subNode = Node->FirstChildElement("phyaddr");
            if(subNode)
                sFaultinfo.phyaddr = atoi(subNode->GetText());

           // qDebug()<<"check something"<<__LINE__;
            subNode = Node->FirstChildElement("faultname");
          //  qDebug()<<"check something"<<__LINE__<<strlen(subNode->GetText());
            if(subNode)
                memcpy(sFaultinfo.faultnamearray, subNode->GetText(),strlen(subNode->GetText()));
          //  qDebug()<<"check something"<<__LINE__;
            pfaultDisplaylist->append(sFaultinfo);
            Node = Node->NextSiblingElement("faultpoint");
        }
    }
   // qDebug()<<"check something"<<__LINE__;
}

