#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#include "longlibrary.h"
#include <QDateTime>
#include <QList>
#include <QString>

class Certificate
{
public:
	Certificate();
	QByteArray toByteArray() const;

private:
	Long mCliendID;
	Long mPublicKey;
	QDateTime mLifeTime;
	bool mValid;
	QList<QString> mAvailableHashList;
	QList<QString> mAvailableCypherList;


};

#endif // CERTIFICATE_H
