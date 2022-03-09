#pragma once
#include "global.h"
#include "ponderinfo.h"
#include <QObject>
#include <QTimer>
#include <QVariant>

class EngineThread;
class EngineProcess;


class Engine : public QObject {
    Q_OBJECT
public:
    struct Option {
        QVariant value;
        QMetaType::Type type {QMetaType::Void};
        qint64 max {0};
        qint64 min {0};
    };

    struct EngineInfo {
        QString name;
        QString path;
        QString author;
        QMap<QString, Option> options;
    };

    class EngineContext {
    public:
        virtual ~EngineContext() { }
        virtual void start() = 0;
        virtual void terminate() { }
    };

    virtual ~Engine();

    bool open(const QString &path);
    void close();

    QString name() const { return _name; }
    QString author() const { return _author; }
    const QVariantMap &options() const { return _options; }
    QVariantMap &options() { return _options; }
    void setOptions(const QVariantMap &options) { _options = options; }
    QByteArray startPosition() const { return _startPositionSfen; }
    void setStartPosition(const QByteArray &sfen = QByteArray());
    QByteArrayList allMoves() const { return _allMoves; }  // 全指し手（SFEN形式）
    void setSkillLevel(int level) { _level = level; }
    bool newGame(int slowMover = 100);
    void gameover();
    void quit();
    bool go(const QByteArrayList &position, int senteTime, int goteTime, int byoyomi);  // 考慮開始
    bool ponder(int senteTime, int goteTime, int byoyomi);  // 先読み開始
    void stop();  // 考慮中止
    bool mated(const QByteArrayList &moves);  // 局面が詰んでいるか
    bool mated(const QByteArray &startPosition, const QByteArrayList &moves = QByteArrayList());
    QByteArray lastPondered() const { return _lastPondered; }
    maru::Turn currentTurn() const { return _turn; }
    bool hasSkillLevelOption() const;
    QMetaType::Type optionType(const QString &option) const;
    QString error() const { return _error; }

    // 棋譜解析
    bool startAnalysis();
    bool analysis(const QByteArray &sfen);

    static Engine &instance();
    static EngineInfo getEngineInfo(const QString &path);

signals:
    void bestMove(const QByteArray &best);  // 指し手
    void pondering(const PonderInfo &info);
    void resign();  // 投了
    void win();  // 入玉勝ち宣言

protected slots:
    void getResponse();

private:
    void openUsi(const QString &path);  // start process/thread
    void closeUsi();
    bool go(const QByteArrayList &position, bool ponderFlag, int senteTime, int goteTime, int byoyomi);
    void setTurn();
    void sendOptions(const QVariantMap &options);

    enum State : int {
        NotRunning,
        GameReady,  // 対局OK（初期化終了）
        Idle,  // 対局中アイドル
        Going,  // 考慮中
        Pondering,  // 先読み中
        EngineError,
    };

    Engine(QObject *parent = nullptr);

    QString _enginePath;
    QString _name;
    QString _author;
    QMap<QString, Option> _defaultOptions;  // デフォルトオプション
    QVariantMap _options;
    int _level {20};
    State _state {NotRunning};
    QTimer *_timer {nullptr};  // ポーリングタイマー
    QByteArray _startPositionSfen;
    QByteArrayList _allMoves;
    maru::Turn _turn {maru::Sente};
    QByteArrayList _ponderingMoves;
    QByteArray _lastPondered;
    QString _error;
    EngineContext *_engineContext {nullptr};
};
