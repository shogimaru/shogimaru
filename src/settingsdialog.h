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
    void accept();
    void getEngineFilePath();

protected:
    void loadAvailableEngineList();
    void save();

private:
    Ui::SettingsDialog *_ui {nullptr};
};
