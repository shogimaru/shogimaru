#pragma once
#include "global.h"
#include <QWidget>
#include <QTime>

namespace Ui {
class OperationButtonGroup;
}


class OperationButtonGroup : public QWidget {
    Q_OBJECT
public:
    explicit OperationButtonGroup(QWidget *parent = nullptr);
    ~OperationButtonGroup() { }

    void setEnabled(bool enable);

signals:
    void next();
    void next10Moves();
    void nextCandidate();
    void previous();
    void previous10Moves();
    void previousCandidate();
    void firstPosition();
    void lastPosition();

private:
    Ui::OperationButtonGroup *_ui {nullptr};
};
