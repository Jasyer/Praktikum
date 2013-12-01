#include "clientsbase.h"

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


ClientInfo ClientInfo::nullInfo()
{
	return ClientInfo();
}

bool ClientInfo::isNull()
{
	return (mValid == false);
}
