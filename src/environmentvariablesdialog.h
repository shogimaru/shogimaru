#pragma once
#include <QDialog>
#include <QVariant>

class QTableWidget;
class QTableWidgetItem;

namespace Ui {
class EnvironmentVariablesDialog;
}


class EnvironmentVariablesDialog : public QDialog {
    Q_OBJECT
public:
    explicit EnvironmentVariablesDialog(QWidget *parent = nullptr);
    ~EnvironmentVariablesDialog() { }

    void open();
    void accept();
    QVariantList environmentVariables() const;
    void setEnvironmentVariables(const QVariantList &variables);

private:
    void setEnvironmentVariable(const QString &name, const QString &value);

    Ui::EnvironmentVariablesDialog *_ui {nullptr};
};
