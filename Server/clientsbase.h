#ifndef CLIENTSBASE_H
#define CLIENTSBASE_H

#include <QMap>
#include <QString>
#include "longlibrary.h"

class ClientInfo
{
public:
	ClientInfo() : mNull(true) {}

	ClientInfo(const QString &name, const Long &clientID) : mCliendID(clientID), mName(name) {}
	ClientInfo(const QString &name);
	bool isNull() const;
	Long clientID() const;
	QString name() const;

	static const ClientInfo nullInfo();

	bool operator<(const ClientInfo &info) const;

private:
	bool mNull;
	Long mCliendID;
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
