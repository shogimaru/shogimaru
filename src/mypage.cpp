#include "mypage.h"
#include "ui_mypage.h"
#include "user.h"


MyPage::MyPage(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::MyPage)
{
    _ui->setupUi(this);
    setWindowTitle("MyPage");
}


MyPage::~MyPage()
{
    delete _ui;
}


void MyPage::open()
{
    auto &user = User::load();

    _ui->editNickName->setText(user.nickname());
    _ui->labelRating->setText(QString::number(user.rating()));
    auto res = QString("%1 wins, %2 losses").arg(user.wins()).arg(user.losses());
    _ui->labelResults->setText(res);
    _ui->labelDraws->setText(QString::number(user.draws()));
    _ui->labelFouls->setText(QString::number(user.fouls()));

    QDialog::open();
}


void MyPage::accept()
{
    auto &user = User::load();

    auto nickname = _ui->editNickName->text().trimmed();
    if (!nickname.isEmpty() && nickname != user.nickname()) {
        user.setNickname(nickname);
        user.save();
    }

    QDialog::accept();
}
