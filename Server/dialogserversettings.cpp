#include "dialogserversettings.h"
#include "ui_dialogserversettings.h"

DialogServerSettings::DialogServerSettings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogServerSettings)
{
	ui->setupUi(this);
}

DialogServerSettings::~DialogServerSettings()
{
	delete ui;
}
