#ifndef DIALOGDATABASEITEMEDIT_H
#define DIALOGDATABASEITEMEDIT_H

#include <QDialog>
#include "longlibrary.h"

namespace Ui {
class DialogDatabaseItemEdit;
}

class DialogDatabaseItemEdit : public QDialog
{
	Q_OBJECT

public:
	explicit DialogDatabaseItemEdit(QWidget *parent = 0);
	~DialogDatabaseItemEdit();

private:
	Ui::DialogDatabaseItemEdit *ui;

private slots:
	void onClickedButtonOK();

signals:
	void databaseNewItem(const QString &name, const Long &hashPIN);
};

#endif // DIALOGDATABASEITEMEDIT_H
