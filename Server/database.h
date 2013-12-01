#ifndef DATABASE_H
#define DATABASE_H

#include <QHash>
#include "certificate.h"
#include "longlibrary.h"

class DataBase
{
public:
	DataBase();
//	void add(Long id, const Certificate &certificate);
	void encryptWithKey(const Long &key);

private:
	QHash<Long, Certificate> mData;
};

#endif // DATABASE_H
