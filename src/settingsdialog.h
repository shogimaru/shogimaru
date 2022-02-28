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

public slots:
    void open();
    void accept();
    void getEnginePath();
    void setCurrentEngine(int index);
    void showEngineOptions(int index) const;
    void confirmDelete();
    void deleteEngine();

protected:
    void save();

private:
    Ui::SettingsDialog *_ui {nullptr};
};
