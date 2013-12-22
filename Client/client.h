#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include "clientlistener.h"
#include "longlibrary.h"
#include "certificate.h"
#include <QHostAddress>

namespace Ui {
class Client;
}

class Client : public QMainWindow
{
	Q_OBJECT

public:
	explicit Client(QWidget *parent = 0);
	~Client();
	void printLog(const QString &text);


private slots:
	/*
	 * gui slots
	 */
	void onButtonConnectClicked();
	void onButtonDisconnectClicked();
	void onButtonLogInClicked();
	void onButtonGetCertificatesClicked();
	void onTypeCompanyActivated(int index);

	/*
	 * ClientListener slots
	 */
	void onConnected();
	void onError(const QString &text);
	void onRecievedCertificates(const QList<Certificate> &list);

private:
	Ui::Client *ui;
	ClientListener *mClientListener;
	void connectSignalsFromServerListener();
	void loadCompanyList();

	QHostAddress mServerIP;
	quint16 mServerPort;
	Long mServerPublicKey;

	QList<Certificate> mCertificates;
};

#endif // CLIENT_H
