#ifndef FAULT_H
#define FAULT_H

#include <QWidget>
#include <QKeyEvent>
#include "DefineDb.h"




namespace Ui {
    class Fault;
}
class Fault : public QWidget {
    Q_OBJECT
public:
    Fault(QWidget *parent = 0);
    ~Fault();

    int selectedoption = 0;
    int currentstart = 0;

     QList<storefaultSOEinfo>  *pfaultRealTimelist;
     QList<storefaultSOEinfo>  *pfaultHisTorylist;

     QList<storefaultSOEinfo>  *pfaultDisplaylist;
     HistoryFaultSummary       *pHistoryFaultSummaryIns;

//     CONFIGURE_FILE_CONTENT * pvalid_cfg_info;
//     sysStruct              * psystemStruct;
//     ProtocolList           * pProtocollistIns;
//     ALL_TYPE_VALUE         * pvalid_value;

     Deivce_DataBase *pDeviceDataBase;


//     void createhisfaultsummaryfile();
//     void createhisfaultfile();

     void f1_key_process();
     void displayselectedoption();

     void enter_key_process();

     void readhistoryfaultfile(int fileScode);


     void displayFaultlist(int start);

     void EntryProcess();
public slots:
    // void savehistoryfaultslot();
protected:
//    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *e);
private:
    Ui::Fault *ui;

};

#endif // FAULT_H
