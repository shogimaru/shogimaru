#pragma once

#include "piece.h"
#include "global.h"
#include "ui_mainwindow.h"
#include <QPixmap>
#include <QGraphicsScene>
#include <QPropertyAnimation>
#include <QPair>
#include <QDebug>

class QAbstractButton;
class Square;

// 将棋盤クラス
class Board : public QGraphicsScene {
    Q_OBJECT
public:
    Board(QObject *parent = nullptr);
    Piece *piece(int coord);
    const Piece *piece(int coord) const;
    QList<Piece*> pieces(maru::Turn owner, Piece::Name name) const;  // 盤上から駒を探索
    Piece *pieceInHand(maru::Turn owner, Piece::Name name) const;  // 持ち駒から探索
    QList<Piece*> piecesInHand(maru::Turn owner) const;  // 持ち駒
    int kingCoord(maru::Turn owner) const;  // 玉の座標
    void move(Piece *from, Piece *to, bool promote = false, bool update = true);  // 指す
    bool canMove(const Piece *piece, int coord) const;
    QPair<Piece*, QString> lastMovedPiece() const { return _lastMove; }
    bool isCheck() const;  // 最終手が王手か
    QList<Piece*> searchMovablePeace(maru::Turn turn, int coord) const;
    // QByteArray toStaticSfen(int moveNumber) const;  // SFEN形式へ変換
    // QByteArray toStaticSfen(maru::Turn turn, int moveNumber) const;  // SFEN形式へ変換
    bool startGame(const QByteArray &sfen);
    void setSfen(const QByteArray &sfen, bool movable, const QByteArray &lastMove = QByteArray());

    static QPixmap board();  // 将棋盤

public slots:
    void stopGame();
    void setTurn(maru::Turn turn, bool movable);
    maru::Turn currentTurn() const { return _currentTurn; }
    void movePiece(const QByteArray &coord);

signals:
    void moved();

protected slots:
    void act();
    void slotPromotionDialog(QAbstractButton *button);

protected:
    void clear();  // オブジェクト削除
    void movePieceByUsi(const QByteArray &coord, maru::Turn turn, bool update);
    void updatePos();
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    //void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    bool createPiece(int pos, Piece::Name name, maru::Turn owner = maru::Sente);
    void setLastMove(int coord);

private:
    bool parse(const QByteArray &sfen);

    Piece *_current {nullptr};
    Square *_lastMovedSquare {nullptr};
    QPair<Piece*, QString> _lastMove;  // 駒, 元のマス(頭に'+'が付いたら"成り")
    maru::Turn _currentTurn {maru::Sente};
    bool _pieceMovable {false};
};


// 最終手表示クラス
class Square : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(int alpha READ alpha WRITE setAlpha)
public:
    Square(QGraphicsItem *parent = nullptr) :
        QObject(),
        QGraphicsPixmapItem(parent),
        _animation(new QPropertyAnimation(this, "alpha"))
    {
        connect(_animation, &QPropertyAnimation::finished, this, &Square::hide);

        _animation->setDuration(7000);
        _animation->setStartValue(160);
        _animation->setEndValue(0);
        _animation->setEasingCurve(QEasingCurve::InExpo);
        hide();
    }

    int alpha() const { return _alpha; }
    void setAlpha(int alpha)
    {
        _alpha = alpha;
        update();
    }

    void update()
    {
        const int R = 255;
        const int G = 119;
        const int B = 62;

        QImage image(QSize(60, 64), QImage::Format_ARGB32);
        image.fill(qRgba(R, G, B, _alpha));
        setPixmap(QPixmap::fromImage(image));
    }

    void highlight()
    {
        _animation->stop();
        show();
        _animation->start();
    }

public slots:
    void hide()
    {
        QGraphicsPixmapItem::hide();
    }

private:
    int _alpha {0};
    QPropertyAnimation *_animation {nullptr};
};
