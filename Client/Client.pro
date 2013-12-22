#-------------------------------------------------
#
# Project created by QtCreator 2013-11-20T16:29:57
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Client
TEMPLATE = app

INCLUDEPATH += ../include ../Common
LIBS += ../lib/libCryptograph.a ../lib/libLongLibrary.a

SOURCES += main.cpp\
        client.cpp \
    clientlistener.cpp \
    ../Common/certificate.cpp

HEADERS  += client.h \
    clientlistener.h \
    ../include/cryptoconstants.h \
    ../Common/certificate.h \
    ../include/commands.h

FORMS    += client.ui
