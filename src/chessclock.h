#pragma once
#include "global.h"
#include <QObject>
#include <QElapsedTimer>
//#include <QBasicTimer>


class ChessClock : public QObject {
    Q_OBJECT
public:
    ChessClock(QObject *parent = nullptr);

    void setTime(maru::TimeMethod mode, int basicTime, int byo);
    maru::Turn currentTurn() const { return _currentTurn; }
    maru::TimeMethod method() const { return _method; }
    int byoyomi() const { return _byoyomi; }
    int incrementTime() const { return _incTime; }  // 加算時間
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
    maru::TimeMethod _method {maru::Byoyomi};
    int _senteTime {0};
    int _goteTime {0};
    int _byoyomi {0};
    int _incTime {0};
    QElapsedTimer _elapsedTimer;
    maru::Turn _currentTurn {maru::Sente};
    //QBasicTimer _timeoutTimer;
    bool _bleepEnable {true};
    int _time {0};
    int _elapsedTemp {0};
    int _timerId {0};
};
