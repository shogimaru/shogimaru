#include "environmentvariablesdialog.h"
#include "ui_environmentvariablesdialog.h"
#include "userenvironmentvariabledialog.h"
#include "messagebox.h"


EnvironmentVariablesDialog::EnvironmentVariablesDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::EnvironmentVariablesDialog)
{
    _ui->setupUi(this);

    connect(_ui->tableWidget, &QTableWidget::itemChanged, [](QTableWidgetItem *item) {
        if (item) {
            // ツールチップを更新
            item->setToolTip(item->text());
        }
    });

    // 新規ボタン
    connect(_ui->newButton, &QPushButton::clicked, [this]() {
        UserEnvironmentVariableDialog dialog(this);
        dialog.open();

        if (dialog.exec() == QDialog::Accepted) {
            auto p = dialog.variable();
            setEnvironmentVariable(p.first.trimmed(), p.second.trimmed());
        }
    });

    // 編集ボタン
    connect(_ui->editButton, &QPushButton::clicked, [this]() {
        UserEnvironmentVariableDialog dialog(this);
        int index =_ui->tableWidget->currentRow();
        if (index >= 0 && index < _ui->tableWidget->rowCount()) {
            dialog.setVariable(_ui->tableWidget->item(index, 0)->text(), _ui->tableWidget->item(index, 1)->text());
        }
        dialog.open();

        if (dialog.exec() == QDialog::Accepted) {
            auto p = dialog.variable();
            setEnvironmentVariable(p.first.trimmed(), p.second.trimmed());
        }
    });

    // 削除ボタン
    connect(_ui->deleteButton, &QPushButton::clicked, [this]() {
        int index =_ui->tableWidget->currentRow();
        if (index >= 0 && index < _ui->tableWidget->rowCount()) {
            QString text = tr("Delete %1 variable.\nAre you sure?").arg(_ui->tableWidget->item(index, 0)->text());
            MessageBox::question(tr("Delete variable"), text, [this]() {
                // OKの場合
                _ui->tableWidget->removeRow(_ui->tableWidget->currentRow());
            });
        }
    });
}


void EnvironmentVariablesDialog::open()
{
    QDialog::open();
}


void EnvironmentVariablesDialog::accept()
{
    QDialog::accept();
}


QVariantList EnvironmentVariablesDialog::environmentVariables() const
{
    QVariantList variables;

    for (int row = 0; row < _ui->tableWidget->rowCount(); ++row) {
        QVariantMap var;
        auto* nameItem = _ui->tableWidget->item(row, 0);
        auto* valueItem = _ui->tableWidget->item(row, 1);
        if (!nameItem->text().isEmpty()) {
            var["name"] = nameItem->text();
            var["value"] = valueItem->text();
            variables << var;
        }
    }
    return variables;
}


void EnvironmentVariablesDialog::setEnvironmentVariables(const QVariantList &variables)
{
    int i = 0;
    for (auto &variable : variables) {
        _ui->tableWidget->setRowCount(i + 1);
        auto var = variable.toMap();
        auto item = new QTableWidgetItem(var.value("name").toString());
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        _ui->tableWidget->setItem(i, 0, item);
        item = new QTableWidgetItem(var.value("value").toString());
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        _ui->tableWidget->setItem(i, 1, item);
        i++;
    }

    if (i > 0) {
        _ui->tableWidget->setCurrentCell(0, 0);
    }
}


void EnvironmentVariablesDialog::setEnvironmentVariable(const QString &name, const QString &value)
{
    QString n = name.trimmed();
    QString v = value.trimmed();

    if (n.isEmpty() || v.isEmpty()) {
        return;
    }

    // 同じ変数名のものを削除
    for (int row = _ui->tableWidget->rowCount() - 1; row >= 0; --row) {
        auto* nameItem = _ui->tableWidget->item(row, 0);
        if (nameItem->text() == n) {
            _ui->tableWidget->removeRow(row);
        }
    }

    int row = _ui->tableWidget->rowCount();
    _ui->tableWidget->setRowCount(row + 1);
    auto item = new QTableWidgetItem(name);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    _ui->tableWidget->setItem(row, 0, item);
    item = new QTableWidgetItem(value);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    _ui->tableWidget->setItem(row, 1, item);
    _ui->tableWidget->setCurrentCell(row, 0);
}
