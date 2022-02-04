#pragma once
#include "global.h"
#include <QStringList>
#include <QMap>

/*!
  Shogi Forsyth-Edwards Notation class
 */
class Sfen {
public:
    Sfen() {}
    Sfen(const QByteArray &sfen);
    Sfen(const Sfen &) = default;
    Sfen(Sfen &&) = default;
    Sfen &operator=(const Sfen &) = default;
    Sfen &operator=(Sfen &&) = default;

    bool isEmpty() const;
    QByteArray name(int coord) const;
    QString kanjiName(int coord) const;
    QByteArray move(const QByteArray &usi);
    QPair<QString, int> move(const QByteArray &usi, int prevCoord, bool compact = true);
    Sfen move(const QByteArrayList &usiList);
    QList<QPair<QByteArray, QByteArray>> allMoves() const { return _moves; }
    QByteArray toSfen() const;
    QByteArray toUsi() const;
    QStringList generateKif(const QByteArrayList &moves, bool compact = true) const;
    QPair<QString, QString> players() const { return _players; }
    void setPlayers(const QString &sente, const QString &gote);
    void clear();

    static QByteArray defaultPostion();
    static Sfen fromCsa(const QString &csa, bool *ok = nullptr);
    static Sfen fromSfen(const QByteArray &sfen, bool *ok = nullptr);

private:
    bool parse(const QByteArray &sfen);

    QMap<int, QByteArray> _position;  // <マス目, SFEN>
    QByteArray _inHand;
    maru::Turn _turn {maru::Sente};
    QList<QPair<QByteArray, QByteArray>> _moves;  // <Piece, USI>
    int _counter {1};
    QPair<QString, QString> _players;  // <先手,後手>

    friend class Board;
};
