#ifndef CERTIFICATEVIEWDIALOG_H
#define CERTIFICATEVIEWDIALOG_H

#include <QDialog>
#include "certificate.h"

namespace Ui {
class CertificateViewDialog;
}

class CertificateViewDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CertificateViewDialog(const Certificate &certificate, QWidget *parent = 0);
	~CertificateViewDialog();

private:
	Ui::CertificateViewDialog *ui;
};

#endif // CERTIFICATEVIEWDIALOG_H
