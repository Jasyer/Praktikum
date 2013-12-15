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
	explicit DialogServerSettings(bool pasteSettings = true, QWidget *parent = 0);
	QString getIP() const;
	QString getPort() const;

	~DialogServerSettings();

private:
	Ui::DialogServerSettings *ui;
};

#endif // DIALOGSERVERSETTINGS_H
