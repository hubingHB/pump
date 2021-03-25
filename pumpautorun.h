#ifndef PUMPAUTORUN_H
#define PUMPAUTORUN_H

#include <QWidget>
#include "DefineDb.h"
#include "tinyxml2.h"

#include <QTimer>


namespace Ui {
class pumpautorun;
}

class pumpautorun : public QWidget
{
    Q_OBJECT

public:
    explicit pumpautorun(QWidget *parent = 0);
    ~pumpautorun();

//    CONFIGURE_FILE_CONTENT * pvalid_cfg_info;
//    sysStruct              * psystemStruct;
//    ProtocolList           * pProtocollistIns;

//    ALL_TYPE_VALUE         * pRealtimeData;

//    STsyscurStatus         * psysStatus1;
     Deivce_DataBase *pDeviceDataBase;

    void updateDisplayUIoperate();
    void enterkeypressprocess();

    void keyPressEvent(QKeyEvent *event);

    void readsysconfigxml();
    void updatelist();

    void writesysconfigxml();

    void autorunmainsump();
    void  displaydata();

    void EnterProcess();
    void comstadisplay();
    void widgetdisplay();
    void interlockdisplay();
    void interlockstatuedisplay();
    void lockfileread();
    void lockfilewrite();
    void keymovedisplay();
signals:
    void freq_info(int logic_addr,int cmd,int zone,u32 * value,u8 num);
    void freq_set(int logic_addr);
    void constant_two_info(int logic_addr,int cmd,int zone,u32 *value,u8 num);

public slots:
    void frequpdata();

    void UK4ZcurrentFreqSlot(unsigned int freqvalue);
    void lockfileSLOT();
    void setfileupdatesolt();

    //void FreqRead(int logicaddr);

private slots:

private:
    Ui::pumpautorun *ui;
    int windowdispay = 0;
    int labelsettype = 0;
    int lockFocus = 0;
    int logicaddr =0;
    int setvalue1 = 0 ;int setvalue2 = 0 ;int setvalue3 = 0 ;int setvalue4 = 0 ;
    QTimer freqdispaly;
    u32 writevalid_value1;

    u32 valid_value[MAX_VAL_NUM];
};

#endif // PUMPAUTORUN_H


