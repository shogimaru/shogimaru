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
    enum StartPosision : int {
        Initial = 0,  // 初期局面
        Current,      // 現在の局面
    };

    explicit StartDialog(QWidget *parent = nullptr);
    ~StartDialog() {}

    StartPosision postion() const;
    maru::PlayerType player(maru::Turn turn) const;
    int byoyomi() const;  // 秒読み
    QTime time() const;  // 持ち時間(秒)

private:
    Ui::StartDialog *_ui {nullptr};
};
