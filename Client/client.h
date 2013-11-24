#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include "clientlistener.h"

namespace Ui {
class Client;
}

class Client : public QMainWindow
{
	Q_OBJECT

public:
	explicit Client(QWidget *parent = 0);
	~Client();

private slots:
	void onConnectClicked();
	void onConnected();
	void onError(const QString &text);
	void onMessage(const QString &text);

private:
	Ui::Client *ui;
	ClientListener *mServerListener;
	void printLog(const QString &text);
	void connectSignalsFromServerListener();
};

#endif // CLIENT_H
