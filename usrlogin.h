#ifndef USRLOGIN_H
#define USRLOGIN_H

#include <QWidget>
#include "DefineDb.h"
#include <QKeyEvent>
#include <QTimer>


namespace Ui {
class usrlogin;
}

class usrlogin : public QWidget
{
    Q_OBJECT

public:
    explicit usrlogin(QWidget *parent = 0);
    ~usrlogin();

    Deivce_DataBase *pDeviceDataBase;

    INT32S mCurrentFocus;
    int keypressedflag[11];//

    UsrInfo singleusrinfo;
    QString usrstr;
    QString pwdstr;
    QString pwdstrshow;
    int currentAuth = 0;
    int currentname = 0;
    int rightpasswd = 0;

    int isLogin = 0;

    void validtimestart();
    void validtimestop();

    void updateDisplayUIoperate();
    void enterkeypressprocess();

    void getaccountauthority(QString name);

    void displayusrlogin();

    void usrlogoutprocess();

    void loginInit();
signals:
    void usrloginsignal();
    void usrlogoutsignal();

    void managerloginsignal();

    void usrpasswdmodifyDisplaysignal();

public slots:
    void validtimertimeoutslot();

protected:
    void keyPressEvent(QKeyEvent *pkey);
    void keyReleaseEvent(QKeyEvent *pkey);

private:
    Ui::usrlogin *ui;

    QTimer mLoginValidTimer;
};

#endif // USRLOGIN_H
