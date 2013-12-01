#include "dialogdatabaseitemedit.h"
#include "ui_dialogdatabaseitemedit.h"
#include "cryptograph.h"
#include <QTextCodec>

DialogDatabaseItemEdit::DialogDatabaseItemEdit(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogDatabaseItemEdit)
{
	ui->setupUi(this);

	// gui connections
	connect(ui->buttonOK, SIGNAL(clicked()), SLOT(onClickedButtonOK()));
	connect(ui->buttonCancel, SIGNAL(clicked()), SLOT(reject()));
}

DialogDatabaseItemEdit::~DialogDatabaseItemEdit()
{
	delete ui;
}

void DialogDatabaseItemEdit::onClickedButtonOK()
{
	QString PIN_str = ui->textPIN->text();
	QByteArray PIN_uni = QTextCodec::codecForLocale()->fromUnicode(PIN_str);

	int hash_m_size = PIN_uni.size();
	char *hash_m = new char[hash_m_size];
	for (int i = 0; i < hash_m_size; ++i)
		hash_m[i] = PIN_uni[i];
	char hash[32];
	calc_sha256_hash(hash_m, hash_m_size, hash, 0);
	delete []hash_m;
	Long PIN_hash(hash, 32);
	emit databaseNewItem(ui->textName->text(), PIN_hash);
	accept();
}

