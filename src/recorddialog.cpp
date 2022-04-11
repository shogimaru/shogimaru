#include "recorddialog.h"
#include "messagebox.h"
#include "stringconverter.h"
#include "ui_recorddialog.h"
#include "westerntabstyle.h"
#include <QDebug>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaMethod>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

static WesternTabStyle westernTabStyle;


RecordDialog::RecordDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::RecordDialog)
{
    _ui->setupUi(this);
    _ui->tabWidget->tabBar()->setStyle(&westernTabStyle);
    _ui->listWidget->setSelectionMode(QAbstractItemView::NoSelection);

    connect(_ui->listWidget, &QListWidget::itemClicked, this, &RecordDialog::loadItem);
    connect(_ui->loadTextButton, &QPushButton::clicked, this, &RecordDialog::loadRecord);
    connect(_ui->fileOpenButton, &QPushButton::clicked, this, &RecordDialog::openFile);
    connect(_ui->saveButton, &QPushButton::clicked, this, &RecordDialog::selectSaveFile);
    connect(_ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);  // 閉じるボタン

    // 引用
    _ui->labelWeb->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    _ui->labelWeb->setOpenExternalLinks(true);
    _ui->labelWeb->setTextFormat(Qt::RichText);
    _ui->labelWeb->setText(tr("Citing") + ": <a href='https://shogidb2.com/'>将棋DB2</a>");
    _ui->tabWidget->setCurrentIndex(0);

    // スタイル
    _ui->listWidget->setStyleSheet("QListWidget::item {border-bottom: 1px solid #ddd; padding: 6px 2px;}");
}


RecordDialog::~RecordDialog()
{
    delete _ui;
}


void RecordDialog::loadRecord()
{
    auto str = _ui->textEdit->toPlainText().trimmed();
    if (str.isEmpty()) {
        return;
    }

    if (!validate(str)) {
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
            return !str.isEmpty();
        };

        if (!fileContent.isEmpty()) {
            // SJISでトライ
            QString str = maru::fromShiftJis(fileContent);
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
#ifdef Q_OS_WASM
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


void RecordDialog::request(const QString &url, const char *method)
{
    QUrl reqestUrl(url);
    auto *manager = new QNetworkAccessManager(this);
    auto *reply = manager->get(QNetworkRequest(reqestUrl));
    connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);
    connect(reply, SIGNAL(finished()), this, method);
}


void RecordDialog::open()
{
    constexpr int NUM_RECORDS = 50;
    const QString ShogiDbUrl = "https://api.shogidb2.com/latest?offset=0&limit=%1";

    if (_ui->listWidget->count() == 0) {
        // HTTP request
        request(ShogiDbUrl.arg(NUM_RECORDS), SLOT(parseJsonArray()));
    }

    _ui->textEdit->clear();
    _sfen.clear();
    _ui->listWidget->setEnabled(true);
    QDialog::open();
}


void RecordDialog::parseJsonArray()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << reply->errorString();
        return;
    }

    _ui->listWidget->clear();

    auto body = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(body);
    auto array = doc.array();

    for (auto it = array.begin(); it != array.end(); ++it) {
        auto obj = it->toObject();

        QString str;
        auto date = QDate::fromString(obj.value("start_at").toString().mid(0, 10), "yyyy-MM-dd");
        str += date.toString("MM/dd ");
        str += obj.value("player1").toString().trimmed().mid(0, 16);
        str += tr(" vs ");
        str += obj.value("player2").toString().trimmed().mid(0, 16);
        str += " ";
        str += obj.value("tournament_detail").toString().trimmed().mid(0, 32);
        auto *item = new QListWidgetItem(str, _ui->listWidget);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setData(Qt::UserRole, obj);
        _ui->listWidget->addItem(item);
    }
}


void RecordDialog::loadItem(QListWidgetItem *item)
{
    const QString Url("https://api.shogidb2.com/eval/%1/default.json");

    auto obj = item->data(Qt::UserRole).toJsonObject();
    int id = obj.value("id").toInt();
    // HTTP request
    request(Url.arg(id), SLOT(parseRecordJson()));
    //qDebug() << Url.arg(id);
    _ui->listWidget->setEnabled(false);
}


void RecordDialog::parseRecordJson()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    _ui->listWidget->setEnabled(true);
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << reply->errorString();
        return;
    }

    auto body = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(body);
    auto array = doc.object().value("evals").toArray();
    if (array.count() == 0) {
        MessageBox::information(tr("Error"), tr("Failed to retrieve the Shogi game record."));
        return;
    }

    QString csa;
    for (auto it = array.begin(); it != array.end(); ++it) {
        auto obj = it->toObject();
        if (obj.value("index").toInt() > 0) {
            QString str = obj.value("csa").toString();
            if (str.isEmpty()) {
                csa += "%TORYO\n";
                break;
            }
            csa += str;
            csa += "\n";
        }
    }

    if (validate(csa)) {
        if (_ui->listWidget->currentItem()) {
            auto obj = _ui->listWidget->currentItem()->data(Qt::UserRole).toJsonObject();
            _sfen.setPlayers(obj.value("player1").toString(), obj.value("player2").toString());
        }
        accept();
    } else {
        MessageBox::information(tr("Notation Error"), tr("Load Error"));
    }
}
