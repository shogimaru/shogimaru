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
    if (!openContext(path)) {
        return false;
    }
    Command::instance().request("usi");

    auto parseUsi = [this]() {
        // 変換マップ
        static const QMap<QString, QMetaType::Type> conv {
            {"check", QMetaType::Bool},
            {"spin", QMetaType::LongLong},
            {"string", QMetaType::QString},
            {"combo", QMetaType::QStringList},
            {"filename", QMetaType::QUrl},
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
                //qDebug() << str;
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
                            option.defaultValue.setValue(nextWord(items, "default") == "true");  // true or false
                            break;
                        case QMetaType::LongLong:
                            option.defaultValue.setValue(nextWord(items, "default").toLongLong());
                            break;
                        case QMetaType::QUrl:
                        case QMetaType::QString:
                            option.defaultValue.setValue(nextWord(items, "default"));
                            break;
                        case QMetaType::QStringList: {
                            int idx = items.indexOf("default");
                            auto strs = (idx >= 0) ? items.mid(idx + 1) : QStringList();
                            strs.removeAll("var");
                            option.defaultValue.setValue(strs);
                            break;
                        }
                        default:
                            continue;
                        }
                        option.value = option.defaultValue;  // 最初は現在値と初期値は同じ
                        option.max = nextWord(items, "max").toLongLong();
                        option.min = nextWord(items, "min").toLongLong();
                        _usiDefaultOptions.insert(items[0], option);
                        //qInfo() << "insert:" << option.defaultValue;
                    }
                }
            }
        }
        return false;
    };

    if (_usiDefaultOptions.isEmpty()) {
        if (parseUsi()) {
#ifdef Q_OS_WASM
            if (wasmEngineInfo.options.isEmpty()) {
                wasmEngineInfo.name = _name;
                wasmEngineInfo.author = _author;
                wasmEngineInfo.options = _usiDefaultOptions;
            }
#endif
        } else {
            qCritical() << "Error response";
            emit errorOccurred();
            return false;
        }
    } else {
        std::list<std::string> response;
        if (!Command::instance().pollFor("usiok", 1000, response)) {
            qCritical() << "USI Error";
            emit errorOccurred();
            return false;
        }
    }

    _state = GameReady;
    return true;
}


void Engine::close()
{
    closeContext();
#ifndef Q_OS_WASM
    _usiDefaultOptions.clear();
    _options.clear();
#endif
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
        auto &opt = _usiDefaultOptions[it.key()];
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
        default:
            value = it.value().toString();
            break;
        }
        value = value.trimmed();

        if (!it.key().isEmpty() && !value.isEmpty()) {
            if (it.value() != opt.value) {
                // 現在値と違うオプションを送る
                auto bytes = QString("setoption name %1 value %2").arg(it.key()).arg(value);
                //qDebug() << "sendOptions:" << bytes;
                Command::instance().request(bytes.toStdString());
                opt.value.setValue(it.value());  // 現在値にセット
            }
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


bool Engine::newGame(int slowMover)
{
    if (_state != GameReady) {
        qCritical() << "newGame() Invalid state:" << _state;
        return false;
    }

    Command::instance().clearResponse();

    QVariantMap opts = _options;
    if (opts.contains("MultiPV")) {  // 対局では1
        opts.insert("MultiPV", 1);
    }
    if (opts.contains("SlowMover")) {  // 序盤重視率[%]
        opts.insert("SlowMover", slowMover);
    }
    if (opts.contains("SkillLevel")) {
        opts.insert("SkillLevel", _level);
    }
    sendOptions(opts);

    Command::instance().request("isready");
    _timer->start(66);  // 受信開始
    _errorTimer->start(20000);  // エラータイマー開始（初回のisreadyは結構時間がかかる）
    return true;
}


void Engine::usiNewGame()
{
    if (_state != GameReady) {
        qCritical() << "usiNewGame() Invalid state:" << _state;
        return;
    }

    _errorTimer->stop();
    Command::instance().request("usinewgame");
    _state = Idle;
    emit ready();
}


bool Engine::startAnalysis()
{
    if (_state != GameReady) {
        qCritical() << "startAnalysis() Invalid state:" << _state;
        return false;
    }

    Command::instance().clearResponse();
    //qDebug() << _options;
    sendOptions(_options);

    Command::instance().request("isready");
    _timer->start(66);  // 受信開始
    _errorTimer->start(20000);  // エラータイマー開始（初回のisreadyは結構時間がかかる）
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
    return true;
}


void Engine::engineError()
{
    qWarning() << "Engine Error!";
    _error = "Engine Error";
    emit errorOccurred();
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
bool Engine::go(const QByteArrayList &moves, int senteTime, int goteTime, int byoyomi, int incTime)
{
    _allMoves = moves;
    setTurn();
    return go(moves, false, senteTime, goteTime, byoyomi, incTime);
}

// 先読み開始
bool Engine::ponder(int senteTime, int goteTime, int byoyomi, int incTime)
{
    if (!_lastPondered.isEmpty()) {
        auto pos = _allMoves;
        pos << _lastPondered;
        //qDebug() << "先読み: " << qPrintable(pos.join(" "));
        return go(pos, true, senteTime, goteTime, byoyomi, incTime);
    }
    return false;
}


bool Engine::go(const QByteArrayList &moves, bool ponder, int senteTime, int goteTime, int byoyomi, int incTime)
{
    if (_state == Pondering) {
        if (ponder) {
            qDebug() << "go() Error status" << _state << "line:" << __LINE__;
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
        qDebug() << "go() Error status:" << _state << "line:" << __LINE__;
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
    } else if (incTime > 0) {
        cmd += " binc ";
        cmd += QByteArray::number(incTime);
        cmd += " winc ";
        cmd += QByteArray::number(incTime);
    }

    qDebug() << cmd;
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
        if (!res) {
            qWarning() << "go stop" << res;
        }
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
        if (s.find("readyok") == 0) {
            // 準備完了
            usiNewGame();
        } else if (s.find("bestmove ") == 0) {
            if (_state != Pondering && _state != Going) {
                continue;
            }

            // 最善手
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
            // 通知
            if (_state == Pondering || _state == Going) {
                // 先読み候補
                PonderInfo info(QByteArray::fromStdString(s));
                //qDebug() << "multipv" << info.multipv << "pondering score:" << info.scoreCp << "mate:" << info.mate << "mateCount:" << info.mateCount << "depth:" << info.depth << "nodes:" << info.nodes << "pv:" << info.pv;
                emit pondering(info);
            }
        } else {
            if (maru::toLower(s).find("error") != std::string::npos) {  // errorがある場合
                _error = QString::fromStdString(s);
                emit errorOccurred();
                _errorTimer->stop();
            } else {
                qDebug() << "Unknown response" << s.c_str();
            }
        }
    }
}


bool Engine::hasSkillLevelOption() const
{
    return !_usiDefaultOptions.value("SkillLevel").defaultValue.isNull();
}

// Type of the option
QMetaType::Type Engine::type(const QString &option) const
{
    auto optionData = _usiDefaultOptions.value(option);
    return optionData.type;
}
