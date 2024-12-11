#pragma once
#include "global.h"
#include "player.h"
#include <QDialog>
#include <QTime>

class Board;
class ChessClock;
class Engine;

namespace Ui {
class StartDialog;
}


class StartDialog : public QDialog {
    Q_OBJECT
public:
    explicit StartDialog(QWidget *parent = nullptr);
    ~StartDialog() {}

    void open();
    void accept();
    maru::StartPosision position() const;
    maru::PlayerType player(maru::Turn turn) const;

private:
    int byoyomi() const;  // 秒読み
    int basicTime() const;  // 持ち時間(分)
    maru::TimeMethod method() const;

    Ui::StartDialog *_ui {nullptr};
};
