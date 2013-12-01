#-------------------------------------------------
#
# Project created by QtCreator 2013-11-20T16:29:57
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Client
TEMPLATE = app

INCLUDEPATH += ../include
LIBS += ../lib/libCryptograph.a ../lib/libLongLibrary.a

SOURCES += main.cpp\
        client.cpp \
    clientlistener.cpp

HEADERS  += client.h \
    ../include/commands.h \
    clientlistener.h \
    ../include/cryptoconstants.h

FORMS    += client.ui
