#pragma once
#include <QDialog>
#include <QPair>

class QTableWidget;
class QTableWidgetItem;

namespace Ui {
class UserEnvironmentVariableDialog;
}


class UserEnvironmentVariableDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserEnvironmentVariableDialog(QWidget *parent = nullptr);
    ~UserEnvironmentVariableDialog() { }

    void setVariable(const QString &name, const QString &value);
    QPair<QString, QString> variable();
    void open();
    void accept();

private:
    Ui::UserEnvironmentVariableDialog *_ui {nullptr};
};
