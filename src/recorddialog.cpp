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

    connect(_ui->loadTextButton, &QPushButton::clicked, this, &RecordDialog::loadRecord);
    connect(_ui->fileOpenButton, &QPushButton::clicked, this, &RecordDialog::openFileDialog);
    connect(_ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);  // 閉じるボタン
}


void RecordDialog::loadRecord()
{
    if (!validate(_ui->textEdit->toPlainText().trimmed())) {
        MessageBox::information(tr("Notation Error"), tr("Load Error"));
        return;
    }
    QDialog::accept();
}


void RecordDialog::openFileDialog()
{
    auto fileContentReady = [this](const QString &, const QByteArray &fileContent) {
        auto isReadable = [](const QString &str) {
            for (auto c : str) {
                if (c.category() < 3 || c.category() > 27) {
                    return false;
                }
            }
            return true;
        };

        if (!fileContent.isEmpty()) {
            // SJISでトライ
            QString str = QTextCodec::codecForName("Shift-JIS")->toUnicode(fileContent);
            if (!isReadable(str)) {  // 文字化けならUTF-8で読み込む
                str = QString::fromUtf8(fileContent);
            }

            if (!this->validate(str)) {
                MessageBox::information(tr("Notation Error"), tr("Load Error"));
            } else {
                this->accept();
            }
        }
    };
    QFileDialog::getOpenFileContent("*", fileContentReady);
}


void RecordDialog::loadRecordFile(const QString &filePath)
{
    auto isReadable = [](const QString &str) {
        for (auto c : str) {
            if (c.category() < 3 || c.category() > 27) {
                return false;
            }
        }
        return true;
    };

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        auto buf = file.readAll().trimmed();
        file.close();

        QString str = QTextCodec::codecForName("Shift-JIS")->toUnicode(buf);
        if (!isReadable(str)) {
            str = QString::fromUtf8(buf);
        }

        if (!validate(str)) {
            MessageBox::information(tr("Notation Error"), tr("Load Error"));
        } else {
            this->accept();
        }
    }
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
