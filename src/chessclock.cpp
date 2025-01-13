#include "chessclock.h"
#include "global.h"
#include "sound.h"
#include <QTimerEvent>
#include <QDebug>


ChessClock::ChessClock(QObject *parent) :
    QObject(parent)
{ }

// ミリ秒
void ChessClock::setTime(maru::TimeMethod method, int basicTime, int byo)
{
    _method = method;

    if (method == maru::Byoyomi) {
        // 秒読み
        _senteTime = basicTime + byo;
        _goteTime = basicTime + byo;
        _byoyomi = byo;
        _incTime = 0;
    } else {
        // フィッシャー
        _senteTime = basicTime;
        _goteTime = basicTime;
        _byoyomi = 0;
        _incTime = byo;
    }
}

// 残り考慮時間（秒読み分を含まず）
int ChessClock::remainingTime(maru::Turn player)
{
    int elapsed = (player == _currentTurn) ? _elapsedTime.elapsed() : 0;
    int time = (player == maru::Sente) ? _senteTime : _goteTime;
    return std::max(time - _byoyomi - elapsed, 0);
}

// 残り秒読み時間（秒読み分のみ）
int ChessClock::remainingSeconds(maru::Turn player)
{
    int elapsed = (player == _currentTurn) ? _elapsedTime.elapsed() : 0;
    int time = (player == maru::Sente) ? _senteTime : _goteTime;
    return qBound(0, time - elapsed, _byoyomi);
}

// 残り時間（考慮時間 + 秒読み分）
int ChessClock::remaining(maru::Turn player)
{
    int elapsed = (player == _currentTurn) ? _elapsedTime.elapsed() : 0;
    int time = (player == maru::Sente) ? _senteTime : _goteTime;
    return std::max(0, time - elapsed);
}


void ChessClock::start()
{
    _elapsedTime.start();
    if (_secTimerId > 0) {
        qWarning() << "Start again! [Sec timer]";
        killTimer(_secTimerId);
    }

    if (_timeupTimerId > 0) {
        qWarning() << "Start again! [Time up timer]";
        killTimer(_timeupTimerId);
    }

    _secTimerId = startTimer(1000);
    _nextSecTime = maru::currentMSecsSinceStart() + 1000;
    _timeupTimerId = startTimer(remaining(_currentTurn));
}

void ChessClock::stop()
{
    stopTimer();
}


void ChessClock::stopTimer()
{
    if (_secTimerId > 0) {
        killTimer(_secTimerId);
        _secTimerId = 0;
    }

    if (_timeupTimerId > 0) {
        killTimer(_timeupTimerId);
        _timeupTimerId = 0;
    }
}


maru::Turn ChessClock::changeTurn()
{
    int elapsed = _elapsedTime.restart();

    if (_currentTurn == maru::Sente) {
        _senteTime += _incTime - elapsed;
        _senteTime = std::max(_senteTime, _byoyomi);
        _currentTurn = maru::Gote;
    } else {
        _goteTime += _incTime - elapsed;
        _goteTime = std::max(_goteTime, _byoyomi);
        _currentTurn = maru::Sente;
    }
    //qDebug() << "btime:" << _senteTime << " wtime:" << _goteTime;

    // タイマー更新
    stopTimer();
    int t = (remainingTime(_currentTurn) > 0) ? remainingTime(_currentTurn) : remainingSeconds(_currentTurn);
    t = (t % 1000) + 1;
    _secTimerId = startTimer(t);
    _nextSecTime = maru::currentMSecsSinceStart() + t;
    _timeupTimerId = startTimer(remaining(_currentTurn));

    return _currentTurn;
}


void ChessClock::timerEvent(QTimerEvent *event)
{
    static const QList<int> bleepTimes = {20, 10, 5, 4, 3, 2, 1};

    if (event->timerId() == _secTimerId) {
        // 秒タイマー
        emit secondElapsed();  // per 1sec

        if (_bleepEnable) {
            // ビープ音
            constexpr int correctMsecs = 150;
            int sec = (remaining(_currentTurn) + correctMsecs) / 1000;
            if (bleepTimes.contains(sec)) {
                Sound::playBleep();
            }
        }

        // 秒タイマー補正
        killTimer(_secTimerId);
        int64_t currentTimestamp = maru::currentMSecsSinceStart();
        int64_t adjustment;
        do {
            _nextSecTime += 1000;
            adjustment = _nextSecTime - currentTimestamp;  // 次までの補正値
        } while (adjustment < 0);
        _secTimerId = startTimer(adjustment);

        // 時間切れタイマー補正
        killTimer(_timeupTimerId);
        _timeupTimerId = startTimer(remaining(_currentTurn));

    } else if (event->timerId() == _timeupTimerId) {
        // 時間切れタイマー
        emit timeout(_currentTurn);

    } else {
        QObject::timerEvent(event);
    }
}
