#pragma once
#include "global.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QPair>
#include <QGraphicsPixmapItem>

class Badge;


// 駒クラス
class Piece : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    enum Name : int {
        None = 0,        // マス
        KingOu = 'o',    // 王
        KingGyoku = 'k', // 玉
        Rook = 'r',      // 飛
        Bishop = 'b',    // 角
        Gold = 'g',      // 金
        Silver = 's',    // 銀
        Knight = 'n',    // 桂
        Lance = 'l',     // 香
        Pawn = 'p',      // 歩
        PromotedRook   = ('+' << 8) | Rook,   // 竜
        PromotedBishop = ('+' << 8) | Bishop, // 馬
        PromotedSilver = ('+' << 8) | Silver, // 成銀
        PromotedKnight = ('+' << 8) | Knight, // 成桂
        PromotedLance  = ('+' << 8) | Lance,  // 成香
        PromotedPawn   = ('+' << 8) | Pawn,   // と金
        NameMask = 0xFFF,  // マスク
    };

    // 駒の向き
    enum Orientation : int {
        Upward     = 0x10000,
        Downward   = 0x20000,
        Rotated    = 0x40000,  // 180度回転
        Upward_R   = Upward | Rotated,
        Downward_R = Downward | Rotated,
    };

    Piece(QGraphicsItem *parent = nullptr);  // マス
    Piece(Name name, maru::Turn owner, QGraphicsItem *parent = nullptr);
    virtual ~Piece();
    //QRectF boundingRect() const override;
    Name name() const { return _name; }
    QString kanjiName() const;
    QByteArray sfen() const;
    maru::Turn owner() const { return _owner; }
    void setOwner(maru::Turn owner) { _owner = owner; }
    bool isPromoted() const;
    Name promotedName() const;
    Name originalName() const;
    void setBadge(int number);
    void advance(int phase) override;

    static QString kanjiName(Name name);
    static void setRotation(bool rotate);
    static QPair<Name, maru::Turn> name(const QByteArray &p);
    static QPair<Name, maru::Turn> name(char p, bool promoted = false);
    static Name promotedName(Name name);

signals:
    void clicked();

protected:
    void reload();
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    //void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Name _name {None};
    maru::Turn _owner {maru::Sente};
    Orientation _orient {Upward};
    Badge *_badge {nullptr};
};
