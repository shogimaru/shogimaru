#pragma once
#include "global.h"
#include <QString>

// 棋譜
class ShogiRecord {
public:
    static int usiToCoord(const QByteArray &usi);
    static QByteArray coordToUsi(int coord);
    static QMap<int, QString> usiToPosition(const QMap<int, QString> &startPosition, maru::Turn startTurn, const QStringList &moves);
    static QString kanji(int pos);  // マス位置漢字表示
    static QString kanjiName(const QString &piece);  // 駒名称
    static QString kifString(maru::Turn turn, const QByteArray &usi, int prevCoord, bool compact);
    static QString kifString(maru::Turn turn, const QByteArray &usi, const QByteArray &piece, int prevCoord, bool compact);
};
