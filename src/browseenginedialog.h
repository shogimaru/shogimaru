#pragma once
#include "global.h"
#include <QDialog>


namespace Ui {
class BrowseEngineDialog;
}


class BrowseEngineDialog : public QDialog {
    Q_OBJECT
public:
    explicit BrowseEngineDialog(QWidget *parent = nullptr);
    ~BrowseEngineDialog() { }

    void open();
    void accept();
    QString enginePath();
    QVariantList environmentVariables() const { return _environment; }

private:
    Ui::BrowseEngineDialog *_ui {nullptr};

    QString _enginePath;
    QVariantList _environment;
};
