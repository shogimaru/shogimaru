#pragma once
#include "global.h"
#include <QObject>
#include <QElapsedTimer>
//#include <QBasicTimer>


class ChessClock : public QObject {
    Q_OBJECT
public:
    ChessClock(QObject *parent = nullptr);

    void setTime(int basicTime, int byoyomi);
    maru::Turn currentTurn() const { return _currentTurn; }
    int byoyomi() const { return _byoyomi; }
    int remainingTime(maru::Turn player);  // 残り考慮時間（秒読み分を含まず）
    int remainingSeconds(maru::Turn player);  // 残り時間（秒読み分のみ）
    void setBleepEnabled(bool enable) { _bleepEnable = enable; }

public slots:
    void start(maru::Turn startTurn);
    void stop();
    maru::Turn changeTurn();

signals:
    void timeout(maru::Turn player);
    void secondElapsed();  // 1秒経過

protected:
    int remaining(maru::Turn player);  // 残り時間（考慮時間 + 秒読み分）
    //void startTimer();
    void stopTimer();
    //void setBleepTimer(int remaining);
    void timerEvent(QTimerEvent *event) override;

private:
    int _senteTime {0};
    int _goteTime {0};
    int _byoyomi {0};
    QElapsedTimer _elapsedTimer;
    maru::Turn _currentTurn {maru::Sente};
    //QBasicTimer _timeoutTimer;
    bool _bleepEnable {true};
    int _time {0};
    int _elapsedTemp {0};
    int _timerId {0};
};
