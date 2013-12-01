#include "clientlistener.h"
#include <QHostAddress>
#include "commands.h"
#include "cryptoconstants.h"
#include "cryptograph.h"
#include "longlibrary.h"
#include <QTextCodec>
#include <QDebug>
#include "client.h"

ClientListener::ClientListener(const Long &privateKey, const Long &publicKey, Client *parent)
{
	mData.clear();
	mKeys.privateKey = privateKey;
	mKeys.publicKey = publicKey;
	mParent = parent;
	mState.clear();
}

void ClientListener::connectToHost(const QString &IP, const QString &port)
{
	makeSessionKeys();

	connect(&mSocket, SIGNAL(connected()), SLOT(onConnected()));
	connect(&mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			SLOT(onError(QAbstractSocket::SocketError)));
	connect(&mSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
	connect(this, SIGNAL(recheckForReadyRead()), SLOT(onReadyRead()));

	mAddress = IP + ":" + port;
	mParent->printLog("Connecting to " + mAddress + "...");
	mSocket.connectToHost((QHostAddress) IP, (quint16) port.toInt());

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
	mState.waiting = true;
	mState.waitingType = TYPE_LOGIN_STATUS;
	sendData(TYPE_LOGIN, hashPIN.toByteArray());
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
		emit error("Not found " + mAddress);
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

	mData.type = readUInt16();
	mData.size -= 2;

	mData.data = mSocket.read(mData.size);

	parseInputData();

	if (mSocket.bytesAvailable() > 0)
		emit recheckForReadyRead();
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

void ClientListener::sendData(quint16 type, const QByteArray &data)
{
	//mParent->printLog("SENDING " + fromInt(type));

	// size section
	{
		quint32 size = 2 + data.size(); // 2 bytes for type
		char buf[4];
		buf[0] = (size >> 24) & 0xff;
		buf[1] = (size >> 16) & 0xff;
		buf[2] = (size >> 8) & 0xff;
		buf[3] = size & 0xff;
		mSocket.write(buf, 4);
	}

	// type section
	{
		char buf[2];
		buf[0] = (type >> 8) & 0xff;
		buf[1] = type & 0xff;
		mSocket.write(buf, 2);
	}

	// data section
	mSocket.write(data);
}

void ClientListener::sendRequest(quint16 type)
{
	sendData(type, QByteArray()); // empty array
}

void ClientListener::sendPublicKeys()
{
	QByteArray keys_block;

	// public_key_size
	QByteArray public_key_array = mKeys.publicKey.toByteArray();
	mParent->printLog("Sending public key: " + mKeys.publicKey.toString());
	{
		quint16 public_key_size = public_key_array.size();
		char buf[2];
		buf[0] = (public_key_size >> 8) & 0xff;
		buf[1] = public_key_size & 0xff;
		keys_block.append(buf, 2);
	}

	// public_session_key_size
	mParent->printLog("Sending session public key: " + mKeys.publicSessionKey.toString());
	QByteArray public_session_key_array = mKeys.publicSessionKey.toByteArray();
	{
		quint16 public_session_key_size = public_session_key_array.size();
		char buf[2];
		buf[0] = (public_session_key_size >> 8) & 0xff;
		buf[1] = public_session_key_size & 0xff;
		keys_block.append(buf, 2);
	}

	// public_key
	keys_block.append(public_key_array);

	// public_session_key
	keys_block.append(public_session_key_array);

	sendData(TYPE_PUBLIC_KEYS, keys_block);
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
		case TYPE_PUBLIC_KEYS:
			status = parsePublicKeys(mData.data);
			break;
		case STATE_READY:
			status = parseStateReady();
			break;
		case TYPE_LOGIN_STATUS:
			status = parseLoginStatus(mData.data);
			break;
		default:
			break;
		}
		if (status)
			mState.clear();
	}
	switch(mData.type)
	{
	case REQUEST_PUBLIC_KEYS:
		sendPublicKeys();
		break;
	case TYPE_TEXT_MESSAGE:
		parseTextMessage(QTextCodec::codecForLocale()->toUnicode(mData.data));
		break;
	}
	mData.clear();
}

bool ClientListener::parseStateReady()
{
	mState.waiting = true;
	mState.waitingType = TYPE_PUBLIC_KEYS;
	sendRequest(REQUEST_PUBLIC_KEYS);
	return false;
}

bool ClientListener::parseTextMessage(const QString &text)
{
	emit message(text);
	return true;
}

/**
 * @brief ClientListener::parsePublicKeys
 * @param keys
 *
 * Keys block:
 * [ public_key_size | public_session_key_size | public_key | public_session_key ]
 * [    2 bytes      |        2 bytes          |            |                    ]
 */
bool ClientListener::parsePublicKeys(const QByteArray &byteArray)
{
	// public_key_size section
	quint16 public_key_size;
	public_key_size = byteArray[0];
	public_key_size <<= 8;
	public_key_size |= byteArray[1];

	// public_session_key_size section
	quint16 public_session_key_size;
	public_session_key_size = byteArray[2];
	public_session_key_size <<= 8;
	public_session_key_size |= byteArray[3];

	// check integrity
	if (public_session_key_size + public_key_size + 4 != byteArray.size())
	{
		qDebug() << "Incorrect keys message";
		return false;
	}

	// public_key section
	{
		int public_key_index = 4;
		QByteArray public_key_array = byteArray.mid(public_key_index, public_key_size);
		if (!Long::isLong(public_key_array))
		{
			qDebug() << "Incorrect public key";
			return false;
		}
		mKeys.serverPublicKey = Long::fromByteArray(public_key_array);
		mParent->printLog("Recieved server's public key: " + mKeys.serverPublicKey.toString());
	}

	// public_session_key section
	{
		int public_session_key_index = 4 + public_key_size;
		QByteArray public_session_key_array = byteArray.mid(public_session_key_index,
															public_session_key_size);
		if (!Long::isLong(public_session_key_array))
		{
			qDebug() << "Incorrect public session key";
			return false;
		}
		mKeys.serverPublicSessionKey = Long::fromByteArray(public_session_key_array);
		mParent->printLog("Recieved server's session public key: "
						  + mKeys.serverPublicSessionKey.toString());
	}
	mKeys.haveServerKeys = true;

	if (mState.waiting && mState.waitingType == TYPE_PUBLIC_KEYS)
	{
		mState.waiting = false;
		mState.waitingType = 0;
	}
	makeKey(mKeys);

	return true;
}

bool ClientListener::parseLoginStatus(const QByteArray &byteArray)
{
	if (byteArray.size() != 2)
		return false;

	quint16 answer = ((byteArray[0] & 0xff) << 8) | (byteArray[1] & 0xff);
	switch(answer)
	{
	case ANSWER_LOGIN_OK:
		emit message("Successful loginned");
		emit loginned();
		break;
	case ANSWER_LOGIN_FAIL:
		emit message("Invalid PIN");
		break;
	default:
		emit message("Unknown error");
		break;
	}
	return true;
}

void ClientListener::makeSessionKeys()
{
	mParent->printLog("Generating session private key...");
	// private key
	int random_bits_size = GROUP_PRIME.getSize();
	char *random_bits = new char[random_bits_size];
	drbg_generate(random_bits, random_bits_size);
	mKeys.privateSessionKey = Long(random_bits, random_bits_size);
	mKeys.privateSessionKey.setModule(GROUP_PRIME);
	delete []random_bits;

	mParent->printLog("Calculating session public key...");
	// public key
	mKeys.publicSessionKey = GROUP_GENERATOR;
	mKeys.publicSessionKey.setModule(GROUP_PRIME);
	mKeys.publicSessionKey.pow(mKeys.privateSessionKey);

	mParent->printLog("OK. Session private & public keys generated");
}

void ClientListener::makeKey(const ConnectionKeys &keys)
{
	mParent->printLog("Generating common key...");

	char hash[32];

	Long mod(GROUP_PRIME);
	mod.dec();
	int hash_m_size = mKeys.publicSessionKey.bytesNeed();
	char *hash_m = new char[hash_m_size];
	mKeys.publicSessionKey.toChar(hash_m, 0);
	calc_sha256_hash(hash_m, hash_m_size, hash, 0);
	delete []hash_m;
	Long e(hash, 32);
	e.setModule(mod);

	hash_m_size = mKeys.serverPublicSessionKey.bytesNeed();
	hash_m = new char[hash_m_size];
	mKeys.serverPublicSessionKey.toChar(hash_m, 0);
	calc_sha256_hash(hash_m, hash_m_size, hash, 0);
	delete []hash_m;
	Long d(hash, 32);
	d.setModule(mod);

	mKeys.currentKey = mKeys.serverPublicKey;
	//mKeys.currentKey.setModule(GROUP_PRIME);
	mKeys.currentKey.pow(e);
	mKeys.currentKey = mKeys.currentKey * mKeys.serverPublicSessionKey;
	Long a1 = mKeys.privateSessionKey;
	a1.setModule(mod);
	Long a2 = d;
	a2.setModule(mod);
	Long a3 = mKeys.privateKey;
	a3.setModule(mod);
	Long power = a1 + a2 * a3;
	mKeys.currentKey.pow(power);

	mParent->printLog("OK. Common key generated");
	mParent->printLog("key = " + mKeys.currentKey.toString());
}



