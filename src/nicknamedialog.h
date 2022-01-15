#pragma once
#include <QDialog>

namespace Ui {
class NicknameDialog;
}


class NicknameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NicknameDialog(QWidget *parent = nullptr);
    ~NicknameDialog();

public slots:
    void save();

private:
    Ui::NicknameDialog *_ui;
};
