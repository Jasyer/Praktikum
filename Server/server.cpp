#include "server.h"
#include "ui_server.h"
#include "dialogserversettings.h"
#include "stringconstants.h"
#include <QDateTime>
#include <QDebug>

Server::Server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);

	ServerSettings::currentSettings()->setDefaultSettings();

	// connections of gui signals
	connect(ui->actionSettings, SIGNAL(triggered()), SLOT(onClickedActionSettings()));
	connect(ui->actionStart, SIGNAL(triggered()), SLOT(onClickedActionStart()));
	connect(ui->actionStop, SIGNAL(triggered()), SLOT(onClickedActionStop()));

	// connections of server signals
	connect(&mServer, SIGNAL(acceptError(QAbstractSocket::SocketError)),
			SLOT(onErrorAccepted(QAbstractSocket::SocketError)));
}

Server::~Server()
{
	delete ui;
}

/**
 * @brief Printing log message
 * @param text Printing text
 */
void Server::printLog(const QString &text)
{
	QDateTime time = QDateTime::currentDateTime();
	ui->textServerLog->insertPlainText("[" + time.toString("hh:mm:ss:zzz") + "] " + text + "\n");
//	ui->textServerLog->insertHtml("<span style=\" color:#ff0000;\">text</span>");
}

/**
 * @brief Slot for Server->Settings menu
 */
void Server::onClickedActionSettings()
{
	DialogServerSettings dialog;
	dialog.exec();
}

/**
 * @brief Slot for Server->Start menu
 */
void Server::onClickedActionStart()
{
	QString servIP = M_SETTING(textServerIP);
	QString servPort = M_SETTING(textServerPort);
	printLog("Starting server at " + servIP + ":" + servPort + "...");

	if (mServer.listen(QHostAddress(servIP), (quint16) servPort.toInt()))
		printLog("Success");

}

/**
 * @brief Slot for Server->Stop menu
 */
void Server::onClickedActionStop()
{
	mServer.close();
	printLog("Server stopped");
}

void Server::onErrorAccepted(QAbstractSocket::SocketError e)
{
	printLog("Server Error: " + e);
}
