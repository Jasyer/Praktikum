#include "certificate.h"
#include <QTextCodec>

Certificate::Certificate()
{
}

/**
 * @brief Certificate::toByteArray
 *
 * [ size_without_sign | size_of_sign | without_sign | sign ]
 * [     2 bytes       |   2 bytes    |              |      ]
 */
QByteArray Certificate::toByteArray() const
{
	// without_sign
	QByteArray without_sign = toByteArrayWithoutSign();


}

/**
 * @brief Certificate::toByteArrayWithoutSign
 *
 * [ fields_count | size_of_field_1 | field_1 | size_of_field_2 | field_2 | ... ]
 * [   2 bytes    |    2 bytes      |         |     2 bytes     |         | ... ]
 */

QByteArray Certificate::toByteArrayWithoutSign() const
{
	QByteArray ret;
	// mName
	{
		quint16 size = mName.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(QTextCodec::codecForLocale()->fromUnicode(mName));
	}

	// mPublicKey
	{
		QByteArray array = mPublicKey.toByteArray();
		quint16 size = array.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(array);
	}

	// mLifeTime
	{
		QString format = mLifeTime.toString("dd.MM.yyyy hh:mm:ss");
		quint16 size = format.size();
		char buf[2];
		buf[0] = (size >> 8) & 0xff;
		buf[1] = size & 0xff;
		ret.append(buf, 2);
		ret.append(QTextCodec::codecForLocale()->fromUnicode(format));
	}

	// mValid
	{
		quint16 size = 1;
		char buf[2];
		buf[0] = 0;
		buf[1] = 1;
		ret.append(buf, 2);
		ret.append((char) (mValid ? 1 : 0));
	}

	// mAvailableHashList
	{
		QByteArray array;
		for (int i = 0; i < mAvailableHashList.size(); ++i)
		{
			QByteArray arr1 = QTextCodec::codecForLocale()->fromUnicode(
						mAvailableHashList.at(i));
			quint16 size = arr1.size();
			char buf[2];
			buf[0] = (size >> 8) & 0xff;
			buf[1] = size & 0xff;
			array.append(buf, 2);
			array.append(arr1);
		}
	}

	// mAvailableCypherList
	{
		QByteArray array;
		for (int i = 0; i < mAvailableCypherList.size(); ++i)
		{
			QByteArray arr1 = QTextCodec::codecForLocale()->fromUnicode(
						mAvailableCypherList.at(i));
			quint16 size = arr1.size();
			char buf[2];
			buf[0] = (size >> 8) & 0xff;
			buf[1] = size & 0xff;
			array.append(buf, 2);
			array.append(arr1);
		}
	}
}
