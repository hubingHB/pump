# -------------------------------------------------
# Project created by QtCreator 2016-01-14T11:26:02
# -------------------------------------------------
QT += network
#
#
#
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#
#
#
TARGET = powerhmi
TEMPLATE = app
SOURCES += main.cpp \
    widgetmain.cpp \
    measure.cpp \
    constant.cpp \
    control.cpp \
    fault.cpp \
    audioplay.cpp \
    centralctl.cpp \
    tinyxml2.cpp \
    realtimefault.cpp \
    modbustcp_base.cpp \
    modbustcpserver.cpp \
    SerialCommu.cpp \
    SerialCommuBase.cpp \
    SerialCommuModbus.cpp \
    pumpautorun.cpp \
    usrlogin.cpp

HEADERS += widgetmain.h \
    DefineDb.h \
    measure.h \
    constant.h \
    control.h \
    fault.h \
    audioplay.h \
    centralctl.h \
    tinyxml2.h \
    realtimefault.h \
    modbustcp_base.h \
    modbustcpserver.h \
    SerialCommu.h \
    SerialCommuBase.h \
    SerialCommuModbus.h \
    modbustcpheader.h \
    pumpautorun.h \
    usrlogin.h

FORMS += widgetmain.ui \
    measure.ui \
    constant.ui \
    control.ui \
    fault.ui \
    centralctl.ui \
    realtimefault.ui \
    pumpautorun.ui \
    usrlogin.ui

RESOURCES += \
    src.qrc


