#pragma once
#include "global.h"
#include "sfen.h"
#include <QDialog>


namespace Ui {
class NotationDialog;
}


class NotationDialog : public QDialog {
    Q_OBJECT
public:
    explicit NotationDialog(QWidget *parent = nullptr);
    ~NotationDialog() {}

    void open();
    void accept() {}
    Sfen result() const { return _sfen; }
    void loadNotation();

private:
    bool validate(const QString &notation);

    Ui::NotationDialog *_ui {nullptr};
    Sfen _sfen;
};
