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
    connect(_ui->fileOpenButton, &QPushButton::clicked, this, &RecordDialog::openFile);
    connect(_ui->saveButton, &QPushButton::clicked, this, &RecordDialog::selectSaveFile);
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


void RecordDialog::openFile()
{
    auto fileContentReady = [this](const QString &, const QByteArray &fileContent) {
        // 文字化け有無
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
    // ファイルダイアログ
    QFileDialog::getOpenFileContent("*", fileContentReady);
}


void RecordDialog::selectSaveFile()
{
#if __EMSCRIPTEN__
    QString fileName = "untitled.csa";
#else
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "untitled.csa");
#endif
    close();
    emit saveFileSelected(fileName);
}


bool RecordDialog::validate(const QString &record)
{
    bool ok;

    _sfen = Sfen::fromCsa(record, &ok);  // CSA
    if (ok) {
        return ok;
    }

    _sfen = Sfen::fromSfen(record.toLatin1(), &ok);  // SFEN
    return ok;
}


void RecordDialog::open()
{
    _ui->textEdit->clear();
    _sfen.clear();
    QDialog::open();
}
