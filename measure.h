#ifndef MEASURE_H
#define MEASURE_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "DefineDb.h"

#include <QFrame>
#include <QLabel>
#include <QIcon>
namespace Ui {
    class Measure;
}

class Measure : public QWidget {

    Q_OBJECT
public:
    Measure(QWidget *parent = 0);
    ~Measure();


    void EnterProcess();

    void DisplayMeasure(INT32S LogicAddr);

    Deivce_DataBase *pDeviceDataBase;
    INT32S mLogicAddr;
    INT32S mEnterFlag;
    INT32S selectIndex = 0;
    INT32S LogicAddrSET =0;
    void UpdateDisplayValue();

    void updateSelectedBtnDisplay(int index,int isPlus);

public slots:
    //void update_display(ALL_TYPE_VALUE *value);

    void UpdateDevStatusSlot(INT8U LogicAddr,INT8U FrameType,INT8U GroupIndex);
signals:
    void update_value(int logicaddr);

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    Ui::Measure *ui;
    QTimer update;

    void key_f1_press();
    void key_f2_press();
    void key_return_press();
    void key_escape_press();
//    void update_display_row(int index,int dis);
private slots:
//    void get_value();
};
#endif // MEASURE_H
