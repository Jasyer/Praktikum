#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

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

	QTcpServer *mServer;
	QHostAddress *mHostAddress;

private slots:
	void onSettingsClicked();

};

#endif // SERVER_H
