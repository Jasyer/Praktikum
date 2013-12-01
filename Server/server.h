#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include "serversettings.h"
#include "serverlistener.h"
#include "clientsbase.h"
#include "longlibrary.h"

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
	void printLog(const QString &text);
	bool loginUser(const Long &hashPIN);

private:
	Ui::Server *ui;

	QTcpServer mServer;
	Long mPrivateKey;
	Long mPublicKey;

	QHash<int, ServerListener *> mClients;

	ClientsBase mClientsBase;

private slots:
	// gui slots
	void onClickedActionSettings();
	void onClickedActionStart();
	void onClickedActionStop();
	void onClickedActionDatabaseAddItem();
	void onClickedActionDatabaseRemoveItem();

	// server slots
	void onErrorAccepted(QAbstractSocket::SocketError e);
	void onNewConnection();
	void onDatabaseAddNewItem(const QString &name, const Long &hashPIN);

	void makePrivatePublicKeys();
};

#endif // SERVER_H
