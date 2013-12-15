#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include "serversettings.h"
#include "serverlistener.h"
#include "clientsbase.h"
#include "longlibrary.h"
#include "certificate.h"
#include <QHash>
#include <QList>

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
	bool loginUser(const Long &hashPIN, const Long &publicKey);
	QList<Certificate> getCertificates();

private:
	Ui::Server *ui;

	Long mServerID;
	QString mServerName;
	QTcpServer mServer;
	Long mPrivateKey;
	Long mPublicKey;
	ServerListener *anotherServerListener;

	QMap<int, ServerListener *> mClients;
	QMap<ClientInfo, Certificate> mCertificates;
	ClientsBase mClientsBase;

	QList<QString> mAvailableHashList;
	QList<QString> mAvailableCipherList;

private slots:
	// gui slots
	void onClickedActionSettings();
	void onClickedActionStart();
	void onClickedActionStop();
	void onClickedActionAdd_friend_server();
	void onClickedActionDatabaseAddItem();
	void onClickedActionDatabaseRemoveItem();

	// server slots
	void onErrorAccepted(QAbstractSocket::SocketError e);
	void onNewConnection();
	void onDatabaseAddNewItem(const QString &name, const Long &hashPIN);

	// server slots when connecting to another server
	void onDeleteMe();
	void onAddCertificates(const QList<Certificate> &certificates);

	void makePrivatePublicKeys();
};

#endif // SERVER_H
