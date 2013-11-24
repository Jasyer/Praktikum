#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include "serversettings.h"
#include "serverlistener.h"

#define M_SETTING(q) ServerSettings::currentSettings()->get(q)

namespace Ui {
class Server;
}

class Server : public QMainWindow
{
	Q_OBJECT

public:
	explicit Server(QWidget *parent = 0);
	~Server();

private:
	Ui::Server *ui;

	QTcpServer mServer;
	QHash<int, ServerListener *> mClients;
	void printLog(const QString &text);

private slots:
	// gui slots
	void onClickedActionSettings();
	void onClickedActionStart();
	void onClickedActionStop();

	// server slots
	void onErrorAccepted(QAbstractSocket::SocketError e);
	void onNewConnection();
};

#endif // SERVER_H
