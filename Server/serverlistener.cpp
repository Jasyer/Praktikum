#include "serverlistener.h"
#include "cryptograph.h"
#include <QTcpSocket>
#include <QTextCodec>
#include "../include/commands.h"

ServerListener::ServerListener(QTcpSocket *socket)
{
	mSocket = socket;
	mSocketID = socket->socketDescriptor();

	// connections for socket signals
	connect(mSocket, SIGNAL(disconnected()), SLOT(onDisconnected()));
	connect(mSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));

	sendText("Hello world!");
}

void ServerListener::onDisconnected()
{
	// emit diconnected(socket_id);
}

/**
 * @brief Reading bytes from socket
 *
 * [  size  |  type   |    data      ]
 * [4 bytes | 2 bytes |	(size) bytes ]
 */
void ServerListener::onReadyRead()
{
	if (mSocket->bytesAvailable() < 4)
		return;
}

void ServerListener::sendData(quint16 type, const QByteArray &data)
{
	quint32 size = data.size() + 2;
	writeUInt32(size);
	writeUInt16(type);
	mSocket->write(data);
}

void ServerListener::sendText(const QString &text)
{
	sendData(TYPE_TEXT_MESSAGE, QTextCodec::codecForLocale()->fromUnicode(text));
}

void ServerListener::writeUInt32(quint32 n)
{
	char buf[4];
	buf[3] = n & 0xff;
	n >>= 8;
	buf[2] = n & 0xff;
	n >>= 8;
	buf[1] = n & 0xff;
	n >>= 8;
	buf[0] = n & 0xff;
	mSocket->write(buf, 4);
}

void ServerListener::writeUInt16(quint16 n)
{
	char buf[2];
	buf[1] = n & 0xff;
	n >>=8;
	buf[0] = n & 0xff;
	mSocket->write(buf, 2);
}
