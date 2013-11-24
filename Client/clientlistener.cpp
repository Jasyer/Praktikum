#include "clientlistener.h"
#include <QHostAddress>
#include "../include/commands.h"

ClientListener::ClientListener()
{
	mData.flush();
}

void ClientListener::connectToHost(const QString &IP, const QString &port)
{
	mAddress = IP + ":" + port;
	mSocket.connectToHost((QHostAddress) IP, (quint16) port.toInt());

	connect(&mSocket, SIGNAL(connected()), SLOT(onConnected()));
	connect(&mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			SLOT(onError(QAbstractSocket::SocketError)));
	connect(&mSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
	connect(this, SIGNAL(recheckForReadyRead()), SLOT(onReadyRead()));
}

void ClientListener::onConnected()
{
	emit connected();
}

void ClientListener::onError(QAbstractSocket::SocketError e)
{
	switch(e)
	{
	case QAbstractSocket::ConnectionRefusedError:
		emit error("Connection refused by host");
		break;
	case QAbstractSocket::RemoteHostClosedError:
		emit error("Host closed connection");
		break;
	case QAbstractSocket::HostNotFoundError:
		emit error("Not found " + mAddress);
		break;
	default:
		emit error("Unknown error");
		break;
	}
}

/**
 * @brief ServerListener::onReadyRead
 *
 * Server message structure:
 *
 * [         4 bytes           | 2 bytes |	         ]
 * [ size of command + message | command | message   ]
 */

void ClientListener::onReadyRead()
{
	if (mData.size < 2)
	{
		if (mSocket.bytesAvailable() < 4)
			return;
		mData.size = readUInt32();
	}

	if (mSocket.bytesAvailable() < mData.size)
		return;

	mData.type = readUInt16();
	mData.size -= 2;

	char *buf = new char[mData.size];
	mSocket.read(buf, mData.size);
	mData.data = QByteArray::fromRawData(buf, mData.size);
	delete []buf;

	parseInputData();

	if (mSocket.bytesAvailable() > 0)
		emit recheckForReadyRead();
}

quint32 ClientListener::readUInt32()
{
	char buf[4];
	mSocket.read(buf, 4);
	return ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
}
quint16 ClientListener::readUInt16()
{
	char buf[2];
	mSocket.read(buf, 2);
	return ((buf[0] << 8) | buf[1]);
}

/**
 * @brief ServerListener::parseInputData
 *
 * Server command types:
 *
 */
void ClientListener::parseInputData()
{
	switch(mData.type)
	{
	case TYPE_TEXT_MESSAGE:
		parseTextMessage(QString(mData.data));
		break;
	}
	mData.flush();
}

void ClientListener::parseTextMessage(const QString &text)
{
	emit message(text);
}

