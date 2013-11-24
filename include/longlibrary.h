#ifndef LONGLIBRARY_H
#define LONGLIBRARY_H

#include "longlibrary_global.h"

class LONGLIBRARYSHARED_EXPORT Long
{
public:
    static const quint8 max_int = 0xf;
    Long();
    ~Long();
    Long(const Long &);
    Long(const char *s);
    Long(const QString &s);
    Long(char *data, short int data_size);
    Long(const unsigned int n);

    quint8 &operator[] (quint16 i) const;
    bool operator> (const Long &) const;
    bool operator< (const Long &) const;
    bool operator== (const Long &) const;

    Long operator= (const Long &);
    friend Long operator* (const Long &, const Long &);
    friend Long operator+ (const Long &, const Long &);

    void mod(const Long &_long);
    void pow(const Long &_long);

    quint16 get_size() const;
    void set_module(const Long &l);
    void delete_module();
    void to_char(char *data, int *data_size);
    QString toString() const;
    int bytes_need() const;

    static Long first_bytes_of(const Long &l, quint16 bytes);

private:
    quint8 *data;
    quint16 size;
    Long *module;

    quint32 countOfBits() const;
    void delete_first_zero_bytes();
    void setSize(quint16 new_size);
    bool sub(const Long &l);
    bool shr();

    friend Long multiplicate_mini(const Long &l, quint8 num);
    friend Long devide_mini(const Long &_l1, const Long &_l2);
};

#endif // LONGLIBRARY_H
