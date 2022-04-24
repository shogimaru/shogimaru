#include "chessclock.h"
#include "sound.h"
#include <QDebug>
#include <QTimerEvent>
using namespace maru;


ChessClock::ChessClock(QObject *parent) :
    QObject(parent)
{
}

// ミリ秒
void ChessClock::setTime(maru::TimeMethod method, int basicTime, int byo)
{
    _method = method;

    if (method == maru::Byoyomi) {
        _senteTime = basicTime + byo;
        _goteTime = basicTime + byo;
        _byoyomi = byo;
        _incTime = 0;
    } else {
        _senteTime = basicTime;
        _goteTime = basicTime;
        _byoyomi = 0;
        _incTime = byo;
    }
}

// 残り考慮時間（秒読み分を含まず）
int ChessClock::remainingTime(Turn player)
{
    int elapsed = (player == _currentTurn) ? _elapsedTimer.elapsed() : 0;
    int time = (player == Sente) ? _senteTime : _goteTime;
    return qMax(time - _byoyomi - elapsed, 0);
}

// 残り時間（秒読み分のみ）
int ChessClock::remainingSeconds(maru::Turn player)
{
    int elapsed = (player == _currentTurn) ? _elapsedTimer.elapsed() : 0;
    int time = (player == Sente) ? _senteTime : _goteTime;
    return qBound(0, time - elapsed, _byoyomi);
}


int ChessClock::remaining(maru::Turn player)
{
    int elapsed = (player == _currentTurn) ? _elapsedTimer.elapsed() : 0;
    int time = (player == Sente) ? _senteTime : _goteTime;
    return qMax(0, time - elapsed);
}


void ChessClock::start(Turn startTurn)
{
    _currentTurn = startTurn;
    _elapsedTimer.start();
    //startTimer();
    _timerId = startTimer(1000 / 5);
}

void ChessClock::stop()
{
    stopTimer();
}


void ChessClock::stopTimer()
{
    // _timeoutTimer.stop();
    // _bleepTimer.stop();

    if (_timerId > 0) {
        killTimer(_timerId);
        _timerId = 0;
    }
}


maru::Turn ChessClock::changeTurn()
{
    //stopTimer();

    int elapsed = _elapsedTimer.restart();

    if (_currentTurn == Sente) {
        _senteTime += _incTime - elapsed;
        _senteTime = qMax(_senteTime, _byoyomi);
        _currentTurn = Gote;
    } else {
        _goteTime += _incTime - elapsed;
        _goteTime = qMax(_goteTime, _byoyomi);
        _currentTurn = Sente;
    }

    //qDebug() << "btime:" << _senteTime << " wtime:" << _goteTime;
    return _currentTurn;
}


void ChessClock::timerEvent(QTimerEvent *event)
{
    constexpr int offset = 100;
    static const QList<int> bleepTimes = {20000, 10000, 5000, 4000, 3000, 2000, 1000};

    if (event->timerId() == _timerId) {
        int remain = remaining(_currentTurn);

        if (_elapsedTemp != remain / 1000) {
            _elapsedTemp = remain / 1000;
            emit secondElapsed();  // per 1 sec
        }

        if (remain == 0) {
            emit timeout(_currentTurn);
            emit secondElapsed();
            return;
        }

        if (_bleepEnable) {
            for (int d : bleepTimes) {
                d += offset;
                if (_time > d && d >= remain) {
                    Sound::playBleep();
                    break;
                }
            }
            _time = remain;
        }
    } else {
        QObject::timerEvent(event);
    }
}
