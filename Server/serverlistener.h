#ifndef CLIENTLISTENER_H
#define CLIENTLISTENER_H
#include <QTcpSocket>

struct InputData
{
	quint32 size;
	quint16 type;
	QByteArray data;
};

class ServerListener : public QObject
{
	Q_OBJECT
public:
	ServerListener(QTcpSocket *socket);

signals:
	void message(const QString &text, int socket);
	void checkForBytesAvailable();

private slots:
	void onDisconnected();
	void onReadyRead();

private:
	QTcpSocket *mSocket;
	int mSocketID;
	InputData data;

	void sendData(quint16 type, const QByteArray &data);
	void sendText(const QString &text);

	quint32 readUInt32();
	quint16 readUInt16();
	void writeUInt32(quint32 n);
	void writeUInt16(quint16 n);

};

#endif // CLIENTLISTENER_H
