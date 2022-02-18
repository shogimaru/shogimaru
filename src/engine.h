#pragma once
#include "global.h"
#include "ponderinfo.h"
#include <QObject>
#include <QTimer>

class EngineThread;
class EngineProcess;


class Engine : public QObject
{
    Q_OBJECT
public:
    virtual ~Engine();

    void init();
    QByteArray startPosition() const { return _startPositionSfen; }
    void setStartPosition(const QByteArray &sfen = QByteArray());
    QByteArrayList allMoves() const { return _allMoves; }  // 全指し手（SFEN形式）
    void setSkillLevel(int level) { _level = level; }
    bool newGame(int slowMover = 100);
    void gameover();
    bool go(const QByteArrayList &position, int senteTime, int goteTime, int byoyomi);  // 考慮開始
    bool ponder(int senteTime, int goteTime, int byoyomi);  // 先読み開始
    void stop();    // 考慮中止
    bool mated(const QByteArrayList &moves);  // 局面が詰んでいるか
    bool mated(const QByteArray &startPosition, const QByteArrayList &moves = QByteArrayList());
    QByteArray lastPondered() const { return _lastPondered; }
    maru::Turn currentTurn() const { return _turn; }

    // 棋譜解析
    bool startAnalysis();
    bool analysis(const QByteArray &sfen);

    static Engine &instance();

signals:
    void bestMove(const QByteArray &best);  // 指し手
    void pondering(const PonderInfo &info);
    void resign();  // 投了
    void win();  // 入玉勝ち宣言

protected slots:
    void getResponse();

private:
    void start();
    bool go(const QByteArrayList &position, bool ponderFlag, int senteTime, int goteTime, int byoyomi);
    void setTurn();

    enum State : int {
        EngineStarting,
        GameReady,  // 対局OK（初期化終了）
        Idle,       // 対局中アイドル
        Going,      // 考慮中
        Pondering,  // 先読み中
        EngineError,
    };

    Engine(QObject *parent = nullptr);
    int _level {20};
    State _state {EngineStarting};
    QTimer *_timer {nullptr};  // ポーリングタイマー
    QByteArray _startPositionSfen;
    QByteArrayList _allMoves;
    maru::Turn _turn {maru::Sente};
    QByteArrayList _ponderingMoves;
    QByteArray _lastPondered;
#ifdef Q_OS_WASM
    EngineThread *_engineThread {nullptr};
#endif
};
