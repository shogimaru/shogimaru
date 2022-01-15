#include "nicknamedialog.h"
#include "ui_nicknamedialog.h"
#include "user.h"


NicknameDialog::NicknameDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::NicknameDialog)
{
    _ui->setupUi(this);
    setWindowTitle("Nickname Dialog");

    connect(_ui->okButton, &QPushButton::clicked, this, &NicknameDialog::save);
}


NicknameDialog::~NicknameDialog()
{
    delete _ui;
}


void NicknameDialog::save()
{
    auto &user = User::load();

    QString name = _ui->editNickName->text().trimmed();
    user.setNickname(name);
    user.setRating(_ui->ratingSpinBox->value());
    user.save();
    emit accepted();
}
