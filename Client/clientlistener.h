#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

#include <QObject>
#include <QTcpSocket>

struct InputData
{
	quint32 size;
	quint16 type;
	QByteArray data;
	void flush()
	{
		size = 0;
		type = 0;
		data.clear();
	}
};

class ClientListener : public QObject
{
	Q_OBJECT

public:
	explicit ClientListener();
	void connectToHost(const QString &IP, const QString &port);

signals:
	void connected();
	void error(const QString &text);
	void disconnected();
	void message(const QString &msg);

	void recheckForReadyRead();

public slots:
	void onConnected();
	void onError(QAbstractSocket::SocketError e);
	void onReadyRead();

private:
	QTcpSocket mSocket;
	QString mAddress;
	InputData mData;

	quint32 readUInt32();
	quint16 readUInt16();
	void parseInputData();
	void parseTextMessage(const QString &text);
};

#endif // SERVERLISTENER_H
