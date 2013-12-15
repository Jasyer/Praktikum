#-------------------------------------------------
#
# Project created by QtCreator 2013-11-19T13:16:04
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Server
TEMPLATE = app

INCLUDEPATH += ../include ../Common
LIBS += ../lib/libCryptograph.a ../lib/libLongLibrary.a

SOURCES += main.cpp\
        server.cpp \
    dialogserversettings.cpp \
    serversettings.cpp \
    serverlistener.cpp \
    database.cpp \
    clientsbase.cpp \
    dialogdatabaseitemedit.cpp \
    ../Common/certificate.cpp

HEADERS  += server.h \
    dialogserversettings.h \
    stringconstants.h \
    serversettings.h \
    serverlistener.h \
    database.h \
    certificate.h \
    clientsbase.h \
    dialogdatabaseitemedit.h \
    ../include/cryptoconstants.h \
    ../include/commands.h \
    ../Common/certificate.h

FORMS    += server.ui \
    dialogserversettings.ui \
    dialogdatabaseitemedit.ui
