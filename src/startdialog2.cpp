#include "startdialog2.h"
#include "ui_startdialog2.h"
#include "user.h"


StartDialog2::StartDialog2(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::StartDialog2)
{
    _ui->setupUi(this);
    setWindowTitle("New Game");
}


void StartDialog2::open()
{
    auto &user = User::load();

    int byoyomi = user.byoyomi();
    int basicTime = user.basicTime();

    if (!byoyomi && !basicTime) {  // デフォルト値
        byoyomi = 5;
        basicTime = 5;
    }

    _ui->byoyomiBox->setCurrentText(QString::number(byoyomi));  // 秒読み
    _ui->minBox->setCurrentText(QString::number(basicTime % 60));
    _ui->hourBox->setCurrentText(QString::number(basicTime / 60));

    QDialog::open();
}


void StartDialog2::accept()
{
    auto &user = User::load();

    user.setByoyomi(byoyomi());
    user.setBasicTime(basicTime());
    user.save();

    QDialog::accept();
}


int StartDialog2::byoyomi() const
{
    int b = _ui->byoyomiBox->currentText().toInt();
    return b;
}


int StartDialog2::basicTime() const
{
    int m = _ui->minBox->currentText().toInt();
    int h = _ui->hourBox->currentText().toInt();
    return h * 60 + m;
}


maru::TimeMethod StartDialog2::method() const
{
    return (maru::TimeMethod)_ui->methodBox->currentIndex();
}
