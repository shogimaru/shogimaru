#include "engine.h"
#include "command.h"
#include "enginesettings.h"
#include "enginethread.h"
#include "ponderinfo.h"
#include <QDebug>
#include <cmath>
#include <thread>


Engine::~Engine()
{
    close();
}


Engine &Engine::instance()
{
    static auto engine = new Engine;
    return *engine;
}


bool Engine::open(const QString &path)
{
    if (_state != NotRunning) {
        qDebug() << "Engine running";
        return true;
    }

    // Starts engine
    openUsi(path);
    Command::instance().request("usi");

    auto parseUsi = [this]() {
        // 変換マップ
        static const QMap<QString, QMetaType::Type> conv {
            {"check", QMetaType::Bool},
            {"spin", QMetaType::LongLong},
            {"string", QMetaType::QString},
            {"combo", QMetaType::QStringList},
        };

        // 次のワード
        auto nextWord = [](const QStringList &items, const QString &word) {
            int idx = items.indexOf(word);
            return (idx >= 0) ? items.value(idx + 1) : QString();
        };

        while (true) {
            auto responses = Command::instance().poll(1000);
            if (responses.empty()) {
                break;
            }

            for (auto &response : responses) {
                auto str = QString::fromStdString(response).trimmed();
                if (str == "usiok") {
                    return true;
                }

                if (str.startsWith("id name ")) {
                    _name = str.mid(8).trimmed();
                    continue;
                }

                if (str.startsWith("id author ")) {
                    _author = str.mid(10).trimmed();
                    continue;
                }

                if (str.startsWith("option name ")) {
                    auto items = str.mid(12).split(" ", Qt::SkipEmptyParts);
                    if (items.count() >= 4) {
                        Option option;
                        auto type = nextWord(items, "type");
                        option.type = conv.value(type, QMetaType::Void);
                        if (option.type == QMetaType::Void) {
                            qWarning() << "Invalid type:" << type;
                            continue;
                        }

                        switch (option.type) {
                        case QMetaType::Bool:
                            option.value.setValue(nextWord(items, "default") == "true");
                            break;
                        case QMetaType::LongLong:
                            option.value.setValue(nextWord(items, "default").toLongLong());
                            break;
                        case QMetaType::QString:
                            option.value.setValue(nextWord(items, "default"));
                            break;
                        case QMetaType::QStringList: {
                            int idx = items.indexOf("default");
                            auto strs = (idx >= 0) ? items.mid(idx + 1) : QStringList();
                            strs.removeAll("var");
                            option.value.setValue(strs);
                            break;
                        }

                        default:
                            continue;
                        }
                        option.max = nextWord(items, "max").toLongLong();
                        option.min = nextWord(items, "min").toLongLong();
                        _defaultOptions.insert(items[0], option);
                        //qDebug() << "insert:" << option.value;
                    }
                }
            }
        }
        return false;
    };

    if (!parseUsi()) {
        qCritical() << "Error response";
        return false;
    }

    // #if 0
    //     int con = std::thread::hardware_concurrency();  // コア（スレッド）数
    //     int threads = std::max((int)std::round(con * 0.8), 2);  // 80%
    //     std::string cmd = std::string("setoption name Threads value ") + std::to_string(threads);
    //     Command::instance().request(cmd);
    //     Command::instance().request("setoption name NetworkDelay value 50");  // ネットワーク遅延
    //     Command::instance().request("setoption name NetworkDelay2 value 500");  // 切れ負けになる場合のネットワーク遅延
    // #else
    //     auto engine = EngineSettings::instance().currentEngine();
    //     QVariantMap options = EngineSettings::instance().generalOptions();
    //     options.insert(engine.options);
    //     for (auto it = options.begin(); it != options.end(); ++it) {
    //         if (!it.key().isEmpty() && !it.value().isNull()) {
    //             QString command = QString("setoption name %1 value %2").arg(it.key(), it.value().toString());
    //             qDebug() << command;
    //             Command::instance().request(command.toStdString());
    //         }
    //     }
    // #endif

    //     //Command::instance().request("setoption name EvalDir value assets/YaneuraOu/nnue-kp256/");
    //     //Command::instance().request("setoption name EvalDir value assets/YaneuraOu/nnue/");
    //     //Command::instance().request("setoption name BookDir value assets/YaneuraOu/");

    //     for (;;) {
    //         auto res = Command::instance().poll(200);
    //         if (res.empty()) {
    //             break;
    //         }

    //         for (auto &msg : res) {
    //             if (msg.find("Error") == 0) {
    //                 qCritical() << "Engine" << msg.c_str();
    //             } else {
    //                 qDebug() << "Engine response:" << msg.c_str();
    //             }
    //         }
    //     }

    _state = GameReady;
    return true;
}


void Engine::close()
{
    closeUsi();
    _state = NotRunning;
}


void Engine::setStartPosition(const QByteArray &sfen)
{
    _startPositionSfen = sfen;

    if (_startPositionSfen.isEmpty()) {
        _turn = maru::Sente;
        return;
    }

    QByteArrayList items = _startPositionSfen.split(' ');
    _turn = (items.value(1) == "w") ? maru::Gote : maru::Sente;
}


void Engine::sendOptions(const QVariantMap &options)
{
    QString value;
    for (auto it = options.begin(); it != options.end(); ++it) {
        auto opt = _defaultOptions.value(it.key());
#if QT_VERSION < 0x060000
        int type = it.value().type();
#else
        auto type = it.value().typeId();
#endif
        switch (type) {
        case QMetaType::Bool:
            value = (it.value().toBool()) ? QLatin1String("true") : QLatin1String("false");
            break;
        case QMetaType::QStringList:
            value = it.value().toStringList().join(" ");
            break;
        default:
            value = it.value().toString();
            break;
        }

        if (opt.value.toString() != value) {  // デフォルトと違う場合に実行
            auto bytes = QString("setoption name %1 value %2").arg(it.key()).arg(value);
            Command::instance().request(bytes.toStdString());
        }
    }

    for (;;) {
        auto res = Command::instance().poll(200);
        if (res.empty()) {
            break;
        }

        for (auto &msg : res) {
            if (msg.find("Error") == 0) {
                qCritical() << "Engine" << msg.c_str();
            } else {
                qDebug() << "Engine response:" << msg.c_str();
            }
        }
    }
}


bool Engine::startAnalysis()
{
    if (_state != GameReady) {
        qCritical() << "startAnalysis() Invalid state:" << _state;
        return false;
    }

    Command::instance().clearResponse();
#if 0
    Command::instance().request("setoption name MultiPV value 5");
    Command::instance().request("setoption name BookDepthLimit value 0");  // やねうら王用の定跡専用オプション
    Command::instance().request("setoption name BookMoves value 0");  // 定跡を用いる手数
    Command::instance().request("setoption name BookEvalDiff value 0");  // 最善手のみを採用するなら0に。ソフトが指す定跡に幅を持たせたいなら、10〜50ぐらいの大きめの値に。
    Command::instance().request("setoption name SlowMover value 100");  // 序盤重視率[%]
    Command::instance().request("setoption name SkillLevel value 20");  // MAX値
#else
    QVariantMap opts = _options;
    auto def = _defaultOptions.value("MultiPV");
    if (!def.value.isNull()) {
        opts.insert("MultiPV", 5);
    }
    def = _defaultOptions.value("SlowMover");
    if (!def.value.isNull()) {
        opts.insert("SlowMover", def.value);
    }
    def = _defaultOptions.value("SkillLevel");
    if (!def.value.isNull()) {
        opts.insert("SkillLevel", def.value);
    }
    sendOptions(opts);
#endif
    Command::instance().request("isready");
    std::list<std::string> response;
    if (!Command::instance().pollFor("readyok", 1000, response)) {  // readyok
        _error = QString::fromStdString(maru::join(response, "\n"));
        _state = EngineError;
        return false;
    }
    Command::instance().request("usinewgame");
    _state = Idle;
    return true;
}


bool Engine::analysis(const QByteArray &sfen)
{
    if (_state != Idle) {
        qCritical() << "analysis() Invalid state:" << _state;
        return false;
    }

    // 検討開始
    setStartPosition(sfen);
    QByteArray cmd = "position sfen ";
    cmd.reserve(256);
    cmd += sfen;
    Command::instance().request(cmd.toStdString());
    Command::instance().request("go infinite");
    _state = Pondering;

    _timer->start(100);
    return true;
}


bool Engine::newGame(int slowMover)
{
    if (_state != GameReady) {
        qCritical() << "newGame() Invalid state:" << _state;
        return false;
    }

    Command::instance().clearResponse();
#if 0
    Command::instance().request("setoption name MultiPV value 1");
    Command::instance().request("setoption name BookFile value user_book1.db");  // 定跡ファイル:100テラショック定跡
    Command::instance().request("setoption name BookDepthLimit value 0");  // やねうら王用の定跡専用オプション
    Command::instance().request("setoption name BookMoves value 256");
    Command::instance().request("setoption name BookEvalDiff value 30");  // 最善手のみを採用するなら0に。ソフトが指す定跡に幅を持たせたいなら、10〜50ぐらいの大きめの値に。
    //Command::instance().request("setoption name BookFile value no_book");  // 定跡を使わない
    Command::instance().request("setoption name SlowMover value " + std::to_string(slowMover));  // 序盤重視率[%]
    //qDebug() << "序盤重視率(SlowMover):" << slowMover;
    std::string cmd = std::string("setoption name SkillLevel value ") + std::to_string(_level);
    Command::instance().request(cmd);
#else
    QVariantMap opts = _options;
    auto def = _defaultOptions.value("MultiPV");
    if (!def.value.isNull()) {
        opts.insert("MultiPV", 1);
    }
    def = _defaultOptions.value("SlowMover");
    if (!def.value.isNull()) {
        opts.insert("SlowMover", slowMover);
    }
    def = _defaultOptions.value("SkillLevel");
    if (!def.value.isNull()) {
        opts.insert("SkillLevel", _level);
    }
    sendOptions(opts);
#endif
    Command::instance().request("isready");
    std::list<std::string> response;
    if (!Command::instance().pollFor("readyok", 1000, response)) {  // readyok
        _error = QString::fromStdString(maru::join(response, "\n"));
        _state = EngineError;
        return false;
    }
    Command::instance().request("usinewgame");
    _state = Idle;
    _timer->start(100);
    return true;
}

// 手番再設定
void Engine::setTurn()
{
    setStartPosition(startPosition());
    if (_allMoves.count() % 2) {
        _turn = (_turn == maru::Sente) ? maru::Gote : maru::Sente;
    }
}

// 思考開始
bool Engine::go(const QByteArrayList &moves, int senteTime, int goteTime, int byoyomi)
{
    _allMoves = moves;
    setTurn();
    return go(moves, false, senteTime, goteTime, byoyomi);
}

// 先読み開始
bool Engine::ponder(int senteTime, int goteTime, int byoyomi)
{
    if (!_lastPondered.isEmpty()) {
        auto pos = _allMoves;
        pos << _lastPondered;
        //qDebug() << "先読み: " << qPrintable(pos.join(" "));
        return go(pos, true, senteTime, goteTime, byoyomi);
    }
    return false;
}


bool Engine::go(const QByteArrayList &moves, bool ponder, int senteTime, int goteTime, int byoyomi)
{
    if (_state == Pondering) {
        if (ponder) {
            qDebug() << "Error status";
            return false;
        }

        if (moves == _ponderingMoves) {
            //qDebug() << "ponderhit";
            Command::instance().request("ponderhit");
            return true;
        } else {
            stop();
        }
    }

    if (_state != Idle) {
        qDebug() << "Error status";
        return false;
    }

    QByteArray cmd = "position ";
    cmd.reserve(256);
    cmd += (_startPositionSfen.isEmpty()) ? QByteArray("startpos") : QByteArray("sfen ") + _startPositionSfen;
    if (!moves.isEmpty()) {
        cmd += " moves ";
        cmd += moves.join(' ');
    }
    Command::instance().request(cmd.toStdString());
    //qDebug() << "cmd: " << cmd;

    if (ponder) {
        _state = Pondering;
        _ponderingMoves = moves;
        cmd = "go ponder";
    } else {
        _state = Going;
        _ponderingMoves.clear();
        _lastPondered.clear();
        cmd = "go";
    }

    if (senteTime > 0) {
        cmd += " btime ";
        cmd += QByteArray::number(senteTime);
    }
    if (goteTime > 0) {
        cmd += " wtime ";
        cmd += QByteArray::number(goteTime);
    }
    if (byoyomi > 0) {
        cmd += " byoyomi ";
        cmd += QByteArray::number(byoyomi);
    }

    Command::instance().request(cmd.toStdString());
    return true;
}


void Engine::stop()
{
    bool res;
    std::list<std::string> response;

    switch (_state) {
    case Going:
    case Pondering:
        Command::instance().request("stop");
        res = Command::instance().pollFor("bestmove", 1000, response);
        //qDebug() << "go stop" << res;
        _state = Idle;
        break;

    case GameReady:
    default:
        // do nothing
        qCritical() << "stop(): Invalid state:" << _state;
        break;
    }
}


bool Engine::mated(const QByteArrayList &moves)
{
    QByteArray startpos = (_startPositionSfen.isEmpty()) ? QByteArray("startpos") : _startPositionSfen;
    return mated(startpos, moves);
}


bool Engine::mated(const QByteArray &startPosition, const QByteArrayList &moves)
{
    // エンジンは打ち歩詰めを反則と認識するので局面を初期配置として送って判断させること

    switch (_state) {
    case Idle:
    case Going:
    case Pondering: {
        QString cmd = QString("position sfen ") + startPosition;
        if (!moves.isEmpty()) {
            cmd += " moves ";
            cmd += moves.join(" ");
        }
        //qDebug() << cmd.toStdString();
        Command::instance().request(cmd.toStdString());
        Command::instance().request("mated");
        auto res = Command::instance().poll(1000);
        if (res.empty()) {
            return false;
        }
        return (*res.begin() == "1");
    }
    case GameReady:
    default:
        // do nothing
        break;
    }
    return false;
}


void Engine::quit()
{
    switch (_state) {
    case GameReady:
    case Idle:
    case Going:
    case Pondering:
        Command::instance().request("quit");
        break;

    default:
        // do nothing
        break;
    }
    _timer->stop();
    _state = NotRunning;
}


void Engine::gameover()
{
    switch (_state) {
    case Idle:
    case Going:
    case Pondering:
        Command::instance().request("gameover win");  // win固定
        break;

    case GameReady:
    default:
        // do nothing
        break;
    }
    _timer->stop();
    _state = GameReady;
}


void Engine::getResponse()
{
    auto response = Command::instance().poll(0);

    if (response.empty()) {
        return;
    }

    for (auto &s : response) {
        if (s.find("bestmove ") == 0) {
            _state = Idle;
            auto strs = maru::split(s, ' ', true);

            if (strs.size() < 2) {
                _state = EngineError;
                return;
            }

            _lastPondered.clear();
            QByteArray best = QByteArray::fromStdString(strs[1]);
            if (best == "win") {
                // 入玉勝ち宣言
                _state = GameReady;
                emit win();
            } else if (best == "resign") {
                // 投了
                _state = GameReady;
                emit resign();
            } else {
                if (strs.size() == 4 && strs[2] == "ponder") {
                    _lastPondered = QByteArray::fromStdString(strs[3]);
                }
                _state = Idle;
                _allMoves << best;
                emit bestMove(best);
                setTurn();
            }
        } else if (s.find("info ") == 0) {
            // 先読み候補
            PonderInfo info(QByteArray::fromStdString(s));
            //qDebug() << "multipv" << info.multipv << "pondering score:" << info.scoreCp << "mate:" << info.mate << "mateCount:" << info.mateCount << "depth:" << info.depth << "nodes:" << info.nodes << "pv:" << info.pv;
            emit pondering(info);
        } else {
            qDebug() << "Unknown response" << s.c_str();
        }
    }
}


Engine::EngineInfo Engine::getEngineInfo(const QString &path)
{
    Engine::EngineInfo info;
    auto engine = new Engine;
    if (engine->open(path)) {
        info.name = engine->name();
        info.path = path;
        info.author = engine->author();
        info.options = engine->_defaultOptions;
    }
    engine->close();
    delete engine;
    return info;
}
