#ifndef CLIENTSBASE_H
#define CLIENTSBASE_H

#include <QMap>
#include <QString>
#include "longlibrary.h"

class ClientInfo
{
public:
	ClientInfo() : mValid(false) {}
	ClientInfo(const QString &name) : mValid(true), mName(name){}
	static ClientInfo nullInfo();
	bool isNull();
private:
	bool mValid;
	QString mName;
};

class ClientsBase
{
public:
	ClientsBase();
	bool add(const Long &hashKey, const ClientInfo &info);
	ClientInfo find(const Long &hashKey);

private:
	QMap<Long, ClientInfo> mData;
};

#endif // CLIENTBASE_H
