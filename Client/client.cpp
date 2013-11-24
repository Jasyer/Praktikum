#include "client.h"
#include "ui_client.h"
#include <QDateTime>

Client::Client(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::Client)
{
	ui->setupUi(this);
	mServerListener = NULL;

	connect(ui->buttonConnect, SIGNAL(clicked()), SLOT(onConnectClicked()));
}

Client::~Client()
{
	delete ui;
}

void Client::printLog(const QString &text)
{
	QString time = QTime::currentTime().toString("hh:mm:ss");
	ui->textClientLog->insertPlainText("[" + time + "] " + text + "\n");
}

void Client::connectSignalsFromServerListener()
{
	connect(mServerListener, SIGNAL(connected()), SLOT(onConnected()));
	connect(mServerListener, SIGNAL(error(QString)), SLOT(onError(QString)));
	connect(mServerListener, SIGNAL(message(QString)), SLOT(onMessage(QString)));
}

void Client::onConnectClicked()
{
	ui->textClientLog->moveCursor(QTextCursor::End);
	printLog("Connecting to " + ui->textIP->text() + ":" + ui->textPort->text() + "...");
	mServerListener = new ClientListener;
	connectSignalsFromServerListener();
	mServerListener->connectToHost(ui->textIP->text(), ui->textPort->text());
}

void Client::onConnected()
{
	printLog("Success");
}

void Client::onError(const QString &text)
{
	printLog("Error: " + text);
}

void Client::onMessage(const QString &text)
{
	printLog("Server: " + text);
}
