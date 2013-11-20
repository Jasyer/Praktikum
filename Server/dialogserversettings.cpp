#include "dialogserversettings.h"
#include "ui_dialogserversettings.h"
#include "serversettings.h"
#include "stringconstants.h"

DialogServerSettings::DialogServerSettings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogServerSettings)
{
	ui->setupUi(this);

	connect(ui->buttonOK, SIGNAL(clicked()), SLOT(onClickedButtonOK()));
	connect(ui->buttomCancel, SIGNAL(clicked()), SLOT(reject()));

	ui->textIP->setText(ServerSettings::currentSettings()->get(textServerIP));
	ui->textPort->setText(ServerSettings::currentSettings()->get(textServerPort));
}

DialogServerSettings::~DialogServerSettings()
{
	delete ui;
}

void DialogServerSettings::onClickedButtonOK()
{
	ServerSettings::currentSettings()->set(textServerIP, ui->textIP->text());
	ServerSettings::currentSettings()->set(textServerPort, ui->textPort->text());
	accept();
}
