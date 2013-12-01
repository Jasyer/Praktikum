#include "client.h"
#include "ui_client.h"
#include <QDateTime>
#include "cryptograph.h"
#include "cryptoconstants.h"
#include "longlibrary.h"
#include <QTextCodec>
#include <QTimer>

Client::Client(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::Client)
{
	ui->setupUi(this);
	mClientListener = NULL;

	connect(ui->buttonConnect, SIGNAL(clicked()), SLOT(onButtonConnectClicked()));
	connect(ui->buttonLogIn, SIGNAL(clicked()), SLOT(onButtonLogInClicked()));

	QTimer::singleShot(0, this, SLOT(makePrivatePublicKey()));
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
	ui->textClientLog->moveCursor(QTextCursor::End);
}

void Client::connectSignalsFromServerListener()
{
	connect(mClientListener, SIGNAL(connected()), SLOT(onConnected()));
	connect(mClientListener, SIGNAL(error(QString)), SLOT(onError(QString)));
	connect(mClientListener, SIGNAL(message(QString)), SLOT(onMessage(QString)));
}

void Client::makePrivatePublicKey()
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

void Client::onButtonConnectClicked()
{
	ui->textClientLog->moveCursor(QTextCursor::End);
	printLog("Connecting to " + ui->textIP->text() + ":" + ui->textPort->text() + "...");
	mClientListener = new ClientListener(mPrivateKey, mPublicKey, this);
	connectSignalsFromServerListener();
	mClientListener->connectToHost(ui->textIP->text(), ui->textPort->text());
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
