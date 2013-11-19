#-------------------------------------------------
#
# Project created by QtCreator 2013-11-19T13:16:04
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Server
TEMPLATE = app


SOURCES += main.cpp\
        server.cpp \
    dialogserversettings.cpp

HEADERS  += server.h \
    dialogserversettings.h

FORMS    += server.ui \
    dialogserversettings.ui
