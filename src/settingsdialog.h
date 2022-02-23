#pragma once
#include "global.h"
#include <QDialog>

namespace Ui {
class SettingsDialog;
}


class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() { }

    void open();

private:
    Ui::SettingsDialog *_ui {nullptr};
};
