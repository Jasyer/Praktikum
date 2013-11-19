#include "serversettings.h"
#include "stringconstants.h"
#include <QHostAddress>

ServerSettings *ServerSettings::mInstanse = NULL;

/**
 * @brief Default constructor
 */
ServerSettings::ServerSettings()
{
	mDefaultSettings.insert(textServerIP, "127.0.0.1");
	mDefaultSettings.insert(textServerPort, "1919");
}

/**
 * @brief Destructor
 */
ServerSettings::~ServerSettings()
{
	delete mInstanse;
}

/**
 * @brief Gets current settings (singleton)
 * @return Instanse of the ServerSettings class
 */
ServerSettings *ServerSettings::currentSettings()
{
	if (mInstanse == NULL)
		mInstanse = new ServerSettings;
	return mInstanse;
}

/**
 * @brief Sets default values in settings
 */
void ServerSettings::setDefaultSettings()
{
	mSettings = mDefaultSettings;
}

/**
 * @brief Gets value of settingName setting
 * @param settingName Name of setting
 * @return String with value of settingName setting
 * @throws NoSuchSettingName if not found settingName setting
 */
QString ServerSettings::get(const QString &settingName)
{
	QHash<QString, QString>::Iterator iter;
	if ((iter = mSettings.find(settingName)) == mSettings.end())
		throw NoSuchSettingName;
	return iter.value();
}

/**
 * @brief Sets new value of settingName setting
 * @param settingName Name of setting
 * @param settingValue Value of settingName setting
 * @throws NoSuchSettingName if not found settingName setting
 */
void ServerSettings::set(const QString &settingName, const QString &settingValue)
{
	QHash<QString, QString>::Iterator iter;
	if ((iter = mSettings.find(settingName)) == mSettings.end())
		throw NoSuchSettingName;
	mSettings.insert(settingName, settingValue);
}

