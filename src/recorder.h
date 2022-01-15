#pragma once
#include "global.h"
#include "piece.h"
#include "ponderinfo.h"
#include <QVector>
#include <QByteArray>
#include <QPair>
#include <QMap>

class Piece;

// 棋譜項目
class FoulCheck {
public:
    FoulCheck() = default;
    FoulCheck(const FoulCheck &) = default;
    FoulCheck(bool c, const QByteArray &s) :
        check(c),
        sfen(s)
    {}

    bool check {false};  // 王手か
    QByteArray sfen;  // 指し手後の局面
};


class ScoreItem {
public:
    ScoreItem() = default;
    ScoreItem(const ScoreItem &) = default;
    ScoreItem(int sc, int mt = 0, int mc = 0, int dep = 0, qint64 n = 0, const QByteArrayList &p = QByteArrayList()) :
        score(sc),
        mate(mt),
        mateCount(mc),
        depth(dep),
        nodes(n),
        pv(p)
    {}

    bool isEmpty() const
    {
        return !score && !mate && !mateCount && !depth && !nodes && pv.isEmpty();
    }

    int score {0};  // +:先手有利  -:後手有利
    int mate {0};   // 0:詰みなし  1:先手詰みあり  -1:後手詰みあり
    int mateCount {0};  // 詰み手数（0:手数不明）
    int depth {0};  // 深さ
    qint64 nodes {0};   // 探索局面数
    QByteArrayList pv;  // 読み筋
};


class Recorder {
public:
    Recorder();

    bool setFirstPosition(const QByteArray &sfen);
    QString record(const QPair<Piece*, QString> &move, bool check);
    QString record(maru::Turn turn, const QPair<Piece*, QString> &move, bool check, const QByteArray &sfenstr, const PonderInfo &info = PonderInfo());
    void recordPonderingScore(int index, int multipv, const ScoreItem &item);
    QVector<ScoreItem> scores(int index) const { return _pvList.value(index).second; }
    bool isRepetition() const;  // 最後の手が千日手か
    bool isPerpetualCheck() const;  // 連続王手の千日手か
    QByteArray move(int index) const;  // 指し手
    QByteArrayList moves(int index) const;
    QByteArray sfen(int index) const;
    QByteArray sfenMoves(int index) const;
    maru::Turn turn(int index) const;
    void removeLast();
    int count() const { return _pvList.count(); }  // 局面数 (初期値:1, 初手指すと2)
    void clear();
    QString kifString(maru::Turn turn, const QPair<Piece*, QString> &move, int prevCoord) const;

    // USI
    QByteArrayList allMoves() const; // USI形式
    QByteArray lastMove() const;  // USI形式

private:
    void addFoulItem(bool check, const QByteArray &sfen);

    QList<FoulCheck> _foulItems;

    // 棋譜検討用
    QList<QPair<QByteArray, QVector<ScoreItem>>> _pvList;  // <USI, PV[]>のリスト
    QList<int> _moveRoutes;
};
