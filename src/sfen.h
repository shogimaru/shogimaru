#pragma once
#include "global.h"
#include <QStringList>
#include <QMap>

/*!
  Shogi Forsyth-Edwards Notation class
 */
class Sfen {
public:
    Sfen();  // Initial position
    Sfen(const QByteArray &sfen);
    inline Sfen(const Sfen &) = default;
    Sfen &operator=(Sfen &) = default;

    QByteArray name(int coord) const;
    QString kanjiName(int coord) const;
    QByteArray move(const QByteArray &usi);
    QPair<QString, int> move(const QByteArray &usi, int prevCoord, bool compact = true);
    Sfen move(const QByteArrayList &usiList);
    QByteArray toSfen() const;
    QStringList generateKif(const QByteArrayList &moves, bool compact = true) const;

    static QByteArray csaToSfen(const QByteArray &csa);
    static Sfen fromCsa(const QByteArray &csa, bool *ok = nullptr);
    static Sfen fromSfen(const QByteArray &sfen, bool *ok = nullptr);

private:
    bool parse(const QByteArray &sfen);

    QMap<int, QByteArray> _position;  // <マス目, SFEN>
    QByteArray _inHand;
    maru::Turn _turn {maru::Sente};
    int _counter {1};

    friend class Board;
};
