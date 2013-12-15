#include "server.h"
#include "ui_server.h"
#include "dialogserversettings.h"
#include "stringconstants.h"
#include <QDateTime>
#include <QDebug>
#include "longlibrary.h"
#include "dialogdatabaseitemedit.h"
#include <QTimer>
#include "cryptoconstants.h"
#include "cryptograph.h"

Server::Server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);

	ServerSettings::currentSettings()->setDefaultSettings();

	char c = ui->typeCompany->currentIndex();
	char buf[8];
	derive_key(&c, 1, buf, 8);
	mServerID = Long(buf, 8);
	mServerName = ui->typeCompany->currentText();

	// connections of gui signals
	connect(ui->actionSettings, SIGNAL(triggered()), SLOT(onClickedActionSettings()));
	connect(ui->actionStart, SIGNAL(triggered()), SLOT(onClickedActionStart()));
	connect(ui->actionStop, SIGNAL(triggered()), SLOT(onClickedActionStop()));
	connect(ui->actionAdd_friend_server,
			SIGNAL(triggered()),
			SLOT(onClickedActionAdd_friend_server()));
	connect(ui->actionAddItem, SIGNAL(triggered()), SLOT(onClickedActionDatabaseAddItem()));
	connect(ui->actionRemoveItem, SIGNAL(triggered()), SLOT(onClickedActionDatabaseRemoveItem()));

	// connections of server signals
	connect(&mServer, SIGNAL(acceptError(QAbstractSocket::SocketError)),
			SLOT(onErrorAccepted(QAbstractSocket::SocketError)));
	connect(&mServer, SIGNAL(newConnection()), SLOT(onNewConnection()));

	// mAvailableHashList
	mAvailableHashList.append("SHA256");

	// mAvailableCipherList
	mAvailableCipherList.append("AES256");

	QTimer::singleShot(0, this, SLOT(makePrivatePublicKeys()));
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
	QTime time = QTime::currentTime();
	ui->textServerLog->moveCursor(QTextCursor::End);
	if (!ui->textServerLog->textCursor().atStart())
		ui->textServerLog->insertPlainText("\n");
	ui->textServerLog->insertPlainText(time.toString("[hh:mm:ss.zzz] ") + text);
	ui->textServerLog->moveCursor(QTextCursor::End);
	//ui->textServerLog->scroll(0, ui->textServerLog->contentsRect().size().height() - );
	//	ui->textServerLog->insertHtml("<span style=\" color:#ff0000;\">text</span>");
}

bool Server::loginUser(const Long &hashPIN, const Long &publicKey)
{
	ClientInfo info = mClientsBase.find(hashPIN);
	if (info.isNull())
		return false;
	else
	{ // creating certificate
		QMap<ClientInfo, Certificate>::Iterator it = mCertificates.find(info);
		if (it != mCertificates.end())
		{ // certificate already exists
			Certificate cert = it.value();
			cert.update();
		}
		else
		{ // create new
			QDateTime lifeTime = QDateTime::currentDateTime();
			lifeTime.addMonths(1);
			Certificate cert(mServerID,
							 mServerName,
							 info.clientID(),
							 info.name(),
							 publicKey,
							 lifeTime,
							 mAvailableHashList,
							 mAvailableCipherList
							 );
			Long key = RSA_E;
			key.setModule(RSA_PHI);
			key = Long::getInversed(key);
			cert.signRSA(key);
			cert.signElGamal(key);

			mCertificates.insert(info, cert);
		}
		return true;
	}
}

QList<Certificate> Server::getCertificates()
{
	QList<Certificate> ret;
	QMap<ClientInfo, Certificate>::Iterator it = mCertificates.begin();
	for (int i = 0; i < mCertificates.size(); ++i)
	{
		ret.append(it.value());
		it++;
	}
	return ret;
}

/**
 * @brief Slot for Server->Settings menu
 */
void Server::onClickedActionSettings()
{
	DialogServerSettings dialog;
	if (dialog.exec() == QDialog::Accepted)
	{
		ServerSettings::currentSettings()->set(textServerIP, dialog.getIP());
		ServerSettings::currentSettings()->set(textServerPort, dialog.getPort());
	}
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

void Server::onClickedActionAdd_friend_server()
{
	DialogServerSettings dialog;
	if (dialog.exec() == QDialog::Accepted)
	{
		QTcpSocket *socket = new QTcpSocket;
		anotherServerListener = new ServerListener(socket, mPrivateKey, mPublicKey, this, true);
		connect(anotherServerListener, SIGNAL(deleteMe()), SLOT(onDeleteMe()));
		connect(anotherServerListener,
				SIGNAL(addCertificates(QList<Certificate>)),
				SLOT(onAddCertificates(QList<Certificate>)));
		anotherServerListener->connectToServer(dialog.getIP(), dialog.getPort());
	}
}

void Server::onClickedActionDatabaseAddItem()
{
	DialogDatabaseItemEdit dial;
	connect(&dial,
			SIGNAL(databaseNewItem(QString,Long)),
			SLOT(onDatabaseAddNewItem(QString,Long)));
	dial.exec();
}

void Server::onClickedActionDatabaseRemoveItem()
{

}

void Server::onErrorAccepted(QAbstractSocket::SocketError e)
{
	printLog("Server Error: " + e);
}

void Server::onNewConnection()
{
	QTcpSocket *socket = mServer.nextPendingConnection();
	ServerListener *client = new ServerListener(socket, mPrivateKey, mPublicKey, this);
	mClients.insert(socket->socketDescriptor(), client);
}

void Server::onDatabaseAddNewItem(const QString &name, const Long &hashPIN)
{
	ClientInfo info = ClientInfo(name);
	mClientsBase.add(hashPIN, info);
	ui->tableDatabase->setRowCount(ui->tableDatabase->rowCount() + 1);
	QTableWidgetItem *newItemName = new QTableWidgetItem(name);
	ui->tableDatabase->setItem(ui->tableDatabase->rowCount() - 1, 0, newItemName);
	QTableWidgetItem *newItemHashPIN = new QTableWidgetItem(hashPIN.toString());
	ui->tableDatabase->setItem(ui->tableDatabase->rowCount() - 1, 1, newItemHashPIN);
}

void Server::onDeleteMe()
{
	anotherServerListener->deleteLater();
}

void Server::onAddCertificates(const QList<Certificate> &certificates)
{
	Long key = RSA_E;
	key.setModule(RSA_PHI);
	key = Long::getInversed(key);

	for (int i = 0; i < certificates.count(); ++i)
	{
		Certificate cert = certificates[i];
		cert.signRSA(key);
		cert.signElGamal(key);
		mCertificates.insert(ClientInfo(cert.name(), cert.clientID()), cert);
	}
	printLog("Certificates from another server included in base");
}

void Server::makePrivatePublicKeys()
{
	printLog("Generating private key...");
	// private key
	int random_bits_size = GROUP_PRIME.getSize();
	char *random_bits = new char[random_bits_size];
	drbg_generate(random_bits, random_bits_size);
	mPrivateKey = Long(random_bits, random_bits_size);
	mPrivateKey.setModule(GROUP_PRIME);
	delete []random_bits;

	printLog("Calculating public key...");
	// public key
	mPublicKey = GROUP_GENERATOR;
	mPublicKey.setModule(GROUP_PRIME);
	mPublicKey.pow(mPrivateKey);

	printLog("OK. Private & public keys generated");
}

