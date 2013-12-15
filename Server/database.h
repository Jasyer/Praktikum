#ifndef DATABASE_H
#define DATABASE_H

#include <QMap>
#include "certificate.h"
#include "longlibrary.h"

class DataBase
{
public:
	DataBase();
//	void add(Long id, const Certificate &certificate);
	void encryptWithKey(const Long &key);

private:
	QMap<Long, Certificate> mData;
};

#endif // DATABASE_H
