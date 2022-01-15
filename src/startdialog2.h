#pragma once
#include "global.h"
#include <QDialog>
#include <QTime>

class Board;
class ChessClock;
class Engine;

namespace Ui {
class StartDialog2;
}


class StartDialog2 : public QDialog {
    Q_OBJECT
public:
    explicit StartDialog2(QWidget *parent = nullptr);
    ~StartDialog2() {}

    void open();
    void accept();
    int byoyomi() const;  // 秒読み
    int basicTime() const;  // 持ち時間(分)

private:
    Ui::StartDialog2 *_ui {nullptr};
};
