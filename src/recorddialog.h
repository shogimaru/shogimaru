#pragma once
#include "global.h"
#include "sfen.h"
#include <QDialog>

namespace Ui {
class RecordDialog;
}


class RecordDialog : public QDialog {
    Q_OBJECT
public:
    explicit RecordDialog(QWidget *parent = nullptr);
    ~RecordDialog() { }

    void open();
    Sfen result() const { return _sfen; }
    void loadRecord();

public slots:
    void openFile();
    void selectSaveFile();

signals:
    void saveFileSelected(const QString &filePath);

private:
    bool validate(const QString &record);

    Ui::RecordDialog *_ui {nullptr};
    Sfen _sfen;
};
