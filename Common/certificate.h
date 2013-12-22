#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#include "longlibrary.h"
#include <QDateTime>
#include <QList>
#include <QString>

class Certificate
{
public:
	Certificate(const Long &serverID,
				const QString &serverName,
				const Long &cliendID,
				const QString &name,
				const Long &publicKey,
				const QDateTime &lifeTime,
				const QList<QString> &availableHashList,
				const QList<QString> &availableCipherList);
//	Certificate(const Certificate &cert);

	QByteArray toByteArray() const;
	void signRSA(const Long &key);
	void signElGamal(const Long &key);
	bool invoked() const;
	bool valid() const;
	Long clientID() const;
	QString name() const;
	void update();

	void operator=(const Certificate &cert);

	static Certificate fromByteArray(const QByteArray &byteArray);

	Certificate(); // was deprecate

private:
	Long mServerID;
	QString mServerName;
	Long mCliendID;
	QString mName;
	Long mPublicKey;
	QDateTime mLifeTime;
	bool mValid;
	bool mSigned;
	bool mInvoked;
	QList<QString> mAvailableHashList;
	QList<QString> mAvailableCipherList;

	Long mSignRSA;
	Long mSignElGamal;

	QByteArray toByteArrayWithoutSign() const;
	static Certificate invalidCertificate();

};

#endif // CERTIFICATE_H
