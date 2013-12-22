#include "server.h"
#include "ui_server.h"
#include "dialogserversettings.h"
#include "stringconstants.h"
#include <QDateTime>
#include <QDebug>
#include "longlibrary.h"
#include "dialogdatabaseitemedit.h"
#include <QTimer>
#include <QList>
#include "cryptoconstants.h"
#include "cryptograph.h"
#include "commands.h"

Server::Server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);

	// connections of gui signals
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

	choiceServer();
	this->setWindowTitle(mServerName + " Server");
	QTimer::singleShot(0, this, SLOT(loopUpdateCertificateList()));
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

bool Server::loginUser(const Long &hashPIN)
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
			QDateTime lifeTime = QDateTime::currentDateTime().addDays(1);
			Certificate cert(mServerID,
							 mServerName,
							 info.clientID(),
							 info.name(),
							 mPublicKey,
							 lifeTime,
							 mAvailableHashList,
							 mAvailableCipherList
							 );
			cert.signRSA(mPrivateKey);
			cert.signElGamal(mPrivateKey);

			mCertificates.insert(info, cert);
			mListCertValid.append(info);
//			updateCertificateList();
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

void Server::choiceServer()
{
	DialogServerSettings dial;
	while (true)
	{
		if (dial.exec() == QDialog::Rejected)
			exit(0);
		else
		{
			mServerType = dial.getServerType();
			char buf[8];
			derive_key((char *) &mServerType, 1, buf, 8);
			mServerID = Long(buf, 8);
			switch (mServerType)
			{
			case SERVER_FEDERAL_AGENCY:
				mServerIP = ipFederalAgency;
				mServerName = dial.getServerName();
				mServerPort = portFederalAgency;
				mPublicKey = RSA_E_FEDERAL_AGENCY;
				mPrivateKey = RSA_D_FEDERAL_AGENCY;
				return;
			case SERVER_MINISTRY:
				mServerIP = ipMinistry;
				mServerName = dial.getServerName();
				mServerPort = portMinistry;
				mPublicKey = RSA_E_MINISTRY;
				mPrivateKey = RSA_D_MINISTRY;
				return;
			default:
				break;
			}
		}
	}
}

void Server::updateCertificateList()
{
	// update list of valid/invoked certificates
	QList<ClientInfo>::Iterator it = mListCertValid.begin();
	while(it != mListCertValid.end())
	{
		Certificate &cert = mCertificates[*it];
		cert.update();
		if (cert.invoked())
		{
			mListCertInvoked.append(*it);
			it = mListCertValid.erase(it);
		}
		else
			it++;
	}
	// update gui
	ui->listCertValid->clear();
	ui->certificates->setTabText(0, "Valid(" + QString::number(mListCertValid.size()) + ")");
	it = mListCertValid.begin();
	for (; it != mListCertValid.end(); ++it)
		ui->listCertValid->addItem(mCertificates[*it].name());

	ui->listCertInvoked->clear();
	ui->certificates->setTabText(1, "Invoked(" + QString::number(mListCertInvoked.size()) + ")");
	it = mListCertInvoked.begin();
	for (; it != mListCertInvoked.end(); ++it)
		ui->listCertInvoked->addItem(mCertificates[*it].name());
}

/**
 * @brief Slot for Server->Start menu
 */
void Server::onClickedActionStart()
{
	QString address = mServerIP.toString() + ":" + QString::number(mServerPort);
	printLog("Starting server at " + address + "...");

	if (mServer.listen(mServerIP, mServerPort))
		printLog("OK. Started server on " + address);
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
	DialogServerSettings dialog(mServerType);
	if (dialog.exec() == QDialog::Accepted)
	{
		QHostAddress srvIP;
		quint16 srvPort;
		Long publicKey;
		switch (dialog.getServerType())
		{
		case SERVER_FEDERAL_AGENCY:
			srvIP = ipFederalAgency;
			srvPort = portFederalAgency;
			publicKey = RSA_E_FEDERAL_AGENCY;
			break;
		case SERVER_MINISTRY:
			srvIP = ipMinistry;
			srvPort = portMinistry;
			publicKey = RSA_E_MINISTRY;
			break;
		}
		anotherServerListener = new ServerListener(new QTcpSocket, Long(), publicKey, this, true);
		connect(anotherServerListener, SIGNAL(deleteMe()), SLOT(onDeleteMe()));
		connect(anotherServerListener,
				SIGNAL(addCertificates(QList<Certificate>)),
				SLOT(onAddCertificates(QList<Certificate>)));
		anotherServerListener->connectToServer(srvIP, srvPort);
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
	Long key = mPrivateKey;
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

void Server::makePrivateKey()
{
	printLog("Calculating private key...");

	mPrivateKey = mPublicKey;
	mPrivateKey.setModule(RSA_PHI);
	mPrivateKey = Long::getInversed(mPrivateKey);

	printLog("OK. Private key generated");
}

void Server::loopUpdateCertificateList()
{
	updateCertificateList();
	QTimer::singleShot(10000, this, SLOT(loopUpdateCertificateList()));
}

