#pragma once
#include "global.h"
#include "engine.h"
#include <QDialog>
#include <QMap>

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
    void showEngineOptions(int index);
    void resetEngineOptions();

public slots:
    void switchEngineOptions(int index);

protected slots:
    void save();

protected:
    void loadSettings();
    void updateEngineOptions(int index);

private:
    Ui::SettingsDialog *_ui {nullptr};
    QMap<QString, Engine::Option> _defaultOptions;  // 選択中のエンジンのデフォルトオプション
};
