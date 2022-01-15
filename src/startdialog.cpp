#include "startdialog.h"
#include "ui_startdialog.h"


StartDialog::StartDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::StartDialog)
{
    _ui->setupUi(this);
    setWindowTitle("StartDialog");

    // 持ち時間
    for (int i = 0; i < 10; i++) {
        _ui->hourBox->addItem(QString::number(i), i);
    }
    for (int i = 0; i < 60; i++) {
        _ui->minBox->addItem(QString::number(i), i);
    }
    _ui->minBox->setCurrentIndex(10);

    for (int i = 0; i <= 60; i++) {
        _ui->byoyomiBox->addItem(QString::number(i), i);
    }
    _ui->byoyomiBox->setCurrentIndex(30);
}


StartDialog::StartPosision StartDialog::postion() const
{
    return _ui->initPosButton->isChecked() ? Initial : Current;
}


maru::PlayerType StartDialog::player(maru::Turn turn) const
{
    if (turn == maru::Sente) {
        return (_ui->senteHuman->isChecked()) ? maru::Human : maru::Computer;
    } else {
        return (_ui->goteHuman->isChecked()) ? maru::Human : maru::Computer;
    }
}


int StartDialog::byoyomi() const
{
    return _ui->byoyomiBox->currentData().toInt();
}


QTime StartDialog::time() const
{
    return QTime(_ui->hourBox->currentData().toInt(), _ui->minBox->currentData().toInt());
}
