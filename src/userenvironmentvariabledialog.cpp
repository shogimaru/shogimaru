#include "userenvironmentvariabledialog.h"
#include "ui_userenvironmentvariabledialog.h"
#include <QFileDialog>


UserEnvironmentVariableDialog::UserEnvironmentVariableDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::UserEnvironmentVariableDialog)
{
    _ui->setupUi(this);

    connect(_ui->browseDirButton, &QPushButton::clicked, [&] {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "", QFileDialog::ShowDirsOnly);
        if (!dir.isEmpty()) {
            _ui->valueLineEdit->setText(dir);
        }
    });
    connect(_ui->browseFileButton, &QPushButton::clicked, [&] {
        QString path = QFileDialog::getOpenFileName(this, tr("Open File"), "", "*");
        if (!path.isEmpty()) {
            _ui->valueLineEdit->setText(path);
        }
    });

    // 変数名と変数値がともに空でない場合にOKボタンは有効
    connect(_ui->nameLineEdit, &QLineEdit::textChanged, [&]() {
        auto *okButton = _ui->buttonBox->button(QDialogButtonBox::Ok);
        okButton->setEnabled(!_ui->nameLineEdit->text().trimmed().isEmpty() && !_ui->valueLineEdit->text().trimmed().isEmpty());
    });
    connect(_ui->valueLineEdit, &QLineEdit::textChanged, [&]() {
        auto *okButton = _ui->buttonBox->button(QDialogButtonBox::Ok);
        okButton->setEnabled(!_ui->nameLineEdit->text().trimmed().isEmpty() && !_ui->valueLineEdit->text().trimmed().isEmpty());
    });
}


QPair<QString, QString> UserEnvironmentVariableDialog::variable()
{
    return QPair<QString, QString>(_ui->nameLineEdit->text(), _ui->valueLineEdit->text());
}


void UserEnvironmentVariableDialog::setVariable(const QString &name, const QString &value)
{
    _ui->nameLineEdit->setText(name.trimmed());
    _ui->valueLineEdit->setText(value.trimmed());
}


void UserEnvironmentVariableDialog::open()
{
    // OKボタン有効化/無効化
    auto *okButton = _ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(!_ui->nameLineEdit->text().trimmed().isEmpty() && !_ui->valueLineEdit->text().trimmed().isEmpty());
    QDialog::open();
}


void UserEnvironmentVariableDialog::accept()
{
    QDialog::accept();
}
