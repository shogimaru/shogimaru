#pragma once
#include "global.h"
#include "sfen.h"
#include <QDialog>

class QNetworkAccessManager;
class QNetworkReply;
class QListWidgetItem;

namespace Ui {
class RecordDialog;
}


class RecordDialog : public QDialog {
    Q_OBJECT
public:
    explicit RecordDialog(QWidget *parent = nullptr);
    ~RecordDialog();

    void open();
    void openRecord(const QString &hash);
    Sfen result() const { return _sfen; }
    void loadRecord();
    void loadUrlRecord();
    void openFile();
    void selectSaveFile();
    void parseRecord();
    void parseJsonArray();
    void parseRecordJson();
    void loadItem(QListWidgetItem *item);
    void readRecord(const QString &hash);

signals:
    void saveFileSelected(const QString &filePath);

private:
    bool isReadable(const QString &text);
    bool validate(const QString &record);
    template <typename Func> void request(const QString &url, Func slot);

    Ui::RecordDialog *_ui {nullptr};
    Sfen _sfen;
};
