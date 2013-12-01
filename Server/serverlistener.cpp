#include "serverlistener.h"
#include "cryptograph.h"
#include "cryptoconstants.h"
#include "longlibrary.h"
#include <QTcpSocket>
#include <QTextCodec>
#include "commands.h"
#include "server.h"

ServerListener::ServerListener(QTcpSocket *socket, const Long &privateKey, const Long &publicKey,
							   Server *parent)
{
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
	makeSessionKeys();

	sendCommand(STATE_READY);
}

void ServerListener::onDisconnected()
{
	// emit diconnected(socket_id);
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

	mData.type = readUInt16();
	mData.size -= 2; // 2 bytes for type

	mData.data = mSocket->read(mData.size);

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
	mParent->printLog("SENDING " + fromInt(type));

	quint32 size = data.size() + 2;
	writeUInt32(size);
	writeUInt16(type);
	mSocket->write(data);
}

void ServerListener::sendText(const QString &text)
{
	sendData(TYPE_TEXT_MESSAGE, QTextCodec::codecForLocale()->fromUnicode(text));
}

/**
 * @brief ServerListener::sendPublicKeys
 *
 * Keys block:
 * [ public_key_size | public_session_key_size | public_key | public_session_key ]
 * [    2 bytes      |        2 bytes          |            |                    ]
 */
void ServerListener::sendPublicKeys()
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
	switch(mData.type)
	{
	case REQUEST_PUBLIC_KEYS:
		sendPublicKeys();
		sendCommand(REQUEST_PUBLIC_KEYS);
		break;
	case TYPE_PUBLIC_KEYS:
		parsePublicKeys(mData.data);
		break;
	case TYPE_LOGIN:
		parseLogin(mData.data);
		break;
	default:
		break;
	}
	mData.clear();
}

void ServerListener::parsePublicKeys(const QByteArray &byteArray)
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
		return;
	}

	// public_key section
	{
		int public_key_index = 4;
		QByteArray public_key_array = byteArray.mid(public_key_index, public_key_size);
		if (!Long::isLong(public_key_array))
		{
			qDebug() << "Incorrect public key";
			return;
		}
		mKeys.clientPublicKey = Long::fromByteArray(public_key_array);
		mParent->printLog("Recieved client's public key: " + mKeys.clientPublicKey.toString());
	}

	// public_session_key section
	{
		int public_session_key_index = 4 + public_key_size;
		QByteArray public_session_key_array = byteArray.mid(public_session_key_index,
															public_session_key_size);
		if (!Long::isLong(public_session_key_array))
		{
			qDebug() << "Incorrect public session key";
			return;
		}
		mKeys.clientPublicSessionKey = Long::fromByteArray(public_session_key_array);
		mParent->printLog("Recieved client's session public key: "
						  + mKeys.clientPublicSessionKey.toString());
	}
	mKeys.haveClientKeys = true;

	if (mState.waiting && mState.waitingType == TYPE_PUBLIC_KEYS)
	{
		mState.waiting = false;
		mState.waitingType = 0;
	}
	makeKey(mKeys);
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

void ServerListener::makeSessionKeys()
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

void ServerListener::makeKey(const ConnectionKeys &keys)
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

	hash_m_size = mKeys.clientPublicSessionKey.bytesNeed();
	hash_m = new char[hash_m_size];
	mKeys.clientPublicSessionKey.toChar(hash_m, 0);
	calc_sha256_hash(hash_m, hash_m_size, hash, 0);
	delete []hash_m;
	Long d(hash, 32);
	d.setModule(mod);

	mKeys.currentKey = mKeys.clientPublicKey;
	//mKeys.currentKey.setModule(GROUP_PRIME);
	mKeys.currentKey.pow(e);
	mKeys.currentKey = mKeys.currentKey * mKeys.clientPublicSessionKey;
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
