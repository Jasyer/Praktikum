#include "client.h"
#include "ui_client.h"
#include <QDateTime>
#include "cryptograph.h"
#include "cryptoconstants.h"
#include "longlibrary.h"
#include "stringconstants.h"
#include "commands.h"
#include "certificateviewdialog.h"
#include "certificate.h"
#include <QTextCodec>
#include <QTimer>
#include <QListWidget>

Client::Client(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::Client)
{
	ui->setupUi(this);
	mClientListener = NULL;

	loadCompanyList();

	connect(ui->buttonConnect, SIGNAL(clicked()), SLOT(onButtonConnectClicked()));
	connect(ui->buttonLogIn, SIGNAL(clicked()), SLOT(onButtonLogInClicked()));
	connect(ui->buttonDisconnect, SIGNAL(clicked()), SLOT(onButtonDisconnectClicked()));
	connect(ui->buttonGetCertificates, SIGNAL(clicked()), SLOT(onButtonGetCertificatesClicked()));
	connect(ui->typeCompany, SIGNAL(activated(int)), SLOT(onTypeCompanyActivated(int)));

	onTypeCompanyActivated(0);

	connectContextMenuSlots();
}

Client::~Client()
{
	delete ui;
}

void Client::printLog(const QString &text)
{
	QTime time = QTime::currentTime();
	ui->textClientLog->moveCursor(QTextCursor::End);
	if (!ui->textClientLog->textCursor().atStart())
		ui->textClientLog->insertPlainText("\n");
	ui->textClientLog->insertPlainText(time.toString("[hh:mm:ss.zzz] ") + text);
//	ui->textClientLog->insertHtml("<pre>" + time.toString("[hh:mm:ss.zzz] ") + text + "</pre>");
	ui->textClientLog->moveCursor(QTextCursor::End);
}

void Client::connectSignalsFromServerListener()
{
	connect(mClientListener,
			SIGNAL(connected()),
			SLOT(onConnected()));
	connect(mClientListener,
			SIGNAL(error(QString)),
			SLOT(onError(QString)));
	connect(mClientListener,
			SIGNAL(recievedCertificates(QList<Certificate>)),
			SLOT(onRecievedCertificates(QList<Certificate>)));
}

void Client::loadCompanyList()
{
	ui->typeCompany->clear();
	ui->typeCompany->addItem(textServerNameFederalAgency);
	ui->typeCompany->addItem(textServerNameMinistry);
	ui->typeCompany->setCurrentIndex(0);
}

void Client::connectContextMenuSlots()
{
	ui->listCertValid->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->listCertValid,
			SIGNAL(customContextMenuRequested(QPoint)),
			SLOT(contextMenuListCertValid()));
	ui->listCertInvoked->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->listCertInvoked,
			SIGNAL(customContextMenuRequested(QPoint)),
			SLOT(contextMenuListCertInvoked()));
}

void Client::onButtonConnectClicked()
{
	ui->buttonConnect->setEnabled(false);
	ui->typeCompany->setEnabled(false);

	mClientListener = new ClientListener(mServerPublicKey, this);
	connectSignalsFromServerListener();
	mClientListener->connectToHost(mServerIP, mServerPort);
}

void Client::onButtonDisconnectClicked()
{
	ui->buttonDisconnect->setEnabled(false);

	mClientListener->disconnectFromHost();
	delete mClientListener;

	ui->buttonConnect->setEnabled(true);
	ui->typeCompany->setEnabled(true);
}

#include <QInputDialog>

void Client::onButtonLogInClicked()
{
	QString PIN_str = QInputDialog().getText(this, "Log In to server", "Enter your PIN",
											 QLineEdit::Password);
	QByteArray PIN_uni = QTextCodec::codecForLocale()->fromUnicode(PIN_str);
	int hash_m_size = PIN_uni.size();
	char *hash_m = new char[hash_m_size];
	for (int i = 0; i < hash_m_size; ++i)
		hash_m[i] = PIN_uni[i];
	char hash[32];
	calc_sha256_hash(hash_m, hash_m_size, hash, 0);
	delete []hash_m;
	Long PIN(hash, 32);
	mClientListener->login(PIN);
}

void Client::onButtonGetCertificatesClicked()
{
	mClientListener->getCertificates();
}

void Client::onTypeCompanyActivated(int index)
{
	switch(index)
	{
	case 0:
		mServerPublicKey = RSA_E_FEDERAL_AGENCY;
		mServerIP = ipFederalAgency;
		mServerPort = portFederalAgency;
		break;
	case 1:
		mServerPublicKey = RSA_E_MINISTRY;
		mServerIP = ipMinistry;
		mServerPort = portMinistry;
		break;
	default:
		mServerPublicKey = RSA_E_FEDERAL_AGENCY;
		mServerIP = ipFederalAgency;
		mServerPort = portFederalAgency;
		break;
	}
}

void Client::contextMenuListCertValid()
{
	if (ui->listCertValid->selectedItems().empty())
		return;
	QMenu menu;
	menu.addAction("View", this, SLOT(onClickedActionCertView()));
	menu.exec(QCursor::pos());
}

void Client::contextMenuListCertInvoked()
{
	if (ui->listCertInvoked->selectedItems().empty())
		return;
	QMenu menu;
	menu.addAction("View", this, SLOT(onClickedActionCertView()));
	menu.exec(QCursor::pos());
}

void Client::onClickedActionCertView()
{
	Certificate current;
	switch(ui->certificates->currentIndex())
	{
	case 0:
		// Valid tab
		current = mCertificatesValid[ui->listCertValid->currentRow()];
		break;
	case 1:
		// Invoked tab
		current = mCertificatesInvoked[ui->listCertInvoked->currentRow()];
		break;
	default:
		return;
	}
	CertificateViewDialog dialog(current);
	dialog.exec();
}

void Client::onConnected()
{
	printLog("OK. Connected");

	ui->buttonDisconnect->setEnabled(true);
}
void Client::onError(const QString &text)
{
	printLog("Error: " + text);
	ui->buttonConnect->setEnabled(true);
	ui->buttonDisconnect->setEnabled(false);
	ui->typeCompany->setEnabled(true);
}

void Client::onRecievedCertificates(const QList<Certificate> &list)
{
	ui->listCertValid->clear();
	ui->listCertInvoked->clear();
	mCertificatesValid.clear();
	mCertificatesInvoked.clear();
	for (int i = 0; i < list.count(); ++i)
	{
		Certificate cert = list[i];
		QString line = cert.name();
		if (cert.invoked())
		{
			ui->listCertInvoked->insertItem(ui->listCertInvoked->count(), line);
			mCertificatesInvoked.append(cert);
		}
		else
		{
			ui->listCertValid->insertItem(ui->listCertValid->count(), line);
			mCertificatesValid.append(cert);
		}
	}
	ui->certificates->setTabText(0, "Valid(" +
								 QString::number(mCertificatesValid.count()) + ")");
	ui->certificates->setTabText(1, "Invoked(" +
								 QString::number(mCertificatesInvoked.count()) + ")");
}
