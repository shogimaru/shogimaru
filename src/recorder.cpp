#include "recorder.h"
#include "shogirecord.h"
#include "piece.h"
#include "sfen.h"
#include "ponderinfo.h"
#include <QDebug>

namespace {

constexpr int MULTIPV_MAX = 5;
const QByteArray firstPosition = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";

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

} // namespace


Recorder::Recorder()
{
    clear();
}

QString Recorder::record(const QPair<Piece*, QString> &move, bool check)
{
    auto *piece = move.first;

    if (!piece) {
        return QString();
    }

    int coord = piece->data(maru::Coord).toInt();
    int fromcrd = move.second.toInt();
    bool promoted = move.second.contains('+');
    int prevCoord = (_pvList.count() > 1) ? ShogiRecord::usiToCoord(_pvList.last().first.mid(2, 2)) : 0; // 直前の指し手マス

    // USI
    QByteArray usi;
    if (fromcrd < 11 || fromcrd > 99) {
        // 打つ
        usi += QByteArray(1, usiChar(piece->name())).toUpper(); // 大文字
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

    // リストへ追加
    int idx = _pvList.count() - 1;
    auto currentTurn = turn(idx);
    _pvList << qMakePair(usi, QVector<ScoreItem>({ScoreItem()}));
    _moveRoutes << 0;  // 指し手
    addFoulItem(check, sfen(idx + 1));
    return kifString(currentTurn, move, prevCoord);
}


QString Recorder::record(maru::Turn turn, const QPair<Piece*, QString> &move, bool check, const QByteArray &sfen, const PonderInfo &info)
{
    auto *piece = move.first;

    if (!piece) {
        return QString();
    }

    int coord = piece->data(maru::Coord).toInt();
    int fromcrd = move.second.toInt();
    bool promoted = move.second.contains('+');
    int prevCoord = (_pvList.count() > 1) ? ShogiRecord::usiToCoord(_pvList.last().first.mid(2, 2)) : 0; // 直前の指し手マス

    // USI
    QByteArray usi;
    if (fromcrd < 11 || fromcrd > 99) {
        // 打つ
        usi += QByteArray(1, usiChar(piece->name())).toUpper(); // 大文字
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

    ScoreItem score(info.scoreCp, info.mate, info.mateCount, info.depth, info.nodes, info.pv);
    _pvList << qMakePair(usi, QVector<ScoreItem>({score}));
    _moveRoutes << 0;  // 指し手

    // リストへ追加
    addFoulItem(check, sfen);

    return kifString(turn, move, prevCoord);
}


// 棋譜検討用の評価記録
//  pvList[0]: 対局中の先読み手順
//  pvList[1]: 解析での最有力手順
//  pvList[2]: 解析での第二有力手順
//  pvList[3]: (以下同様)
void Recorder::recordPonderingScore(int index, int multipv, const ScoreItem &item)
{
    if (multipv > MULTIPV_MAX || index < 0) {
        qCritical() << "Invalid value!" << "multipv:" << multipv << "index:" << index;
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


QString Recorder::kifString(maru::Turn turn, const QPair<Piece*, QString> &move, int prevCoord) const
{
#if 1
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
    qDebug() << "==" << usi << piece;
    return ShogiRecord::kifString(turn, usi, piece, prevCoord, false);
#else
    QString kif;

    auto *piece = move.first;
    int coord = piece->data(maru::Coord).toInt();
    int fromcrd = move.second.toInt();
    bool promoted = move.second.contains('+');
    //int prevCoord = (_items.isEmpty()) ? 0 : _items.last().move.second;

    // 棋譜
    if (turn == maru::Sente) {
        kif += QString::fromUtf8(u8"▲");
    } else {
        kif += QString::fromUtf8(u8"△");
    }

    if (coord == prevCoord && prevCoord > 0) {
        kif += QString::fromUtf8(u8"同  ");
    } else {
        kif += ShogiRecord::kanji(coord);  // マス
    }

    // 駒名称
    if (promoted) {
        kif += Piece::kanjiName(piece->originalName());
        kif += QString::fromUtf8(u8"成");
    } else {
        kif += piece->kanjiName();
    }

    if (fromcrd < 11 || fromcrd > 99) {
        kif += QString::fromUtf8(u8"打");
    }
    return kif;
#endif
}


void Recorder::addFoulItem(bool check, const QByteArray &sfen)
{
    // 手数以降の文字を削除
    QByteArray sf = sfen.split(' ').mid(0, 3).join(" ");
    _foulItems << FoulCheck(check, sf);
}

// 千日手か
bool Recorder::isRepetition() const
{
    if (_foulItems.count() < 13) {
        return false;
    }

    int count = 0;
    const QString &sfen = _foulItems.last().sfen;

    for (int i = _foulItems.count() - 1; i >= 0; i -= 2) {
        if (_foulItems[i].sfen == sfen) {
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
    if (_foulItems.count() < 13 || !_foulItems.last().check) {
        return false;
    }

    int count = 0;
    const QString &sfen = _foulItems.last().sfen;

    for (int i = _foulItems.count() - 1; i >= 0; i -= 2) {
        if (_foulItems[i].sfen == sfen) {
            if (++count == 4) {
                return true;
            }
        }

        if (!_foulItems[i].check) {
            break;
        }
    }
    return false;
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

    for (int i = 1; i <= index; i++) {
        list << _pvList.value(i).first;
    }
    return list;
}

// SFEN(盤面のみ)  index:インデックス(0オリジン)
QByteArray Recorder::sfen(int index) const
{
    Sfen sfen(_pvList[0].first);
    for (int i = 1; i <= index; i++) {
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


maru::Turn Recorder::turn(int index) const
{
    // index:0 = Sente
    return (index % 2) ? maru::Gote: maru::Sente;
}


void Recorder::removeLast()
{
    _foulItems.removeLast();
}


void Recorder::clear()
{
    _foulItems.clear();
    _pvList.clear();
    _pvList << qMakePair(firstPosition, QVector<ScoreItem>({ScoreItem()}));
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
