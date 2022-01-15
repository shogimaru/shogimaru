#pragma once
#include "global.h"
#include <QDialog>

namespace Ui {
class AnalysisDialog;
}


class AnalysisDialog : public QDialog {
    Q_OBJECT
public:
    enum Scope : int {
        All,
        FromCurrentPosition,
    };

    explicit AnalysisDialog(QWidget *parent = nullptr);
    ~AnalysisDialog();

    Scope scope() const;

public slots:
    void open() override;
    void accept() override;

signals:
    void accepted();
    void rejected();

private:
    Ui::AnalysisDialog *_ui {nullptr};
};
