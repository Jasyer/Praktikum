#ifndef LONGLIBRARY_H
#define LONGLIBRARY_H

#include "longlibrary_global.h"
#include <QString>
#include <QByteArray>

class LONGLIBRARYSHARED_EXPORT Long
{
public:
	static const quint8 max_int = 0xf;
	Long();
	~Long();
	Long(const Long &);
	Long(const char *s);
	Long(const QString &s);
	Long(char *data, int data_size);
	Long(const unsigned int n);

	quint8 &operator[] (quint16 i) const;
	bool operator> (const Long &) const;
	bool operator< (const Long &) const;
	bool operator== (const Long &) const;

	Long operator= (const Long &);
	friend Long LONGLIBRARYSHARED_EXPORT operator* (const Long &, const Long &);
	friend Long LONGLIBRARYSHARED_EXPORT operator+ (const Long &, const Long &);

	void pow(const Long &_long);
	void dec();

	quint16 getSize() const;
	void setModule(const Long &l);
	void setSize(quint16 new_size);
	void deleteModule();
	void toChar(char *data, int *data_size);
	QString toString() const;
	QByteArray toByteArray() const;
	int bytesNeed() const;

	static Long firstBytesOf(const Long &l, quint16 bytes);
	static Long fromByteArray(const QByteArray &byteArray);
	static bool isLong(const QByteArray &byteArray);
	static Long getInversed(const Long &l);

private:
	quint8 *data;
	quint16 size;
	Long *module;

	quint32 countOfBits() const;
	void mod(const Long &_long);
	bool sub(const Long &l);
	void delete_first_zero_bytes();
	bool sub_mini(const Long &l);
	bool shr();
	friend void div_mod(const Long &_l1, const Long &_l2, Long *div, Long *mod);
	friend Long multiplicate_mini(const Long &l, quint8 num);
	friend Long devide_mini(const Long &_l1, const Long &_l2, quint8 *ans);
};

void div_mod(const Long &_l1, const Long &_l2, Long *div, Long *mod);

#endif // LONGLIBRARY_H
