#pragma once
#include "global.h"
#include <QDialog>

class QTableWidget;
class QTableWidgetItem;

namespace Ui {
class EnvironmentVariablesDialog;
}


class EnvironmentVariablesDialog : public QDialog {
    Q_OBJECT
public:
    explicit EnvironmentVariablesDialog(int engineIndex, QWidget *parent = nullptr);
    ~EnvironmentVariablesDialog() { }

    void open();
    void accept();

private:
    void setEnvironmentVariable(const QString &name, const QString &value);

    Ui::EnvironmentVariablesDialog *_ui {nullptr};

    int _index {0};
};
