#pragma once
#include <QStringList>
#include <QByteArray>


class PonderInfo {
public:
    int depth {0};
    int seldepth {0};
    int time {0};
    qint64 nodes {0};
    int multipv {0};
    int scoreCp {0};
    int bound {0};  // 1:upperbound  -1:lowerbound
    int mate {0};  // 1:詰みあり先手勝ち  -1:詰みあり後手勝ち  0:詰みなし(不明)
    int mateCount {0};  // 詰み手数（0なら手数不明）
    QByteArray currmove;
    int hashfull {0};
    qint64 nps {0};
    QByteArrayList pv;
    QByteArray message;

    PonderInfo() = default;
    PonderInfo(const QByteArray &info);
    PonderInfo(const PonderInfo &) = default;
    PonderInfo &operator=(const PonderInfo &) = default;

    bool isEmpty() const { return !scoreCp && !nodes && pv.isEmpty(); }
    void clear();

protected:
    void parse(const QByteArray &info);
};
