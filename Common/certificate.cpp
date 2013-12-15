#include "certificate.h"
#include "cryptograph.h"
#include "cryptoconstants.h"
#include "longlibrary.h"
#include <QTextCodec>
#include <QDebug>


Certificate::Certificate() : mValid(false), mSigned(false)
{
}

Certificate::Certificate(const Long &serverID,
						 const QString &serverName,
						 const Long &cliendID,
						 const QString &name,
						 const Long &publicKey,
						 const QDateTime &lifeTime,
						 const QList<QString> &availableHashList,
						 const QList<QString> &availableCipherList) :
	mServerID(serverID),
	mServerName(serverName),
	mCliendID(cliendID),
	mName(name),
	mPublicKey(publicKey),
	mLifeTime(lifeTime),
	mValid(true), // it is one way to create valid certificate
	mSigned(false),
	mInvoked(false),
	mAvailableHashList(availableHashList),
	mAvailableCipherList(availableCipherList)
{
}

Certificate::Certificate(const Certificate &cert) :
	mCliendID(cert.mCliendID),
	mName(cert.mName),
	mPublicKey(cert.mPublicKey),
	mLifeTime(cert.mLifeTime),
	mValid(cert.mValid),
	mSigned(cert.mSigned),
	mInvoked(cert.mInvoked),
	mAvailableHashList(cert.mAvailableHashList),
	mAvailableCipherList(cert.mAvailableCipherList),
	mSignRSA(cert.mSignRSA)
{
}

/**
 * @brief Certificate::toByteArray
 *
 * same as toByteArrayWithoutSign with added sign
 */

QByteArray Certificate::toByteArray() const
{
	QByteArray ret;
	ret = toByteArrayWithoutSign();

	// mSignRSA
	{
		qDebug() << "mSignRSA index: " << ret.size();
		QByteArray array = mSignRSA.toByteArray();
		quint16 size = array.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(array);
	}

	// mSignElGamal
	{
		qDebug() << "mSignElGamal index: " << ret.size();
		QByteArray array = mSignElGamal.toByteArray();
		quint16 size = array.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(array);
	}

	qDebug() << "to: " << ret.size();
	return ret;
}

/**
 * @brief Certificate::toByteArrayWithoutSign
 *
 * [ fields_count | size_of_field_1 | field_1 | size_of_field_2 | field_2 | ... ]
 * [   2 bytes    |    2 bytes      |         |     2 bytes     |         | ... ]
 */

QByteArray Certificate::toByteArrayWithoutSign() const
{
	QByteArray ret;
	// mServerID
	{
		qDebug() << "mServerID index: " << ret.size();
		QByteArray array = mServerID.toByteArray();
		quint16 size = array.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(array);
	}

	// mServerName
	{
		qDebug() << "mServerName index: " << ret.size();
		quint16 size = mServerName.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(QTextCodec::codecForLocale()->fromUnicode(mServerName));
	}

	// mClientID
	{
		qDebug() << "mCliendID index: " << ret.size();
		QByteArray array = mCliendID.toByteArray();
		quint16 size = array.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(array);
	}

	// mName
	{
		qDebug() << "mName index: " << ret.size();
		quint16 size = mName.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(QTextCodec::codecForLocale()->fromUnicode(mName));
	}

	// mPublicKey
	{
		qDebug() << "mPublicKey index: " << ret.size();
		QByteArray array = mPublicKey.toByteArray();
		quint16 size = array.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(array);
	}

	// mLifeTime
	{
		qDebug() << "mLifeTime index: " << ret.size();
		QString format = mLifeTime.toString("dd.MM.yyyy hh:mm:ss");
		quint16 size = format.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(QTextCodec::codecForLocale()->fromUnicode(format));
	}

	// mValid
	{
		qDebug() << "mValid index: " << ret.size();
		quint16 size = 1;
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append((char) (mValid ? 1 : 0));
	}

	// mSigned
	{
		qDebug() << "mSigned index: " << ret.size();
		quint16 size = 1;
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append((char) (mSigned ? 1 : 0));
	}

	// mInvoked
	{
		qDebug() << "mInvoked index: " << ret.size();
		quint16 size = 1;
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append((char) (mInvoked ? 1 : 0));
	}

	// mAvailableHashList
	{
		qDebug() << "mAvailableHashList index: " << ret.size();
		quint16 count = mAvailableHashList.count();
		{
			char buf[2];
			buf[0] = (count >> 8) & 0xff;
			buf[1] = count & 0xff;
			ret.append(buf, 2);
		}
		for (int i = 0; i < count; ++i)
		{
			qDebug() << "	[" << i << "]: " << ret.size();
			QByteArray arr1 = QTextCodec::codecForLocale()->fromUnicode(
						mAvailableHashList.at(i));
			quint16 size = arr1.size();
			char buf[2];
			buf[0] = (size >> 8) & 0xff;
			buf[1] = size & 0xff;
			ret.append(buf, 2);
			ret.append(arr1);
		}
	}

	// mAvailableCipherList
	{
		qDebug() << "mAvailableCipherList index: " << ret.size();
		quint16 count = mAvailableCipherList.count();
		{
			char buf[2];
			buf[0] = (count >> 8) & 0xff;
			buf[1] = count & 0xff;
			ret.append(buf, 2);
		}
		for (int i = 0; i < count; ++i)
		{
			qDebug() << "	[" << i << "]: " << ret.size();
			QByteArray arr1 = QTextCodec::codecForLocale()->fromUnicode(
						mAvailableCipherList.at(i));
			quint16 size = arr1.size();
			char buf[2];
			buf[0] = (size >> 8) & 0xff;
			buf[1] = size & 0xff;
			ret.append(buf, 2);
			ret.append(arr1);
		}
	}
	return ret;
}

void Certificate::signRSA(const Long &key)
{
	QByteArray arr = toByteArrayWithoutSign();
	int size = arr.size();
	char *buf = new char[size];
	for (int i = 0; i < size; ++i)
		buf[i] = arr[i];
	char hash[32];
	calc_sha256_hash(buf, size, hash, 0);
	Long rsa_d = key;
	rsa_d.setModule(RSA_PHI);
	rsa_d = Long::getInversed(rsa_d);

	mSignRSA = Long(hash, 32);
	mSignRSA.setModule(RSA_N);
	mSignRSA.pow(rsa_d);

	mSigned = !(mSignElGamal == Long((uint) 0));
}

void Certificate::signElGamal(const Long &key)
{
	QByteArray arr = toByteArrayWithoutSign();
	int size = arr.size();
	char *buf = new char[size];
	for (int i = 0; i < size; ++i)
		buf[i] = arr[i];
	char hash[32];
	calc_sha256_hash(buf, size, hash, 0);
	Long rsa_d = key;
	rsa_d.setModule(RSA_PHI);
	rsa_d = Long::getInversed(rsa_d);

	mSignElGamal = Long(hash, 32);
	mSignElGamal.setModule(RSA_N);
	mSignElGamal.pow(rsa_d);

	mSigned = !(mSignRSA == Long((uint) 0));
}

bool Certificate::invoked() const
{
	return mInvoked;
}

bool Certificate::valid() const
{
	return mValid;
}

Long Certificate::clientID() const
{
	return mCliendID;
}

QString Certificate::name() const
{
	return mName;
}

void Certificate::update()
{
	if (QDateTime::currentDateTime() >= mLifeTime)
		mInvoked = true;
}

void Certificate::operator=(const Certificate &cert)
{
	mCliendID = cert.mCliendID;
	mName = cert.mName;
	mPublicKey = cert.mPublicKey;
	mLifeTime = cert.mLifeTime;
	mValid = cert.mValid;
	mSigned = cert.mSigned;
	mInvoked = cert.mInvoked;
	mAvailableHashList = cert.mAvailableHashList;
	mAvailableCipherList = cert.mAvailableCipherList;
	mSignRSA = cert.mSignRSA;
}

Certificate Certificate::fromByteArray(const QByteArray &byteArray)
{
	qDebug() << "from: " << byteArray.size() << " bytes";
	Certificate ret;
	// mServerID
	int index = 0;
	{
		qDebug() << "mServerID index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();
		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size > byteArray.size())
			return invalidCertificate();

		QByteArray array = byteArray.mid(index, size);
		ret.mServerID = Long::fromByteArray(array);
		index += size;
	}

	// mName
	{
		qDebug() << "mName index: " << index;
		QByteArray array;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size > byteArray.size())
			return invalidCertificate();

		array = byteArray.mid(index, size);
		ret.mServerName = QTextCodec::codecForLocale()->toUnicode(array);
		index += size;
	}

	// mClientID
	{
		qDebug() << "mClientID index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();
		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size > byteArray.size())
			return invalidCertificate();

		QByteArray array = byteArray.mid(index, size);
		ret.mCliendID = Long::fromByteArray(array);
		index += size;
	}

	// mName
	{
		qDebug() << "mName index: " << index;
		QByteArray array;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size > byteArray.size())
			return invalidCertificate();

		array = byteArray.mid(index, size);
		ret.mName = QTextCodec::codecForLocale()->toUnicode(array);
		index += size;
	}

	// mPublicKey
	{
		qDebug() << "mPublicKey index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size > byteArray.size())
			return invalidCertificate();

		QByteArray array = byteArray.mid(index, size);
		ret.mPublicKey = Long::fromByteArray(array);
		index += size;
	}

	// mLifeTime
	{
		qDebug() << "mLifeTime: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size > byteArray.size())
			return invalidCertificate();

		QByteArray array = byteArray.mid(index, size);
		ret.mLifeTime = QDateTime::fromString(
					QTextCodec::codecForLocale()->toUnicode(array), "dd.MM.yyyy hh:mm:ss");
		index += size;
	}

	// mValid
	{
		qDebug() << "mValid index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (size != 1 || index + size > byteArray.size())
			return invalidCertificate();

		if (byteArray[index] != 0 && byteArray[index] != 1)
			return invalidCertificate();

		ret.mValid = (bool) byteArray[index];
		index += size;
	}

	// mSigned
	{
		qDebug() << "mSigned index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (size != 1 || index + size > byteArray.size())
			return invalidCertificate();

		if (byteArray[index] != 0 && byteArray[index] != 1)
			return invalidCertificate();

		ret.mSigned = (bool) byteArray[index];
		index += size;
	}

	// mInvoked
	{
		qDebug() << "mInvoked index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (size != 1 || index + size > byteArray.size())
			return invalidCertificate();

		if (byteArray[index] != 0 && byteArray[index] != 1)
			return invalidCertificate();

		ret.mInvoked = (bool) byteArray[index];
		index += size;
	}
	// mAvailableHashList
	{
		qDebug() << "mAvailableHashList index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		for (int i = 0; i < size; ++i)
		{
			qDebug() << "	[" << i << "]: " << index;
			if (index + 2 > byteArray.size())
				return invalidCertificate();
			int size1 = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
			index += 2;

			if (index + size1 > byteArray.size())
				return invalidCertificate();

			QByteArray arr1 = byteArray.mid(index, size1);
			ret.mAvailableHashList.append(QTextCodec::codecForLocale()->toUnicode(arr1));
			index += size1;
		}
	}

	// mAvailableCipherList
	{
		qDebug() << "mAvailableCipherList index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		for (int i = 0; i < size; ++i)
		{
			qDebug() << "	[" << i << "]: " << index;
			if (index + 2 > byteArray.size())
				return invalidCertificate();
			int size1 = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
			index += 2;

			if (index + size1 > byteArray.size())
				return invalidCertificate();

			QByteArray arr1 = byteArray.mid(index, size1);
			ret.mAvailableCipherList.append(QTextCodec::codecForLocale()->toUnicode(arr1));
			index += size1;
		}
	}

	// mSignRSA
	{
		qDebug() << "mSignRSA index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size > byteArray.size())
			return invalidCertificate();

		QByteArray array = byteArray.mid(index, size);
		ret.mSignRSA = Long::fromByteArray(array);
		index += size;
	}

	// mSignElGamal
	{
		qDebug() << "mSignElGamal index: " << index;
		if (index + 2 > byteArray.size())
			return invalidCertificate();

		int size = (((quint8) byteArray[index]) << 8) | ((quint8) byteArray[index + 1]);
		index += 2;

		if (index + size > byteArray.size())
			return invalidCertificate();

		QByteArray array = byteArray.mid(index, size);
		ret.mSignElGamal = Long::fromByteArray(array);
		index += size;
	}
	return ret;
}

Certificate Certificate::invalidCertificate()
{
	return Certificate();
}
