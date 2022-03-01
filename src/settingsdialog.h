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
    void getEnginePath();
    void setCurrentEngine(int index);
    void confirmDelete();
    void deleteEngine();
public slots:
    void showEngineOptions(int index) const;

protected:
    void save();

private:
    Ui::SettingsDialog *_ui {nullptr};
};
