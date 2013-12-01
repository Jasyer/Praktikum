#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include "clientlistener.h"
#include  "longlibrary.h"

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
	void onButtonLogInClicked();

	/*
	 * ClientListener slots
	 */
	void onConnected();
	void onError(const QString &text);
	void onMessage(const QString &text);

	/*
	 * other slots
	 */
	void makePrivatePublicKey();

private:
	Ui::Client *ui;
	ClientListener *mClientListener;
	void connectSignalsFromServerListener();

	Long mPrivateKey;
	Long mPublicKey;
};

#endif // CLIENT_H
