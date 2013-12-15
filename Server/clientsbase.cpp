#include "clientsbase.h"
#include "cryptograph.h"

ClientsBase::ClientsBase()
{

}

bool ClientsBase::add(const Long &hashKey, const ClientInfo &info)
{
	if (mData.contains(hashKey))
		return false;
	mData.insert(hashKey, info);
	return true;
}

ClientInfo ClientsBase::find(const Long &hashKey)
{
	if (!mData.contains(hashKey))
		return ClientInfo::nullInfo();
	else
		return mData[hashKey];
}


ClientInfo::ClientInfo(const QString &name) : mNull(false), mName(name)
{
	char buf[8];
	drbg_generate(buf, 8);
	mCliendID = Long(buf, 8);
}

const ClientInfo ClientInfo::nullInfo()
{
	return ClientInfo();
}

bool ClientInfo::operator<(const ClientInfo &info) const
{
	return mCliendID < info.clientID();
}

bool ClientInfo::isNull() const
{
	return mNull;
}

Long ClientInfo::clientID() const
{
	return mCliendID;
}

QString ClientInfo::name() const
{
	return mName;
}
