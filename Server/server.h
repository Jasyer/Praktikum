#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include "serverlistener.h"
#include "clientsbase.h"
#include "longlibrary.h"
#include "certificate.h"
#include <QHash>
#include <QList>

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
	QList<Certificate> getCertificates();

private:
	Ui::Server *ui;

	quint8 mServerType;
	Long mServerID;
	QString mServerName;
	QHostAddress mServerIP;
	quint16 mServerPort;
	QTcpServer mServer;
	Long mPrivateKey;
	Long mPublicKey;
	ServerListener *anotherServerListener;

	QMap<int, ServerListener *> mClients;
	QMap<ClientInfo, Certificate> mCertificates;
	ClientsBase mClientsBase;

	QList<ClientInfo> mListCertValid;
	QList<ClientInfo> mListCertInvoked;

	QList<QString> mAvailableHashList;
	QList<QString> mAvailableCipherList;

	void choiceServer();
	void updateCertificateList();

	void connectContextMenuSlots();

private slots:
	// gui slots
	void onClickedActionStart();
	void onClickedActionStop();
	void onClickedActionAdd_friend_server();
	void onClickedActionDatabaseAddItem();
	void onClickedActionDatabaseRemoveItem();

	// certificate list context menu
	void contextMenuListCertValid();
	void contextMenuListCertInvoked();

	void onClickedActionCertView();
	void onClickedActionCertInvoke();
	void onClickedActionCertReissue();

	// server slots
	void onErrorAccepted(QAbstractSocket::SocketError e);
	void onNewConnection();
	void onDatabaseAddNewItem(const QString &name, const Long &hashPIN);

	// server slots when connecting to another server
	void onDeleteMe();
	void onAddCertificates(const QList<Certificate> &certificates);

	// other
	void makePrivateKey();
	void loopUpdateCertificateList();
};

#endif // SERVER_H
