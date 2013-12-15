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
	Long currentKey;
	Long privateKey;
	Long publicKey;
	Long privateSessionKey;
	Long publicSessionKey;

	bool haveClientKeys;
	Long clientPublicKey;
	Long clientPublicSessionKey;
	ConnectionKeys() : haveClientKeys(false) {}
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
	void connectToServer(const QString &IP, const QString &port);
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

	void sendPublicKeys();
	void sendCertificates();

	quint32 readUInt32();
	quint16 readUInt16();
	void writeUInt32(quint32 n);
	void writeUInt16(quint16 n);

	void parseInputData();
	bool parsePublicKeys(const QByteArray &byteArray);
	void parseLogin(const QByteArray &byteArray);

	void parseInputDataFromAnotherServer();
	void parseCertificates(const QByteArray &byteArray); // if mConnectToServer

	void makeSessionKeys();
	void makeKey(const ConnectionKeys &keys);

};

#endif // CLIENTLISTENER_H
