#ifndef COMMANDS_H
#define COMMANDS_H

static const quint16 TYPE_TEXT_MESSAGE = 1;
static const quint16 TYPE_PUBLIC_KEYS = 2;
static const quint16 TYPE_LOGIN = 3;
static const quint16 TYPE_LOGIN_STATUS = 4;
static const quint16 TYPE_CERTIFICATES = 5;

static const quint16 STATE_READY = 33;

static const quint16 REQUEST_PUBLIC_KEYS = 1001;
static const quint16 REQUEST_CERTIFICATES = 1002;

static const quint16 ANSWER_LOGIN_OK = 2001;
static const quint16 ANSWER_LOGIN_FAIL = 2002;

#endif // COMMANDS_H
