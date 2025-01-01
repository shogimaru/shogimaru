#include "browseenginedialog.h"
#include "ui_browseenginedialog.h"
#include "environmentvariablesdialog.h"
#include <QFileDialog>


BrowseEngineDialog::BrowseEngineDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::BrowseEngineDialog)
{
    _ui->setupUi(this);

    // ファイル参照ボタン
    connect(_ui->fileBrowseButton, &QPushButton::clicked, [this] () {
#ifdef Q_OS_WIN
        QString filter = QObject::tr("Executable (*.exe)");
#else
        QString filter = QObject::tr("Executable (*)");
#endif
        QString path = QFileDialog::getOpenFileName(this, QObject::tr("Select Engine"), QString(), filter);

        if (!path.trimmed().isEmpty()) {
            _ui->pathLineEdit->setText(path.trimmed());
        }
    });

    // 環境変数ボタン
    connect(_ui->envButton, &QPushButton::clicked, [this] () {
        EnvironmentVariablesDialog dialog(this);
        dialog.setEnvironmentVariables(_environment);
        if (dialog.exec() == QDialog::Accepted) {
            _environment = dialog.environmentVariables();
        }
    });

    // パスが入力された場合にOKボタンは有効
    connect(_ui->pathLineEdit, &QLineEdit::textChanged, [&]() {
        auto *okButton = _ui->buttonBox->button(QDialogButtonBox::Ok);
        okButton->setEnabled(!_ui->pathLineEdit->text().trimmed().isEmpty());
    });
    // 最初はOKボタン無効
    auto *okButton = _ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(!_ui->pathLineEdit->text().trimmed().isEmpty());
}


QString BrowseEngineDialog::enginePath()
{
    return _ui->pathLineEdit->text();
}


void BrowseEngineDialog::open()
{
    QDialog::open();
}


void BrowseEngineDialog::accept()
{
    QDialog::accept();
}
