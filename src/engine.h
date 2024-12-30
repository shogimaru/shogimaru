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
        QMetaType::Type type {QMetaType::Void};
        QVariant defaultValue;  // 初期値
        QVariant value;  // 現在値
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

    enum State : int {
        NotRunning = 0,
        GameReady,  // 対局OK（初期化終了）
        Idle,  // 対局中アイドル
        Going,  // 考慮中
        Pondering,  // 先読み中
        EngineError,
    };

    virtual ~Engine();

    bool open(const QString &path);
    void close();
    bool isOpen() const;

    QString name() const { return _name; }
    QString author() const { return _author; }
    QString shortName() const;
    const QVariantMap &options() const { return _options; }
    QVariantMap &options() { return _options; }
    void setOptions(const QVariantMap &options) { _options = options; }
    QByteArray startPosition() const { return _startPositionSfen; }
    void setStartPosition(const QByteArray &sfen = QByteArray());
    QByteArrayList allMoves() const { return _allMoves; }  // 全指し手（SFEN形式）
    void setSkillLevel(int level) { _level = qBound(0, level, 20); }
    bool newGame(int slowMover = 0);
    void gameover();
    void quit();
    bool go(const QByteArrayList &position, int senteTime, int goteTime, int byoyomi, int incTime);  // 考慮開始
    bool ponder(int senteTime, int goteTime, int byoyomi, int incTime);  // 先読み開始
    void stop();  // 考慮中止
    bool mated(const QByteArrayList &moves);  // 局面が詰んでいるか
    bool mated(const QByteArray &startPosition, const QByteArrayList &moves = QByteArrayList());
    QByteArray lastPondered() const { return _lastPondered; }
    maru::Turn currentTurn() const { return _turn; }
    bool hasSkillLevelOption() const;
    QMetaType::Type type(const QString &option) const;
    QString error() const { return _error; }
    State state() const { return _state; }

    // 棋譜解析
    bool startAnalysis();
    bool analysis(const QByteArray &sfen);

    static Engine &instance();
    static EngineInfo getEngineInfo(const QString &path);

signals:
    void readyGame();  // 準備完了
    void bestMove(const QByteArray &best);  // 指し手
    void pondering(const PonderInfo &info);
    void resign();  // 投了
    void win();  // 入玉勝ち宣言
    void errorOccurred();
    void serializingModel();  // モデル変換中（処理に時間がかかる）

protected slots:
    void getResponse();
    void usiNewGame();
    void engineError();

private:
    bool openContext(const QString &path);  // start process/thread
    void closeContext();
    bool go(const QByteArrayList &position, bool ponderFlag, int senteTime, int goteTime, int byoyomi, int incTime);
    void setTurn();
    void sendOptions(const QVariantMap &options);

    Engine(QObject *parent = nullptr);
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;

    QString _enginePath;
    QString _name;
    QString _author;
    QVariantMap _options;  // カスタムオプション
    QMap<QString, Option> _usiDefaultOptions;  // デフォルトオプション
#ifdef Q_OS_WASM
    static EngineInfo wasmEngineInfo;  // WASMではデフォルトオプションをstaticに保持
#endif
    int _level {20};
    State _state {NotRunning};
    QTimer *_timer {nullptr};  // ポーリングタイマー
    QTimer *_errorTimer {nullptr};  // エラータイマー
    QByteArray _startPositionSfen;
    QByteArrayList _allMoves;
    maru::Turn _turn {maru::Sente};
    QByteArrayList _ponderingMoves;
    QByteArray _lastPondered;
    QString _error;
    EngineContext *_engineContext {nullptr};
};

Q_DECLARE_METATYPE(Engine::Option)
