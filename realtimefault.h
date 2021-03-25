#ifndef REALTIMEFAULT_H
#define REALTIMEFAULT_H

#include <QWidget>
#include <QKeyEvent>
#include "DefineDb.h"
#include <QTimer>
namespace Ui {
class realtimefault;
}

class realtimefault : public QWidget
{
    Q_OBJECT

public:
    explicit realtimefault(QWidget *parent = 0);
    ~realtimefault();


   // void realtimefaultProcess(int phyaddr,DETAIL_VALUE*pfaultinfo);

   // void displayFaultlist();

  //  void initfaultIndex();

    QList<storefaultSOEinfo>  *pfaultRealTimelist;
    QList<storefaultSOEinfo>  *pfaultHisTorylist;
    HistoryFaultSummary       *pHistoryFaultSummaryIns;


//    CONFIGURE_FILE_CONTENT * pvalid_cfg_info;
//    sysStruct              * psystemStruct;
//    ProtocolList           * pProtocollistIns;
//    ALL_TYPE_VALUE         * pvalid_value;

    Deivce_DataBase *pDeviceDataBase;

    storefaultSOEinfo singleFaultSOEinfo;

   // void clearparam();

    int saveflag = 1;

    int totalIndex = 0;

 //   QTimer testtimer;

//public slots:
//    void testtimeoutslot();

signals:
   // void savehistoryfaultsignal();

//protected:
//    void keyPressEvent(QKeyEvent *pkey);


private:
    Ui::realtimefault *ui;


};

#endif // REALTIMEFAULT_H
