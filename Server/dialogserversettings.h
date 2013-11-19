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
	explicit DialogServerSettings(QWidget *parent = 0);
	~DialogServerSettings();

private:
	Ui::DialogServerSettings *ui;
};

#endif // DIALOGSERVERSETTINGS_H
