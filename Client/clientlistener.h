#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

#include <QObject>
#include <QTcpSocket>
#include "longlibrary.h"

struct InputData
{
	quint32 size;
	quint16 type;
	QByteArray data;
	void clear()
	{
		size = 0;
		type = 0;
		data.clear();
	}
};

struct ConnectionKeys
{
	char aesKey[32];
	Long currentKey;
	Long privateKey;
	Long publicKey;
	Long privateSessionKey;
	Long publicSessionKey;

	bool haveServerKeys;
	Long serverPublicKey;
	Long serverPublicSessionKey;
	ConnectionKeys() : haveServerKeys(false) {}
};

struct SocketState
{
	bool waiting;
	quint16 waitingType;
	SocketState() : waiting(false), waitingType(0) {}
	void clear()
	{
		waiting = false;
		waitingType = 0;
	}
};

class Client;

class ClientListener : public QObject
{
	Q_OBJECT

public:
	explicit ClientListener(const Long &privateKey, const Long &publicKey, Client *parent);
	void connectToHost(const QString &IP, const QString &port);
	void login(const Long &hashPIN);

signals:
	void connected();
	void error(const QString &text);
	void disconnected();
	void message(const QString &msg);
	void logined();

	void recheckForReadyRead();

public slots:
	void onConnected();
	void onError(QAbstractSocket::SocketError e);
	void onReadyRead();

private:
	Client *mParent;
	QTcpSocket mSocket;
	QString mAddress;

	InputData mData;
	ConnectionKeys mKeys;
	SocketState mState;

	/*
	 * Write in socket functions
	 */
	void sendData(quint16 type, const QByteArray &data);
	void sendRequest(quint16 type);
	void sendPublicKeys();

	/*
	 * Read from socket functions
	 */
	quint32 readUInt32();
	quint16 readUInt16();

	/*
	 * Parse functions
	 */
	void parseInputData();
	bool parseStateReady();
	bool parseTextMessage(const QString &text);
	bool parsePublicKeys(const QByteArray &byteArray);
	bool parseLoginStatus(const QByteArray &byteArray);

	/*
	 * Other
	 */
	void makeSessionKeys();
	void makeKey(const ConnectionKeys &keys);

};


#endif // SERVERLISTENER_H
