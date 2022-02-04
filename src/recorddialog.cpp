#include "recorddialog.h"
#include "ui_recorddialog.h"
#include "westerntabstyle.h"
#include "messagebox.h"

static WesternTabStyle westernTabStyle;


RecordDialog::RecordDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::RecordDialog)
{
    _ui->setupUi(this);
    _ui->tabWidget->tabBar()->setStyle(&westernTabStyle);

    connect(_ui->loadButton, &QPushButton::clicked, this, &RecordDialog::loadNotation);
    connect(_ui->closeButton, &QPushButton::clicked, this, &QDialog::reject); // 閉じるボタン
}


void RecordDialog::loadNotation()
{
    if (!validate(_ui->textEdit->toPlainText().trimmed())) {
        MessageBox::information(tr("Notation Error"), tr("Load Error"));
        return;
    }
    QDialog::accept();
}


bool RecordDialog::validate(const QString &notation)
{
    bool ok;
    _sfen = Sfen::fromCsa(notation, &ok);  // CSA
    if (ok) {
        return ok;
    }

    _sfen = Sfen::fromSfen(notation.toLatin1(), &ok);  // SFEN
    return ok;
}


void RecordDialog::open()
{
    _ui->textEdit->clear();
    _sfen.clear();
    QDialog::open();
}
