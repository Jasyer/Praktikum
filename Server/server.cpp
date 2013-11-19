#include "server.h"
#include "ui_server.h"
#include "dialogserversettings.h"

Server::Server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);

	/*
	 *	Connections of interface actions
	 */
	connect(ui->actionSettings, SIGNAL(triggered()), SLOT(onClickedActionSettings()));

}

Server::~Server()
{
    delete ui;
}

void Server::onClickedActionSettings()
{
	DialogServerSettings dialog;
	dialog.exec();
}

void Server::onClickedActionStart()
{

}

void Server::onClickedActionStop()
{

}
