#ifndef COMMANDS_H
#define COMMANDS_H

#include <QHostAddress>
#include <QString>

static const quint16 TYPE_TEXT_MESSAGE = 1;
static const quint16 TYPE_SESSION_KEY = 2;
static const quint16 TYPE_LOGIN = 3;
static const quint16 TYPE_LOGIN_STATUS = 4;
static const quint16 TYPE_CERTIFICATES = 5;

static const quint16 STATE_READY = 33;
static const quint16 STATE_OK = 34;

static const quint16 REQUEST_CERTIFICATES = 1002;

static const quint16 ANSWER_LOGIN_OK = 2001;
static const quint16 ANSWER_LOGIN_FAIL = 2002;

static const quint8 SERVER_FEDERAL_AGENCY = 1;
static const quint8 SERVER_MINISTRY = 2;
static const QHostAddress ipFederalAgency = QHostAddress("127.0.0.1");
static const quint16 portFederalAgency = 1919;
static const QHostAddress ipMinistry = QHostAddress("127.0.0.1");
static const quint16 portMinistry = 1818;

#endif // COMMANDS_H
