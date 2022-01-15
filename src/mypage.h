#pragma once
#include "global.h"
#include <QDialog>

namespace Ui {
class MyPage;
}


class MyPage : public QDialog {
    Q_OBJECT
public:
    explicit MyPage(QWidget *parent = nullptr);
    ~MyPage();

public slots:
    void open() override;
    void accept() override;

private:
    Ui::MyPage *_ui {nullptr};
};
