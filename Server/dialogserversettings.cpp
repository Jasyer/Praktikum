#include "dialogserversettings.h"
#include "ui_dialogserversettings.h"
#include "serversettings.h"
#include "stringconstants.h"

DialogServerSettings::DialogServerSettings(bool pasteSettings, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogServerSettings)
{
	ui->setupUi(this);

	connect(ui->buttonOK, SIGNAL(clicked()), SLOT(accept()));
	connect(ui->buttomCancel, SIGNAL(clicked()), SLOT(reject()));

	if (pasteSettings)
	{
		ui->textIP->setText(ServerSettings::currentSettings()->get(textServerIP));
		ui->textPort->setText(ServerSettings::currentSettings()->get(textServerPort));
	}
	else
	{
		ui->textIP->clear();
		ui->textPort->clear();
	}
}

QString DialogServerSettings::getIP() const
{
	return ui->textIP->text();
}

QString DialogServerSettings::getPort() const
{
	return ui->textPort->text();
}

DialogServerSettings::~DialogServerSettings()
{
	delete ui;
}
