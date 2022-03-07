#pragma once
#include "global.h"
#include <QDialog>

class QTableWidget;
class QTableWidgetItem;

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
    void slotItemClicked(QTableWidgetItem *item);
    void resetEngineOptions();
    //void deleteEngineOption();
    //void deleteGeneralOption();

public slots:
    void showEngineOptions(int index);

protected slots:
    void save();

protected:
    void loadSettings();
    void updateEngineOptions(int index);
    //void deleteOption(QTableWidget *tableWidget, const QStringList &excludes = QStringList());

private:
    Ui::SettingsDialog *_ui {nullptr};
};
