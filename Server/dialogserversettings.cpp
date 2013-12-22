#include "dialogserversettings.h"
#include "ui_dialogserversettings.h"
#include "commands.h"
#include "stringconstants.h"

DialogServerSettings::DialogServerSettings(quint8 currentType, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogServerSettings)
{
	ui->setupUi(this);

	connect(ui->buttonOK, SIGNAL(clicked()), SLOT(accept()));
	connect(ui->buttomCancel, SIGNAL(clicked()), SLOT(reject()));
	ui->typeCompany->clear();
	switch (currentType)
	{
	case SERVER_FEDERAL_AGENCY:
		indexMinistry = 0;
		ui->typeCompany->addItem(textServerNameMinistry);
		break;
	case SERVER_MINISTRY:
		indexFederalAgency = 0;
		ui->typeCompany->addItem(textServerNameFederalAgency);
		break;
	default:
		indexFederalAgency = 0;
		ui->typeCompany->addItem(textServerNameFederalAgency);
		indexMinistry = 1;
		ui->typeCompany->addItem(textServerNameMinistry);
		break;
	}
}

quint8 DialogServerSettings::getServerType() const
{
	int index = ui->typeCompany->currentIndex();
	if (index == indexFederalAgency)
		return SERVER_FEDERAL_AGENCY;
	else if (index == indexMinistry)
		return SERVER_MINISTRY;
	return SERVER_FEDERAL_AGENCY;
}

QString DialogServerSettings::getServerName() const
{
	return ui->typeCompany->currentText();
}

DialogServerSettings::~DialogServerSettings()
{
	delete ui;
}
