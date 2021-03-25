#include "realtimefault.h"
#include "ui_realtimefault.h"
#include <iterator>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
realtimefault::realtimefault(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::realtimefault)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

   // clearparam();
}

realtimefault::~realtimefault()
{
    delete ui;
}
#if 0
void realtimefault::keyPressEvent(QKeyEvent *pkey)
{
    if(pkey->key() == Qt::Key_F1)// stop music
    {
        ;
    }
    else if(pkey->key() == Qt::Key_Return) // confirm
    {
        int row = ui->tableWidget->currentRow();
        ui->tableWidget->setFocus();
        if(ui->tableWidget->currentItem())
        {
            qDebug()<<"enter setup"<<ui->tableWidget->currentRow()<<ui->tableWidget->currentItem()->column();

            ui->tableWidget->item(row,1)->setBackgroundColor(QColor(0,255,0));
            ui->tableWidget->item(row,1)->setText(tr("是"));

            storefaultSOEinfo localonestorefaultSOEinfo;
            localonestorefaultSOEinfo = pfaultRealTimelist->at(row);
            localonestorefaultSOEinfo.isconfirmed = 1;
            pfaultRealTimelist->removeAt(row);
            pfaultRealTimelist->insert(row,localonestorefaultSOEinfo);
            qDebug()<<"enter setup"<<pfaultRealTimelist->count();
        }
    }
    else if(pkey->key() == Qt::Key_Escape) // hide
    {
       // pfaultRealTimelist->clear();
        int index = 0;

        int total = pfaultRealTimelist->count();
        for(int i = 0;i < total; i++)
        {
            if(pfaultRealTimelist->at(index).isconfirmed)
            {
                pfaultRealTimelist->removeAt(index);
            }
            else
            {
                index++;
            }
            qDebug()<<"number="<<i<<pfaultRealTimelist->count()<<index;
        }

//        if(pfaultRealTimelist->count() != 0)
//        {
//            QMessageBox msgbox(QMessageBox::Information, tr("提示"), tr("还有故障未确定,是否退出"));
//            QPushButton *buttonY = msgbox.addButton(tr("是"), QMessageBox::AcceptRole);
//            QPushButton *buttonN = msgbox.addButton(tr("否"), QMessageBox::RejectRole);
//            buttonN->setFocus();
//            msgbox.exec();

//            if (msgbox.clickedButton() == buttonY)
//            {
//                this->hide();
//            }
//        }
//        else
            this->hide();
    }
    else if(pkey->key() == Qt::Key_F3)
    {
        singleFaultSOEinfo.time = tr("2020-10-01 10:00:00");
        singleFaultSOEinfo.devname = tr("PIR8110");
        singleFaultSOEinfo.faultname = tr("短路故障");
        singleFaultSOEinfo.isconfirmed = 0;

        singleFaultSOEinfo.phyaddr = 2;

        pfaultRealTimelist->insert(0,singleFaultSOEinfo);

    }
    else if(pkey->key() == Qt::Key_F4)
    {
        displayFaultlist();
    }
    else if(pkey->key() == Qt::Key_F5)
    {
//        if(!testtimer.isActive())
//            testtimer.start(5000);
//        else
//            testtimer.stop();
//        connect(&testtimer,SIGNAL(timeout()),this,SLOT(testtimeoutslot()));
    }
}

void realtimefault::testtimeoutslot()
{
    DETAIL_VALUE testfaultinfo;
    testfaultinfo.soe.code = 1;
    testfaultinfo.soe.code++;
    int phyaddr = 3;
    testfaultinfo.value[8] = 2020;
    testfaultinfo.value[9] = 8;
    testfaultinfo.value[10] = 20;
    testfaultinfo.value[11] = 10;
    testfaultinfo.value[12] = 20;
    testfaultinfo.value[13] = 30;

    realtimefaultProcess(phyaddr,&testfaultinfo);
}

void realtimefault::realtimefaultProcess(int phyaddr, DETAIL_VALUE *pfaultinfo)
{
#if 1
    saveflag = 0;
    clearparam();
    singleFaultSOEinfo.phyaddr = phyaddr;
    singleFaultSOEinfo.code    = pfaultinfo->soe.code;
   // if(singleFaultSOEinfo.code)
    {
           // qDebug()<<singleFaultSOEinfo.code<<"singleFaultSOEinfo.code" ;

            for(int i =0;i < MAX_RTU_NUM;i++)
            {
                if(pvalid_cfg_info->info[i].phy_addr == phyaddr)
                {
                    singleFaultSOEinfo.logicaddr = i;
                    break;
                }
            }

            int rtuindex = pvalid_cfg_info->info[singleFaultSOEinfo.logicaddr].rtu_typeIndex;
            int verindex = pvalid_cfg_info->info[singleFaultSOEinfo.logicaddr].rtu_typeVer;
            singleFaultSOEinfo.devname = QString(QLatin1String(pvalid_cfg_info->info[singleFaultSOEinfo.logicaddr].num));
            singleFaultSOEinfo.devtype = QString(QLatin1String(pProtocollistIns->ProtoName[rtuindex]));
            singleFaultSOEinfo.time = QString("%1/%2/%3_%4:%5:%6").arg(pfaultinfo->value[8]).arg(pfaultinfo->value[9]).arg(pfaultinfo->value[10])
                    .arg(pfaultinfo->value[11]).arg(pfaultinfo->value[12]).arg(pfaultinfo->value[13]);

            int lookupok = 0;

            memset(singleFaultSOEinfo.faultnamearray,0x00,sizeof(singleFaultSOEinfo.faultnamearray));
            if(pvalid_value[singleFaultSOEinfo.logicaddr].isIntern)
            {
                for(int j=0;j<50;j++)
                {
                    if(pvalid_cfg_info->data[rtuindex][verindex].FaultCodeSection.Point[j].Code == singleFaultSOEinfo.code)
                    {
                        // singleFaultSOEinfo.faultname = QString(QLatin1String(pvalid_cfg_info->data[rtuindex][verindex].FaultCodeSection.Point[j].name));

                         memcpy(singleFaultSOEinfo.faultnamearray,pvalid_cfg_info->data[rtuindex][verindex].FaultCodeSection.Point[j].name,
                                strlen(pvalid_cfg_info->data[rtuindex][verindex].FaultCodeSection.Point[j].name));
                         lookupok = 1;
                         break;
                    }
                }
            }
            else
            {
                for(int j=0;(j<32)&&(j<pvalid_cfg_info->data[rtuindex][verindex].FaultParseSection.total);j++)
                {
                    int TeleIndex = 0;
                    if((singleFaultSOEinfo.code >> j)&0x1)
                    {
                         TeleIndex =  pvalid_cfg_info->data[rtuindex][verindex].FaultParseSection.MapIndex[j];
                       //  singleFaultSOEinfo.faultname = QString(QLatin1String(pvalid_cfg_info->data[rtuindex][verindex].TeleActionSection.Point[TeleIndex].name));

                         memcpy(singleFaultSOEinfo.faultnamearray,pvalid_cfg_info->data[rtuindex][verindex].TeleActionSection.Point[TeleIndex].name,
                                strlen(pvalid_cfg_info->data[rtuindex][verindex].TeleActionSection.Point[TeleIndex].name));
                         lookupok = 1;
                         break;
                    }
                }
            }
            if(lookupok == 0)
            {
                memcpy(singleFaultSOEinfo.faultnamearray,"未知故障",strlen("未知故障"));
            }

            singleFaultSOEinfo.isconfirmed = 0;
            pfaultRealTimelist->insert(0,singleFaultSOEinfo);
            if(pfaultRealTimelist->count() > 3)
            {
                int faultnum = pfaultRealTimelist->count();
                pfaultRealTimelist->removeAt(faultnum-1);
            }

            singleFaultSOEinfo.faultIndex ++;

           // if(pHistoryFaultSummaryIns->copybusyflag == 0)
           // {
              //  pHistoryFaultSummaryIns->copybusyflag = 1;
             //   pfaultHisTorylist->append(singleFaultSOEinfo);
              //  pHistoryFaultSummaryIns->copybusyflag = 0;
           // }
            displayFaultlist();
        #endif

           // this->show();
     }
}

void realtimefault::displayFaultlist()
{
    storefaultSOEinfo sFaultinfo;
    QString confirmstr;

    int rowcnt = pfaultRealTimelist->count();
   //qDebug()<<__FUNCTION__<<__LINE__<<rowcnt;
    ui->tableWidget->setRowCount(rowcnt);
    for(int i=0;i<pfaultRealTimelist->count();i++)
    {
        sFaultinfo = pfaultRealTimelist->at(i);
//        QString time = QString("%1/%2/%3 %4:%5:%6").arg(sFaultinfo.year).arg(sFaultinfo.month).arg(sFaultinfo.day)
//                .arg(sFaultinfo.hour).arg(sFaultinfo.min).arg(sFaultinfo.sec);

        ui->tableWidget->setItem(i,0,new QTableWidgetItem(QString::number(i)));

        if(sFaultinfo.isconfirmed == 0)
            confirmstr = tr("否");
        else
            confirmstr = tr("是");
        ui->tableWidget->setItem(i,1,new QTableWidgetItem(confirmstr));
        ui->tableWidget->setItem(i,2,new QTableWidgetItem(sFaultinfo.time));
        ui->tableWidget->setItem(i,3,new QTableWidgetItem(QString::number(sFaultinfo.phyaddr)));

        ui->tableWidget->setItem(i,4,new QTableWidgetItem(sFaultinfo.devname));
        ui->tableWidget->setItem(i,5,new QTableWidgetItem(sFaultinfo.devtype));
        ui->tableWidget->setItem(i,6,new QTableWidgetItem(sFaultinfo.faultnamearray));
        ui->tableWidget->setItem(i,7,new QTableWidgetItem(sFaultinfo.code));
    }
    ui->tableWidget->resizeColumnsToContents();
}

void realtimefault::initfaultIndex()
{
    singleFaultSOEinfo.faultIndex = pHistoryFaultSummaryIns->currentIndex;
}

void realtimefault::clearparam()
{
    singleFaultSOEinfo.code = 0;

    singleFaultSOEinfo.devname.clear();//
    singleFaultSOEinfo.faultname.clear();
    singleFaultSOEinfo.time.clear();

    singleFaultSOEinfo.phyaddr = 0;

    singleFaultSOEinfo.logicaddr = 0;
    singleFaultSOEinfo.devtype.clear();//
}
#endif
