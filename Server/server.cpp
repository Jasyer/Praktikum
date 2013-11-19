#include "server.h"
#include "ui_server.h"

Server::Server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);

	/*
	 *	Connections of interface actions
	 */
	connect(ui->actionSettings, SIGNAL(triggered()), SLOT(onSettingsClicked()));

}

Server::~Server()
{
    delete ui;
}

Server::onSettingsClicked()
{

}
