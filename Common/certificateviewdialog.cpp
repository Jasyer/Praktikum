#include "certificateviewdialog.h"
#include "ui_certificateviewdialog.h"

CertificateViewDialog::CertificateViewDialog(const Certificate &certificate, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CertificateViewDialog)
{
	ui->setupUi(this);

	// info
	ui->textOwner->setText(certificate.name());
	if (certificate.invoked())
		ui->textStatus->setText("Invoked");
	else
		ui->textStatus->setText("Valid");
	ui->textExpiration->setText(certificate.lifeTime());

	// server info
	ui->textServerName->setText(certificate.serverName());
	ui->textServerPublicRSA->setText(certificate.publicKey().toString());
	ui->textServerRSASign->setText(certificate.getRSAsign().toString());

	QList<QString> list = certificate.availableHashList();
	QString buf;
	for (int i = 0; i < list.size(); ++i)
	{
		buf += list[i];
		if (i + 1 < list.size())
			buf += "\n";
	}
	ui->textAvailableHash->setText(buf);

	list = certificate.availableCipherList();
	buf.clear();
	for (int i = 0; i < list.size(); ++i, buf += "\n")
	{
		buf += list[i];
		if (i + 1 < list.size())
			buf += "\n";
	}
	ui->textAvailableCipher->setText(buf);

}

CertificateViewDialog::~CertificateViewDialog()
{
	delete ui;
}
