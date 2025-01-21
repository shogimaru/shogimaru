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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>


RecordDialog::RecordDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::RecordDialog)
{
    _ui->setupUi(this);

    static WesternTabStyle westernTabStyle;
    _ui->tabWidget->setElideMode(Qt::ElideNone);
    _ui->tabWidget->tabBar()->setStyle(&westernTabStyle);
    _ui->listWidget->setSelectionMode(QAbstractItemView::NoSelection);

    connect(_ui->listWidget, &QListWidget::itemClicked, this, &RecordDialog::loadItem);
    connect(_ui->loadTextButton, &QPushButton::clicked, this, &RecordDialog::loadRecord);
    connect(_ui->loadUrlButton, &QPushButton::clicked, this, &RecordDialog::loadUrlRecord);
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


void RecordDialog::loadUrlRecord()
{
    auto url = _ui->urlLineEdit->text().trimmed();
    if (url.isEmpty()) {
        return;
    }

#ifdef Q_OS_WASM
    url.remove(QRegularExpression(R"(^https?://)"));
    url.prepend("https://shogimaru.com/url/");
#endif

    request(url, &RecordDialog::parseRecord);
}


void RecordDialog::parseRecord()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << reply->errorString();
        return;
    }

    auto kifu = reply->readAll();
    // SJISでトライ
    QString str = maru::fromShiftJis(kifu);
    if (!isReadable(str)) {  // 文字化けならUTF-8で読み込む
        str = QString::fromUtf8(kifu);
    }

    if (!validate(str)) {
        MessageBox::information(tr("Notation Error"), tr("Load Error"));
        return;
    }

    QDialog::accept();
}

// 文字化け有無
bool RecordDialog::isReadable(const QString &text)
{
    for (auto c : text) {
        if (c.category() < 3 || c.category() > 27) {
            return false;
        }
    }
    return !text.isEmpty();
};


void RecordDialog::openFile()
{
    auto fileContentReady = [this](const QString &, const QByteArray &fileContent) {
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


template <typename Func>
void RecordDialog::request(const QString &url, Func slot)
{
    auto *manager = new QNetworkAccessManager(this);
    auto *reply = manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::errorOccurred, [this, url, slot, reply] {
        qCritical() << "Network reply error " << reply->errorString();
        if (_errorCount++ < 1) {
            // 1回だけリトライ
            request(url, slot);
        }
    });
    connect(reply, &QNetworkReply::finished, this, slot);
    connect(manager, &QNetworkAccessManager::finished, reply, &QNetworkReply::deleteLater);
    connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);
}


void RecordDialog::open()
{
    constexpr int NUM_RECORDS = 100;
    const QString ShogiDbUrl = "https://api.shogidb2.com/latest?offset=0&limit=%1";

    auto url = ShogiDbUrl.arg(NUM_RECORDS);
#ifdef Q_OS_WASM
    url.remove(QRegularExpression(R"(^https?://)"));
    url.prepend("https://shogimaru.com/url/");
#endif

    // HTTP request
    request(url, &RecordDialog::parseJsonArray);

    _ui->textEdit->clear();
    _sfen.clear();
    _ui->listWidget->setEnabled(true);
    QDialog::open();
}


void RecordDialog::parseJsonArray()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (!reply) {
        qCritical() << "Logic Error " << __FILE__ << __LINE__;
        return;
    }

    _errorCount = 0;
    _ui->listWidget->clear();
    auto body = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(body);
    auto array = doc.array();

    if (doc.isEmpty()) {
        qCritical() << "JSON empty " << __FILE__ << __LINE__;
        return;
    }

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
    auto obj = item->data(Qt::UserRole).toJsonObject();
    QString hash = obj.value("hash").toString();
    readRecord(hash);
}


void RecordDialog::readRecord(const QString &hash)
{
    const QString Db2Url("https://api.shogidb2.com/games/%1");

    auto url = Db2Url.arg(hash);
#ifdef Q_OS_WASM
    url.remove(QRegularExpression(R"(^https?://)"));
    url.prepend("https://shogimaru.com/url/");
#endif

    if (!hash.isEmpty()) {
        // HTTP request
        request(url, &RecordDialog::parseRecordJson);
        _ui->listWidget->setEnabled(false);
    }
}


void RecordDialog::parseRecordJson()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (!reply) {
        qCritical() << "Logic Error " << __FILE__ << __LINE__;
        return;
    }

    _errorCount = 0;
    _ui->listWidget->setEnabled(true);
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << reply->errorString();
        return;
    }

    auto body = reply->readAll();
    if (body.isEmpty()) {
        qCritical() << "No data. " << __FILE__ << __LINE__;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(body);
    QJsonObject json = doc.object();
    auto array = json.value("moves").toArray();
    if (array.count() == 0) {
        MessageBox::information(tr("Error"), tr("Failed to retrieve the Shogi game record."));
        return;
    }

    QString csa;
    csa += QLatin1String("$EVENT:");
    csa += json.value("tournament_detail").toString();
    csa += "\n";

    for (auto it = array.begin(); it != array.end(); ++it) {
        auto obj = it->toObject();
        // 指し手
        QString str = obj.value("csa").toString();
        csa += str;
        csa += "\n";
    }

    if (validate(csa)) {
        _sfen.setPlayers(json.value("player1").toString(), json.value("player2").toString());
        accept();
    } else {
        MessageBox::information(tr("Notation Error"), tr("Load Error"));
    }
}
