#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

#include <QObject>
#include <QTcpSocket>
#include "longlibrary.h"
#include "certificate.h"

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
	Long serverPublicKey;
	Long sessionKey;
	bool haveKey;
	ConnectionKeys() : haveKey(false) {}
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
	explicit ClientListener(const Long &serverPublicKey, Client *parent);
	void connectToHost(const QHostAddress &IP, quint16 port);
	void disconnectFromHost();
	void login(const Long &hashPIN);
	void getCertificates();

signals:
	void connected();
	void error(const QString &text);
	void disconnected();
	void recievedCertificates(const QList<Certificate> &list);

	void recheckForReadyRead();

public slots:
	void onConnected();
	void onError(QAbstractSocket::SocketError e);
	void onReadyRead();

private:
	Client *mParent;
	QTcpSocket mSocket;
	QString mAddressString;

	InputData mData;
	ConnectionKeys mKeys;
	SocketState mState;

	/*
	 * Write in socket functions
	 */
	void sendData(quint16 type, const QByteArray &data);
	void sendRequest(quint16 type);
	void sendSessionKey();

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
	bool parseStateOK();
	bool parseTextMessage(const QString &text);
	bool parseLoginStatus(const QByteArray &byteArray);
	bool parseCertificates(const QByteArray &byteArray);

	/*
	 * Other
	 */
	void makeSessionKey();
	void encryptData(const QByteArray& input, QByteArray &output);
	void decryptData(const QByteArray& input, QByteArray &output);

};


#endif // SERVERLISTENER_H
