#ifndef CLIENTLISTENER_H
#define CLIENTLISTENER_H
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
	Long privateKey;
	Long publicKey;
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

class Server;

class ServerListener : public QObject
{
	Q_OBJECT

public:
	ServerListener(QTcpSocket *socket, const Long &privateKey, const Long &publicKey,
				   Server *parent, bool connectToServer = false);
	void connectToServer(const QHostAddress &IP, quint16 port);
	~ServerListener();

signals:
	void addCertificates(const QList<Certificate> &certificates);
	void deleteMe();
	void message(const QString &text, int socket);
	void checkForBytesAvailable();

private slots:
	void onConnected();
	void onError(const QAbstractSocket::SocketError &e);
	void onDisconnected();
	void onReadyRead();

private:
	bool mConnectToServer;
	QString mConnectToServerAddress;

	Server *mParent;
	QTcpSocket *mSocket;
	int mSocketID;

	InputData mData;
	ConnectionKeys mKeys;
	SocketState mState;

	void sendCommand(quint16 type);
	void sendAnswer(quint16 type, quint16 answer);
	void sendData(quint16 type, const QByteArray &data);
	void sendText(const QString &text);

	void sendCertificates();

	quint32 readUInt32();
	quint16 readUInt16();
	void writeUInt32(quint32 n);
	void writeUInt16(quint16 n);

	void parseInputData();
	bool parseSessionKey(const QByteArray &byteArray);
	void parseLogin(const QByteArray &byteArray);

	// if connectiong to server
	void parseInputDataFromAnotherServer();
	void makeSessionKey();
	void sendSessionKey();
	void parseCertificates(const QByteArray &byteArray);

	void encryptData(const QByteArray& input, QByteArray &output);
	void decryptData(const QByteArray& input, QByteArray &output);

};

#endif // CLIENTLISTENER_H
