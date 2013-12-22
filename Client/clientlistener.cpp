#include "clientlistener.h"
#include <QHostAddress>
#include "commands.h"
#include "cryptoconstants.h"
#include "cryptograph.h"
#include "longlibrary.h"
#include <QTextCodec>
#include <QDebug>
#include "client.h"
#include "certificate.h"

ClientListener::ClientListener(const Long &serverPublicKey, Client *parent)
{
	mData.clear();
	mKeys.serverPublicKey = serverPublicKey;
	mParent = parent;
	mState.clear();
}

void ClientListener::connectToHost(const QHostAddress &IP, quint16 port)
{
	makeSessionKey();

	connect(&mSocket, SIGNAL(connected()), SLOT(onConnected()));
	connect(&mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			SLOT(onError(QAbstractSocket::SocketError)));
	connect(&mSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
	connect(this, SIGNAL(recheckForReadyRead()), SLOT(onReadyRead()));

	mAddressString = IP.toString() + ":" + QString::number(port);
	mParent->printLog("Connecting to " + mAddressString + "...");
	mSocket.connectToHost(IP, port);

	mState.waiting = true;
	mState.waitingType = STATE_READY;
}

void ClientListener::disconnectFromHost()
{
	mSocket.close();
	mParent->printLog("OK. Disconnected");
}

void ClientListener::login(const Long &hashPIN)
{
	mParent->printLog("Try to login...");
	mState.waiting = true;
	mState.waitingType = TYPE_LOGIN_STATUS;
	sendData(TYPE_LOGIN, hashPIN.toByteArray());
}

void ClientListener::getCertificates()
{
	mState.waiting = true;
	mState.waitingType = TYPE_CERTIFICATES;
	sendRequest(REQUEST_CERTIFICATES);
	mParent->printLog("Sended request for certificates");
}

void ClientListener::onConnected()
{
	emit connected();
}

void ClientListener::onError(QAbstractSocket::SocketError e)
{
	switch(e)
	{
	case QAbstractSocket::ConnectionRefusedError:
		emit error("Connection refused by host");
		break;
	case QAbstractSocket::RemoteHostClosedError:
		emit error("Host closed connection");
		break;
	case QAbstractSocket::HostNotFoundError:
		emit error("Not found " + mAddressString);
		break;
	default:
		emit error("Unknown error");
		break;
	}
}

/**
 * @brief ServerListener::onReadyRead
 *
 * Server message structure:
 *
 * [         4 bytes           | 2 bytes |	         ]
 * [ size of command + message | command | message   ]
 */

void ClientListener::onReadyRead()
{
	if (mData.size < 2) // need 2 bytes for type
	{
		if (mSocket.bytesAvailable() < 4)
			return;
		mData.size = readUInt32();
	}

	if (mSocket.bytesAvailable() < mData.size)
		return;

	QByteArray encryptedData = mSocket.read(mData.size);
	QByteArray decryptedData;
	if (mKeys.haveKey)
		decryptData(encryptedData, decryptedData);
	else
		decryptedData = encryptedData;

	mData.size = decryptedData.size();
	mData.type = (((quint8) decryptedData[0]) << 8) | ((quint8) decryptedData[1]);
	mData.data = decryptedData.mid(2);

	parseInputData();

	if (mSocket.bytesAvailable() > 0)
		emit recheckForReadyRead();
}

void ClientListener::sendData(quint16 type, const QByteArray &data)
{
	QByteArray sourceData;
	{
		char buf[2];
		buf[0] = (type >> 8) & 0xff;
		buf[1] = type & 0xff;
		sourceData.append(buf, 2);
		sourceData.append(data);
	}

	// encrypting
	QByteArray encryptedData;
	if (mKeys.haveKey)
		encryptData(sourceData, encryptedData);
	else
		encryptedData = sourceData;

	// size
	{
		quint32 size = encryptedData.size();
		char buf[4];
		buf[0] = (size >> 24) & 0xff;
		buf[1] = (size >> 16) & 0xff;
		buf[2] = (size >> 8) & 0xff;
		buf[3] = size & 0xff;
		mSocket.write(buf, 4);
	}

	// data
	mSocket.write(encryptedData);
}

void ClientListener::sendRequest(quint16 type)
{
	sendData(type, QByteArray()); // empty array
}

void ClientListener::sendSessionKey()
{
	mParent->printLog("Sending common key to server...");
	Long ciph = mKeys.sessionKey;
	ciph.pow(mKeys.serverPublicKey);
	ciph.deleteModule(); // IMPORTANT!
	sendData(TYPE_SESSION_KEY, ciph.toByteArray());
	mParent->printLog("Waiting for answer...");
}

quint32 ClientListener::readUInt32()
{
	quint8 buf[4];
	mSocket.read((char *) buf, 4);
	return ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
}
quint16 ClientListener::readUInt16()
{
	quint8 buf[2];
	mSocket.read((char *) buf, 2);
	return ((buf[0] << 8) | buf[1]);
}

/**
 * @brief ServerListener::parseInputData
 *
 * Server command types:
 *
 */
void ClientListener::parseInputData()
{
	if (mState.waiting && mData.type != mState.waitingType)
	{
		mData.clear();
		return;
	}
	if (mState.waiting)
	{ // waiting types
		bool status = false;
		switch(mData.type)
		{
		case STATE_READY:
			status = parseStateReady();
			break;
		case STATE_OK:
			status = parseStateOK();
			break;
		case TYPE_LOGIN_STATUS:
			status = parseLoginStatus(mData.data);
			break;
		case TYPE_CERTIFICATES:
			status = parseCertificates(mData.data);
			break;
		default:
			break;
		}
		if (status)
			mState.clear();
		else
		{
			mData.clear();
			return;
		}
	}
	switch(mData.type)
	{
	case TYPE_TEXT_MESSAGE:
		parseTextMessage(QTextCodec::codecForLocale()->toUnicode(mData.data));
		break;
	}
	mData.clear();
}

bool ClientListener::parseStateReady()
{
	mState.waiting = true;
	mState.waitingType = STATE_OK;
	sendSessionKey();
	return false;
}

bool ClientListener::parseStateOK()
{
	mParent->printLog("OK. Secure connection is established");
	mKeys.haveKey = true;
	mParent->printLog("Key = " + mKeys.sessionKey.toString());
//	mParent->printLog("Public = " + mKeys.serverPublicKey.toString());
	return true;
}

bool ClientListener::parseTextMessage(const QString &text)
{
	mParent->printLog("Server: " + text);
	return true;
}

/**
 * @brief ClientListener::parseLoginStatus
 *
 * [ answer  ]
 * [ 2 bytes ]
 *
 * Answer may be {ANSWER_LOGIN_OK, ANSWER_LOGIN_FAIL}
 */
bool ClientListener::parseLoginStatus(const QByteArray &byteArray)
{
	if (byteArray.size() != 2)
		return false;

	quint16 answer = ((byteArray[0] & 0xff) << 8) | (byteArray[1] & 0xff);
	switch(answer)
	{
	case ANSWER_LOGIN_OK:
		mParent->printLog("OK. Successful loginned");
		break;
	case ANSWER_LOGIN_FAIL:
		mParent->printLog("FAIL. Invalid PIN");
		break;
	default:
		mParent->printLog("FAIL. Unknown error");
		break;
	}
	return true;
}

/**
 * @brief ClientListener::parseCertificates
 *
 * [    N    |  size1  | certificate1 |  size2  | certificate2 | ... |  sizeN  | certificateN ]
 * [ 2 bytes | 2 bytes |     ...      | 2 bytes |     ...      | ... | 2 bytes |     ...      ]
 */
bool ClientListener::parseCertificates(const QByteArray &byteArray)
{
	QList<Certificate> certificates;

	int index = 0;
	if (index + 2 > byteArray.size())
		return false; // TODO: error

	int count = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
	index += 2;

	for (int i = 0; i < count; ++i)
	{
		QByteArray arr1;

		if (index + 2 > byteArray.size())
			return false; // TODO: error

		int size1 = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size1 > byteArray.size())
			return false; // TODO: error

		arr1 = byteArray.mid(index, size1);
		index += size1;
		Certificate cert = Certificate::fromByteArray(arr1);
		certificates.append(cert);
	}
	emit recievedCertificates(certificates);
	mParent->printLog("Recieved certificates");
	return true;
}

void ClientListener::makeSessionKey()
{
	mParent->printLog("Generating session key...");
	// private key
	int random_bits_size = RSA_N.getSize();
	char *random_bits = new char[random_bits_size];
	drbg_generate(random_bits, random_bits_size);
	mKeys.sessionKey = Long(random_bits, random_bits_size);
	mKeys.sessionKey.setModule(RSA_N);
	delete []random_bits;

	mParent->printLog("Calculating AES key...");
	// aes key
	{
		int hash_m_size = mKeys.sessionKey.bytesNeed();
		char *hash_m = new char[hash_m_size];
		mKeys.sessionKey.toChar(hash_m, 0);
		calc_sha256_hash(hash_m, hash_m_size, mKeys.aesKey, 0);
		delete []hash_m;
	}

	mParent->printLog("OK. Session & AES keys generated");
}

QString toHex(quint8 x)
{
	QString ret;
	if ((x & 0xf0) > 0x90)
		ret += (char) (((x >> 4) & 0x0f) - 0x0a + 'a');
	else
		ret += (char) (((x >> 4) & 0x0f) + '0');

	if ((x & 0x0f) > 0x09)
		ret += (char) ((x & 0x0f) - 0x0a + 'a');
	else
		ret += (char) ((x & 0x0f) + '0');
	return ret;
}

void printByteArray(const QByteArray &byteArray)
{
	QString buf = "";
	for (int i = 0; i < byteArray.size(); ++i)
	{
		buf += toHex(byteArray[i]);
		if (((i + 1) & 0xf) == 0)
		{
			qDebug() << buf;
			buf = "";
		}
	}
	if (buf != "")
		qDebug() << buf;
}

void ClientListener::encryptData(const QByteArray &input, QByteArray &output)
{
	qDebug() << "Client Encrypt Input:";
	printByteArray(input);

	int in_size = input.size();
	char *in = new char[in_size];
	for (int i = 0; i < in_size; ++i)
		in[i] = input[i];
	char iv[16] = {0};

	int out_size;
	encrypt_aes256_cbc(in, in_size, mKeys.aesKey, 0, &out_size, iv, 16);
	char *out = new char[out_size];
	encrypt_aes256_cbc(in, in_size, mKeys.aesKey, out, &out_size, iv, 16);

	output.clear();
	output.append(out, out_size);
	delete []in;
	delete []out;

	qDebug() << "Client Encrypt Output:";
	printByteArray(output);
}

void ClientListener::decryptData(const QByteArray &input, QByteArray &output)
{
	qDebug() << "Client Decrypt Input:";
	printByteArray(input);

	int in_size = input.size();
	char *in = new char[in_size];
	for (int i = 0; i < in_size; ++i)
		in[i] = input[i];
	char iv[16] = {0};

	int out_size;
	decrypt_aes256_cbc(in, in_size, mKeys.aesKey, 0, &out_size, iv, 16);
	char *out = new char[out_size];
	decrypt_aes256_cbc(in, in_size, mKeys.aesKey, out, &out_size, iv, 16);

	output.clear();
	output.append(out, out_size);
	delete []in;
	delete []out;

	qDebug() << "Client Dencrypt Output:";
	printByteArray(output);
}


