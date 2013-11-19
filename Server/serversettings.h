#ifndef SERVERSETTINGS_H
#define SERVERSETTINGS_H

#include <QHash>

class ServerSettings
{
private:
	static ServerSettings *mInstanse;

	QHash<QString, QString> mDefaultSettings;
	QHash<QString, QString> mSettings;

	ServerSettings(); // singleton

public:
	~ServerSettings();
	enum Error
	{
		NoSuchSettingName
	};

	static ServerSettings *currentSettings();
	void setDefaultSettings();
	QString get(const QString &settingName);
	void set(const QString &settingName, const QString &settingValue);
};

#endif // SERVERSETTINGS_H
