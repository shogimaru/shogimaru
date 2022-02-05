#include "recorddialog.h"
#include "messagebox.h"
#include "ui_recorddialog.h"
#include "westerntabstyle.h"
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>

static WesternTabStyle westernTabStyle;


RecordDialog::RecordDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::RecordDialog)
{
    _ui->setupUi(this);
    _ui->tabWidget->tabBar()->setStyle(&westernTabStyle);

    connect(_ui->loadTextButton, &QPushButton::clicked, this, &RecordDialog::loadNotation);
    connect(_ui->fileOpenButton, &QPushButton::clicked, this, &RecordDialog::openRecordFile);
    connect(_ui->loadFileButton, &QPushButton::clicked, this, &RecordDialog::loadRecordFile);
    connect(_ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);  // 閉じるボタン
}


void RecordDialog::loadNotation()
{
    if (!validate(_ui->textEdit->toPlainText().trimmed())) {
        MessageBox::information(tr("Notation Error"), tr("Load Error"));
        return;
    }
    QDialog::accept();
}


void RecordDialog::openRecordFile()
{
    auto filePath = QFileDialog::getOpenFileName(this, tr("Open Record File"), QDir::homePath(), tr("Record File (*.csa *.sfen *.txt)"));
    _ui->labelRecordFilePath->setText(QFileInfo(filePath).fileName());
    _loadFilePath = filePath;
}


void RecordDialog::loadRecordFile()
{
    auto isReadable = [](const QString &str) {
        for (auto c : str) {
            if (c.category() < 3 || c.category() > 27) {
                return false;
            }
        }
        return true;
    };

    if (_loadFilePath.isEmpty()) {
        return;
    }

    QFile file(_loadFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        auto buf = file.readAll();
        file.close();

        QString str = QTextCodec::codecForName("Shift-JIS")->toUnicode(buf);
        if (!isReadable(str)) {
            str = QString::fromUtf8(buf);
        }

        if (!validate(str)) {
            MessageBox::information(tr("Notation Error"), tr("Load Error"));
        } else {
            QDialog::accept();
        }
    }

    _ui->labelRecordFilePath->clear();
    _loadFilePath.clear();
}


bool RecordDialog::validate(const QString &notation)
{
    bool ok;
    _sfen = Sfen::fromCsa(notation, &ok);  // CSA
    if (ok) {
        return ok;
    }

    _sfen = Sfen::fromSfen(notation.toLatin1(), &ok);  // SFEN
    return ok;
}


void RecordDialog::open()
{
    _ui->textEdit->clear();
    _sfen.clear();
    QDialog::open();
}
