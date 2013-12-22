#include "serverlistener.h"
#include "cryptograph.h"
#include "cryptoconstants.h"
#include "longlibrary.h"
#include <QTcpSocket>
#include <QTextCodec>
#include "commands.h"
#include "server.h"

ServerListener::ServerListener(QTcpSocket *socket, const Long &privateKey, const Long &publicKey,
							   Server *parent, bool connectToServer)
{
	mConnectToServer = connectToServer;
	mParent = parent;
	mSocket = socket;
	mSocketID = socket->socketDescriptor();
	mKeys.privateKey = privateKey;
	mKeys.publicKey = publicKey;

	// connections for socket signals
	connect(mSocket, SIGNAL(disconnected()), SLOT(onDisconnected()));
	connect(mSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
	connect(this, SIGNAL(checkForBytesAvailable()), SLOT(onReadyRead()));

	mData.clear();
	mState.clear();

	if (!mConnectToServer)
	{
		mState.waiting = true;
		mState.waitingType = TYPE_SESSION_KEY;
		mParent->printLog("Waiting session key from client...");
		sendCommand(STATE_READY);
	}
	else
	{
		mState.waiting = true;
		mState.waitingType = STATE_READY;
		makeSessionKey();
	}
}

void ServerListener::connectToServer(const QHostAddress &IP, quint16 port)
{
	if (!mConnectToServer)
		return;
	connect(mSocket, SIGNAL(connected()), SLOT(onConnected()));
	connect(mSocket,
			SIGNAL(error(QAbstractSocket::SocketError)),
			SLOT(onError(QAbstractSocket::SocketError)));

	mConnectToServerAddress = IP.toString() + ":" + QString::number(port);
	mParent->printLog("Connecting to " + mConnectToServerAddress + "...");
	mSocket->connectToHost(IP, port);
}

ServerListener::~ServerListener()
{
	if (mConnectToServer)
		mSocket->deleteLater();
}

void ServerListener::onConnected()
{
	mParent->printLog("OK. Connected to " + mConnectToServerAddress);
}

void ServerListener::onError(const QAbstractSocket::SocketError &e)
{
	switch (e)
	{
	case QAbstractSocket::HostNotFoundError:
		mParent->printLog("Error: " + mConnectToServerAddress + " not found");
		break;
	default:
		mParent->printLog("Error: unknown error");
		break;
	}
	emit deleteMe();
}

void ServerListener::onDisconnected()
{
	if (mConnectToServer)
		emit deleteMe();
	else
	{
		// emit diconnected(socket_id);
	}
}

/**
 * @brief Reading bytes from socket
 *
 * [  size  |  type   |    data      ]
 * [4 bytes | 2 bytes |	(size) bytes ]
 */
void ServerListener::onReadyRead()
{
	if (mData.size < 2)
	{
		if (mSocket->bytesAvailable() < 4)
			return;
		mData.size = readUInt32();
	}

	if (mSocket->bytesAvailable() < mData.size)
		return;

	QByteArray encryptedData = mSocket->read(mData.size);
	QByteArray decryptedData;
	if (mKeys.haveKey)
		decryptData(encryptedData, decryptedData);
	else
		decryptedData = encryptedData;

	mData.size = decryptedData.size();
	mData.type = (((quint8) decryptedData[0]) << 8) | ((quint8) decryptedData[1]);
	mData.data = decryptedData.mid(2);

	if (mConnectToServer)
		parseInputDataFromAnotherServer();
	else
		parseInputData();

	if (mSocket->bytesAvailable() > 0)
		emit checkForBytesAvailable();
}

void ServerListener::sendCommand(quint16 type)
{
	QByteArray null_array;
	sendData(type, null_array);
}

void ServerListener::sendAnswer(quint16 type, quint16 answer)
{
	QByteArray answer_array;
	char buf[2];
	buf[0] = (answer >> 8) & 0xff;
	buf[1] = answer & 0xff;
	answer_array.append(buf, 2);
	sendData(type, answer_array);
}


QString fromInt(int n)
{
	if (n == 0)
		return "0";
	QString ans;
	while (n > 0)
	{
		ans = char((n % 10) + '0') + ans;
		n /= 10;
	}
	return ans;
}

void ServerListener::sendData(quint16 type, const QByteArray &data)
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
		mSocket->write(buf, 4);
	}

	// data
	mSocket->write(encryptedData);
}

void ServerListener::sendText(const QString &text)
{
	sendData(TYPE_TEXT_MESSAGE, QTextCodec::codecForLocale()->fromUnicode(text));
}

/**
 * @brief ServerListener::sendCertificates
 *
 * [ count=N |  size1  | certificate1 |  size2  | certificate2 | ... |  sizeN  | certificateN ]
 * [ 2 bytes | 2 bytes |     ...      | 2 bytes |     ...      | ... | 2 bytes |     ...      ]
 */

void ServerListener::sendCertificates()
{
	QByteArray array;
	QList<Certificate> certs = mParent->getCertificates();

	int count = certs.count();
	{ // count=N
		quint16 N = count;
		char buf[2];
		buf[0] = (N >> 8) & 0xff;
		buf[1] = N & 0xff;
		array.append(buf, 2);
	}

	for (int i = 0; i < count; ++i)
	{
		QByteArray array1 = certs[i].toByteArray();
		// sizeI
		quint16 size1 = array1.size();
		char buf[2];
		buf[0] = (size1 >> 8) & 0xff;
		buf[1] = size1 & 0xff;
		array.append(buf, 2);
		// certificateI
		array.append(array1);
	}
	sendData(TYPE_CERTIFICATES, array);
}

quint32 ServerListener::readUInt32()
{
	quint8 buf[4];
	mSocket->read((char *) buf, 4);
	return ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
}

quint16 ServerListener::readUInt16()
{
	quint8 buf[2];
	mSocket->read((char *) buf, 2);
	return ((buf[0] << 8) | buf[1]);
}

void ServerListener::writeUInt32(quint32 n)
{
	char buf[4];
	buf[3] = n & 0xff;
	n >>= 8;
	buf[2] = n & 0xff;
	n >>= 8;
	buf[1] = n & 0xff;
	n >>= 8;
	buf[0] = n & 0xff;
	mSocket->write(buf, 4);
}

void ServerListener::writeUInt16(quint16 n)
{
	char buf[2];
	buf[1] = n & 0xff;
	n >>=8;
	buf[0] = n & 0xff;
	mSocket->write(buf, 2);
}

void ServerListener::parseInputData()
{
	if (mState.waiting && mData.type != mState.waitingType)
	{
		mData.clear();
		return;
	}
	if (mState.waiting)
	{ // only if waiting
		bool status = false;
		switch(mData.type)
		{
		case TYPE_SESSION_KEY:
			status = parseSessionKey(mData.data);
			break;
		default:
			break;
		}
		if (status)
			mState.clear();
	}
	else
	{ // may be only if not waiting
		switch(mData.type)
		{
		case TYPE_LOGIN:
			parseLogin(mData.data);
			break;
		default:
			break;
		}
	}
	// requests may be either if waiting or if not waiting
	switch(mData.type)
	{
	case REQUEST_CERTIFICATES:
		sendCertificates();
		break;
	default:
		break;
	}
	mData.clear();
}

bool ServerListener::parseSessionKey(const QByteArray &byteArray)
{
	mParent->printLog("OK. Session key recieved");
	mKeys.sessionKey = Long::fromByteArray(byteArray);
	mKeys.sessionKey.setModule(RSA_N);
	mKeys.sessionKey.pow(mKeys.privateKey);
	mKeys.sessionKey.deleteModule();

	mParent->printLog("Calculating AES key...");
	// aes key
	{
		int hash_m_size = mKeys.sessionKey.bytesNeed();
		char *hash_m = new char[hash_m_size];
		mKeys.sessionKey.toChar(hash_m, 0);
		calc_sha256_hash(hash_m, hash_m_size, mKeys.aesKey, 0);
		delete []hash_m;
	}
	mParent->printLog("OK. AES key calculated");

	sendCommand(STATE_OK);
	mParent->printLog("OK. Secure conection is established");
	mKeys.haveKey = true;
	mParent->printLog("Key = " + mKeys.sessionKey.toString());
//	mParent->printLog("Public = " + mKeys.publicKey.toString());
//	mParent->printLog("Private = " + mKeys.privateKey.toString());
	return true;
}

void ServerListener::parseLogin(const QByteArray &byteArray)
{
	Long hashPIN = Long::fromByteArray(byteArray);
	if (mParent->loginUser(hashPIN))
	{
		sendAnswer(TYPE_LOGIN_STATUS, ANSWER_LOGIN_OK);
		mParent->printLog("ANSWER: OK");
	}
	else
	{
		sendAnswer(TYPE_LOGIN_STATUS, ANSWER_LOGIN_FAIL);
		mParent->printLog("ANSWER: FAIL");
	}
}

void ServerListener::parseInputDataFromAnotherServer()
{
	if (!mState.waiting || mData.type != mState.waitingType)
	{
		mData.clear();
		return;
	}
	switch(mData.type)
	{
	case STATE_READY:
		mState.waiting = true;
		mState.waitingType = STATE_OK;
		sendSessionKey();
		break;
	case STATE_OK:
		mParent->printLog("OK. Secure connection is established");
		mKeys.haveKey = true;
		mState.waiting = true;
		mState.waitingType = TYPE_CERTIFICATES;
		mParent->printLog("Sending request for certificates...");
		sendCommand(REQUEST_CERTIFICATES);
		break;
	case TYPE_CERTIFICATES:
		mParent->printLog("OK. Certificates recieved");
		parseCertificates(mData.data);
		mState.clear();
		emit deleteMe();
		break;
	default:
		break;
	}
	mData.clear();
}

void ServerListener::makeSessionKey()
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

void ServerListener::sendSessionKey()
{
	mParent->printLog("Sending common key to server...");
	Long ciph = mKeys.sessionKey;
	ciph.pow(mKeys.publicKey);
	ciph.deleteModule(); // IMPORTANT!
	sendData(TYPE_SESSION_KEY, ciph.toByteArray());
	mParent->printLog("Waiting for answer...");
}

void ServerListener::parseCertificates(const QByteArray &byteArray)
{
	QList<Certificate> certificates;

	int index = 0;
	if (index + 2 > byteArray.size())
		return; // TODO: error

	int count = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
	index += 2;

	for (int i = 0; i < count; ++i)
	{
		QByteArray arr1;

		if (index + 2 > byteArray.size())
			return; // TODO: error

		int size1 = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size1 > byteArray.size())
			return; // TODO: error

		arr1 = byteArray.mid(index, size1);
		index += size1;
		Certificate cert = Certificate::fromByteArray(arr1);
		certificates.append(cert);
	}
	emit addCertificates(certificates);
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

void ServerListener::encryptData(const QByteArray &input, QByteArray &output)
{
	qDebug() << "Server Encrypt Input:";
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

	qDebug() << "Server Encrypt Output:";
	printByteArray(output);
}

void ServerListener::decryptData(const QByteArray &input, QByteArray &output)
{
	qDebug() << "Server Decrypt Input:";
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

	qDebug() << "Server Decrypt Output:";
	printByteArray(output);
}
