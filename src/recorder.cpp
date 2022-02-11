#include "recorder.h"
#include "piece.h"
#include "ponderinfo.h"
#include "sfen.h"
#include "shogirecord.h"
#include <QDebug>

namespace {

constexpr int MULTIPV_MAX = 5;

char usiChar(Piece::Name name)
{
    switch ((char)name) {
    case 'O':
        return 'K';
    case 'o':
        return 'k';
    default:
        return (char)name;
    }
}

}  // namespace


Recorder::Recorder()
{
    clear();
}

// 棋譜記録
// piece:駒  usi:移動  check:王手か  info:読み筋/指し手候補
QString Recorder::record(const QByteArray &piece, const QByteArray &usi, bool check, const PonderInfo &info)
{
    // リストへ追加
    int prevCoord = (_pvList.count() > 1) ? ShogiRecord::usiToCoord(_pvList.last().first.mid(2, 2)) : 0;  // 直前の指し手マス
    int idx = _pvList.count() - 1;
    auto currentTurn = turn(idx);

    ScoreItem score(info.scoreCp, info.mate, info.mateCount, info.depth, info.nodes, info.pv);
    _pvList << qMakePair(usi, QVector<ScoreItem>({score}));
    _moveRoutes << 0;  // 指し手
    addIllegalItem(check, sfen(idx + 1));
    return ShogiRecord::kifString(currentTurn, usi, piece, prevCoord, false);
}


QString Recorder::record(const QPair<Piece *, QString> &move, bool check, const PonderInfo &info)
{
    auto *piece = move.first;

    if (!piece) {
        return QString();
    }

    int coord = piece->data(maru::Coord).toInt();
    int fromcrd = move.second.toInt();
    bool promoted = move.second.contains('+');

    // USI
    QByteArray usi;
    if (fromcrd < 11 || fromcrd > 99) {
        // 打つ
        usi += QByteArray(1, usiChar(piece->name())).toUpper();  // 大文字
        usi += '*';
    } else {
        // 駒移動
        usi += '0' + fromcrd / 10;
        usi += 0x60 + (fromcrd % 10);
    }

    usi += QByteArray::number(coord / 10);
    usi += 0x60 + (coord % 10);

    if (promoted) {
        usi += '+';
    }

    return record(piece->sfen(), usi, check, info);
}


// 棋譜検討用の評価記録
//  pvList[0]: 対局中の先読み手順
//  pvList[1]: 解析での最有力手順
//  pvList[2]: 解析での第二有力手順
//  pvList[3]: (以下同様)
void Recorder::recordPonderingScore(int index, int multipv, const ScoreItem &item)
{
    if (multipv > MULTIPV_MAX || index < 0) {
        qCritical() << "Invalid value!"
                    << "multipv:" << multipv << "index:" << index;
        return;
    }

    index = std::min(index, (int)_pvList.count() - 1);
    auto &scrlist = _pvList[index].second;
    if (scrlist.count() < multipv + 1) {
        scrlist.resize(multipv + 1);
    }

    // より多くの局面で評価していた場合に更新
    if (item.nodes >= scrlist[multipv].nodes) {
        scrlist[multipv] = item;
    }
}


QString Recorder::kifString(maru::Turn turn, const QPair<Piece *, QString> &move, int prevCoord) const
{
    QByteArray usi;
    QByteArray piece = move.first->sfen();
    int fromcrd = move.second.toInt();
    int tocoord = move.first->data(maru::Coord).toInt();
    if (fromcrd < 11 || fromcrd > 99) {
        // 打つ
        usi += piece.toUpper();
        usi += '*';
        usi += ShogiRecord::coordToUsi(tocoord);
    } else {
        usi += ShogiRecord::coordToUsi(fromcrd);
        usi += ShogiRecord::coordToUsi(tocoord);
        if (move.second.contains('+')) {
            usi += '+';
        }
    }
    return ShogiRecord::kifString(turn, usi, piece, prevCoord, false);
}

// 反則チェックアイテム追加
// check:王手か, sfen:盤面
void Recorder::addIllegalItem(bool check, const QByteArray &sfen)
{
    // 手数以降の文字を削除
    QByteArray sf = sfen.split(' ').mid(0, 3).join(" ");
    _illegalItems << IllegalCheck(check, sf);
}

// 千日手か
bool Recorder::isRepetition() const
{
    if (_illegalItems.count() < 13) {
        return false;
    }

    int count = 0;
    const QString &sfen = _illegalItems.last().sfen;

    for (int i = _illegalItems.count() - 1; i >= 0; i -= 2) {
        if (_illegalItems[i].sfen == sfen) {
            if (++count == 4) {
                return true;
            }
        }
    }
    return false;
}


// 連続王手千日手（禁じ手）か
bool Recorder::isPerpetualCheck() const
{
    if (_illegalItems.count() < 13 || !_illegalItems.last().check) {
        return false;
    }

    int count = 0;
    const QString &sfen = _illegalItems.last().sfen;

    for (int i = _illegalItems.count() - 1; i >= 0; i -= 2) {
        if (_illegalItems[i].sfen == sfen) {
            if (++count == 4) {
                return true;
            }
        }

        if (!_illegalItems[i].check) {
            break;
        }
    }
    return false;
}

// 禁じ手（時間切れは含めない）
bool Recorder::isIllegalMove(int index) const
{
    const auto IllegalMoves = QList {
        maru::Illegal_TwoPawns, maru::Illegal_DropPawnMate,
        maru::Illegal_OverlookedCheck, maru::Illegal_PerpetualCheck, maru::Illegal_Other};

    return index == _pvList.count() - 1 && IllegalMoves.contains(_result.second);
}

// 指し手 index:インデックス(0オリジン)
QByteArray Recorder::move(int index) const
{
    return (index > 0) ? _pvList.value(index).first : QByteArray();
}

// 指し手 index:インデックス(0オリジン)
QByteArrayList Recorder::moves(int index) const
{
    QByteArrayList list;

    int num = std::min(index, (int)_pvList.count() - 1);
    for (int i = 1; i <= num; i++) {
        list << _pvList.value(i).first;
    }
    return list;
}

// SFEN(盤面のみ)  index:インデックス(0オリジン)
QByteArray Recorder::sfen(int index) const
{
    Sfen sfen(_pvList[0].first);
    int num = std::min(index, (int)_pvList.count() - 1);
    for (int i = 1; i <= num; i++) {
        sfen.move(_pvList.value(i).first);
    }
    return sfen.toSfen();
}

// SFEN(指し手あり)  index:インデックス(0オリジン)
QByteArray Recorder::sfenMoves(int index) const
{
    QByteArray sfen = _pvList[0].first;
    if (_pvList.count() > 1) {
        sfen += " moves ";
        sfen += moves(index).join(' ');
    }
    return sfen;
}


Sfen Recorder::toSfen() const
{
    Sfen sfen(sfenMoves(99999));
    sfen.setGameResult(_result.first, _result.second);
    return sfen;
}


maru::Turn Recorder::turn(int index) const
{
    // index:0 = Sente
    return (index % 2) ? maru::Gote : maru::Sente;
}


void Recorder::removeLast()
{
    _illegalItems.removeLast();
}


void Recorder::clear()
{
    _illegalItems.clear();
    _pvList.clear();
    _pvList << qMakePair(Sfen::defaultPostion(), QVector<ScoreItem>({ScoreItem()}));
    _moveRoutes.clear();
}


bool Recorder::setFirstPosition(const QByteArray &sfen)
{
    QByteArray sf = sfen.trimmed();
    bool ok;
    Sfen::fromSfen(sf, &ok);
    if (!ok) {
        return false;
    }

    _pvList[0].first = sf;
    return true;
}


QByteArrayList Recorder::allMoves() const
{
    QByteArrayList pos;
    for (int i = 1; i < _pvList.count(); i++) {
        pos << _pvList[i].first;
    }
    return pos;
}


QByteArray Recorder::lastMove() const
{
    return (_pvList.count() > 1) ? _pvList.last().first : QByteArray();
}


void Recorder::setGameResult(maru::GameResult result, maru::ResultDetail detail)
{
    _result.first = result;
    _result.second = detail;
}
