#ifndef DIALOGSERVERSETTINGS_H
#define DIALOGSERVERSETTINGS_H

#include <QDialog>

namespace Ui {
class DialogServerSettings;
}

class DialogServerSettings : public QDialog
{
	Q_OBJECT

public:
	explicit DialogServerSettings(quint8 currentType = 0, QWidget *parent = 0);

	quint8 getServerType() const;
	QString getServerName() const;

	~DialogServerSettings();

private:
	Ui::DialogServerSettings *ui;
	int indexFederalAgency;
	int indexMinistry;
};

#endif // DIALOGSERVERSETTINGS_H
