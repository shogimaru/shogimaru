#include "startdialog.h"
#include "ui_startdialog.h"
#include "user.h"
#include "enginesettings.h"


StartDialog::StartDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::StartDialog)
{
    _ui->setupUi(this);
    setWindowTitle("New Game");
}


void StartDialog::open()
{
    const auto &user = User::load();

    int byoyomi = user.byoyomi();
    int basicTime = user.basicTime();

    if (!byoyomi && !basicTime) {  // デフォルト値
        byoyomi = 5;
        basicTime = 5;
    }

    // if (user.startPosition() == (int)maru::Initial) {
    //     _ui->initPosButton->setChecked(true);
    // } else {
    //     _ui->currentPosButton->setChecked(true);
    // }
    _ui->methodBox->setCurrentIndex(user.method());
    _ui->byoyomiBox->setCurrentText(QString::number(byoyomi));  // 秒読み
    _ui->minBox->setCurrentText(QString::number(basicTime % 60));
    _ui->hourBox->setCurrentText(QString::number(basicTime / 60));

    _ui->senteEngine->setEnabled(EngineSettings::instance().availableEngineCount() > 0);
    _ui->goteEngine->setEnabled(EngineSettings::instance().availableEngineCount() > 0);

    QDialog::open();
}


void StartDialog::accept()
{
    auto &user = User::load();

    user.setByoyomi(byoyomi());
    user.setBasicTime(basicTime());
    user.setStartPosition(position());
    user.setMethod(method());
    user.save();

    QDialog::accept();
}


maru::StartPosision StartDialog::position() const
{
    return _ui->initPosButton->isChecked() ? maru::Initial : maru::Current;
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
    int b = _ui->byoyomiBox->currentText().toInt();
    return b;
}


int StartDialog::basicTime() const
{
    int m = _ui->minBox->currentText().toInt();
    int h = _ui->hourBox->currentText().toInt();
    return h * 60 + m;
}


maru::TimeMethod StartDialog::method() const
{
    return (maru::TimeMethod)_ui->methodBox->currentIndex();
}
