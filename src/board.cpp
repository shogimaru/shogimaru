#include "board.h"
#include "shogirecord.h"
#include "badge.h"
#include "promotiondialog.h"
#include "sfen.h"
#include <QPainter>
#include <QRect>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>
#include <QThread>
#include <QMessageBox>
#include <QDebug>

constexpr int NUM_ROWS = 9;


Board::Board(QObject *parent) :
    QGraphicsScene(parent),
    _lastMovedSquare(new Square)
{
    parse("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
    addItem(_lastMovedSquare);
}


bool Board::parse(const QByteArray &sfen)
{
    clear();
    Sfen parser(sfen);

    for (int row = 1; row < 10; row++) {
        for (int col = 1; col < 10; col++) {
            int coord = col * 10 + row;
            auto str = parser._position.value(coord);
            auto pname = Piece::name(str);
            createPiece(coord, pname.first, pname.second);
        }
    }

    // 持ち駒
    for (auto p : parser._inHand) {
        auto pname = Piece::name(p);
        if (pname.first != Piece::None) {
            createPiece(pname.second, pname.first, pname.second);
        }
    }

    // 手番
    _currentTurn = parser._turn;

    updatePos();
    return true;
}


bool Board::startGame(const QByteArray &sfen)
{
    _current = nullptr;
    _lastMove = QPair<Piece*, QString>(nullptr, QString());
    return parse(sfen);
}


void Board::stopGame()
{
    PromotionDialog::abort();
    _pieceMovable = false;
    _lastMovedSquare->hide();
}


bool Board::createPiece(int coord, Piece::Name name, maru::Turn owner)
{
    Piece *piece = nullptr;
    if (name == Piece::None) {
        piece = new Piece;
    } else {
        piece = new Piece(name, owner);
    }

    piece->setData(maru::Coord, coord);
    addItem(piece);
    connect(piece, &Piece::clicked, this, &Board::act);
    return piece;
}


void Board::updatePos()
{
    static QMap<Piece::Name, int> minWidthMap = []() {
        QMap<Piece::Name, int> map;
        map.insert(Piece::KingOu,    54);
        map.insert(Piece::KingGyoku, 54);
        map.insert(Piece::Rook,      54);
        map.insert(Piece::Bishop,    54);
        map.insert(Piece::Gold,      52);
        map.insert(Piece::Silver,    50);
        map.insert(Piece::Knight,    48);
        map.insert(Piece::Lance,     45);
        map.insert(Piece::Pawn,      41);
        return map;
    }();

    auto priority = [](Piece::Name name) {
        switch (name) {
        case Piece::KingOu:    return 9;
        case Piece::KingGyoku: return 8;
        case Piece::Rook:      return 7;
        case Piece::Bishop:    return 6;
        case Piece::Gold:      return 5;
        case Piece::Silver:    return 4;
        case Piece::Knight:    return 3;
        case Piece::Lance:     return 2;
        case Piece::Pawn:      return 1;
        default:               return 0;
        }
    };

    auto compare = [=](const Piece *p1, const Piece *p2) {
        return priority(p1->name()) > priority(p2->name());
    };

    QList<Piece*> piecesSenteInHand;
    QList<Piece*> piecesGoteInHand;

    for (auto *piece : items()) {
        Piece *p = dynamic_cast<Piece*>(piece);
        if (p) {
            int crd = p->data(maru::Coord).toInt();
            if (crd >= 11 && crd <= 99) {
                // 盤上
                int col = NUM_ROWS - (crd / 10);
                int row = (crd % 10) - 1;
                p->setPos(col * 60.0 + 30.0, row * 64.0 + 102.0);
            } else if (crd == maru::Sente) {
                // 先手持ち駒
                piecesSenteInHand << p;
            } else if (crd == maru::Gote) {
                // 後手持ち駒
                piecesGoteInHand << p;
            } else {
                // do nothing
            }
        }
    }

    // バッジ表示ラムダ
    auto displayBadge = [=](QList<Piece*> &pieces, int y, int diff) {
        QMap<Piece::Name, int> counts;
        for (const auto &p : pieces) {
            counts[p->name()]++;
        }

        QList<Piece::Name> order = counts.keys();
        std::sort(order.begin(), order.end(), [&](Piece::Name n1, Piece::Name n2) {
            if (counts[n1] == counts[n2]) {
                return priority(n1) < priority(n2);
            }
            return counts[n1] > counts[n2];
        });

        int num = 1;
        Piece *piece;
        do {
            piece = nullptr;
            int px = (diff > 0) ? 8 : 532;
            QList<Piece::Name> badgeOrder = order.mid(0, num);

            for (auto *p : pieces) {
                if (piece) {
                    if (p->name() == piece->name() && badgeOrder.contains(p->name())) {
                        p->hide();
                        piece->setBadge(counts[p->name()]);
                        continue;
                    } else {
                        px += 58 * diff;
                    }
                }
                p->setPos(px, y);
                p->show();
                piece = p;
            }
            num++;
        } while (!sceneRect().contains(piece->sceneBoundingRect()));
    };

    auto display = [=](QList<Piece*> &pieces, int x, int y, int diff) {
        if (pieces.isEmpty()) {
            return;
        }

        std::sort(pieces.begin(), pieces.end(), compare);
        int basew = pieces.first()->boundingRect().width();
        Piece *piece = nullptr;

        // 駒フラット表示
        do {
            int px = x;
            for (const auto p : pieces) {
                p->setPos(px, y);
                p->setBadge(0);
                p->show();
                px += qMax(basew, minWidthMap.value(p->name())) * diff;
                piece = p;
            }

            x = qBound(0, x - (int)piece->boundingRect().width() * diff, 540);
            if (x == 0 || x == 540) {
                basew -= 2;
            }
        } while (basew > 38 && !sceneRect().contains(piece->sceneBoundingRect()));

        if (!sceneRect().contains(piece->sceneBoundingRect())) {
            // バッチ表示
            displayBadge(pieces, y, diff);
        }
    };

    display(piecesSenteInHand, 414, 713, 1);
    display(piecesGoteInHand, 126, 3, -1);
    update();
}


void Board::clear()
{
    auto itemList = items();  // copy
    for (auto piece : itemList) {
        delete dynamic_cast<Piece*>(piece);
    }

    _current = nullptr;
    _lastMove = QPair<Piece*, QString>(nullptr, QString());
    update();
}


QPixmap Board::board()
{
    static QPixmap boardPixmap = []() {
        QPixmap pixmap(600, 780);
        QPixmap board("assets/images/shogiboard-bg.jpg");
        QPixmap line("assets/images/shogiboard-l.png");
        QPixmap stand("assets/images/shogistand.jpg");

        QPainter painter(&pixmap);
        painter.drawPixmap(0,   0, stand, 0, 0, stand.width(), stand.height());
        painter.drawPixmap(0, 710, stand, 0, 0, stand.width(), stand.height());
        painter.drawPixmap(0,  70, board, 0, 0, board.width(), board.height());
        painter.drawPixmap(0,  70, line,  0, 0, line.width(), line.height());
        return pixmap;
    }();
    return boardPixmap;
}


void Board::act()
{
    if (!_pieceMovable) {
        return;
    }

    Piece *dst = dynamic_cast<Piece*>(sender());

    if (!dst) {
        return;
    }

    if (!_current) {
        if (dst->name() != Piece::None) {
            // 駒選択
            _current = (_currentTurn == dst->owner()) ? dst : nullptr;
        } else {
            _current = nullptr;
        }
        return;
    }

    // 自分の駒がある場所には移動できない
    if (dst->name() != Piece::None && _current->owner() == dst->owner()) {
        // 駒選択
        _current = (_currentTurn == dst->owner()) ? dst : nullptr;
        return;
    }

    int dstCrd = dst->data(maru::Coord).toInt();

    // 駒移動 or 打てるか
    if (!canMove(_current, dstCrd)) {
        return;
    }

/*
    // 玉は相手の駒が利いているマスには移動しない
    if (_current->name() == Piece::KingOu || _current->name() == Piece::KingGyoku) {
        auto opponent = (_currentTurn == maru::Sente) ? maru::Gote : maru::Sente;
        if (searchMovablePeace(opponent, dstCrd)) {
            // 移動しない
            return;
        }
    }
*/

    // 王、玉、金は成らずに移動
    if (_current->name() == Piece::KingOu || _current->name() == Piece::KingGyoku || _current->name() == Piece::Gold) {
        move(_current, dst, false);
        _current = nullptr;
        return;
    }

    // 持ち駒は打つ（成れない）
    int currentCrd = _current->data(maru::Coord).toInt();
    if (currentCrd < 11 || currentCrd > 99) {
        move(_current, dst, false);
        _current = nullptr;
        return;
    }

    // 成り駒は移動
    if (_current->isPromoted()) {
        move(_current, dst, true);
        _current = nullptr;
        return;
    }

    int currentRow = currentCrd % 10;
    int dstRow = dstCrd % 10;

    // 歩、香、桂の場合にルール上 成らなければならないマスは強制的に成る
    switch (_current->name()) {
    case Piece::Pawn:
    case Piece::Lance:
        if ((_current->owner() == maru::Sente && dstRow == 1) || (_current->owner() == maru::Gote && dstRow == 9)) {
            move(_current, dst, true);
            _current = nullptr;
            return;
        }
        break;

    case Piece::Knight:
        if ((_current->owner() == maru::Sente && dstRow <= 2) || (_current->owner() == maru::Gote && dstRow >= 8)) {
            move(_current, dst, true);
            _current = nullptr;
            return;
        }
        break;
    default:
        break;
    }

    if ((_current->owner() == maru::Sente && (currentRow < 4 || dstRow < 4)) ||
        (_current->owner() == maru::Gote && (currentRow > 6 || dstRow > 6))) {
        // 成りor成らずダイアログ
        PromotionDialog::question(_current, dst, this, SLOT(slotPromotionDialog(QAbstractButton*)));
        return;
    }

    move(_current, dst, false);
    _current = nullptr;
}


// 指定マスへ移動できる駒を探索
QList<Piece*> Board::searchMovablePeace(maru::Turn turn, int coord) const
{
    QList<Piece*> ret;

    for (QGraphicsItem *item : items()) {
        Piece *piece = dynamic_cast<Piece*>(item);
        if (piece && piece->name() != Piece::None) {
            int crd = piece->data(maru::Coord).toInt();
            if (crd >= 11 && crd <= 99 && piece->owner() == turn) {
                if (canMove(piece, coord)) {
                    ret << piece;
                }
            }
        }
    }
    return ret;
}


void Board::slotPromotionDialog(QAbstractButton *button)
{
    auto *dialog = dynamic_cast<PromotionDialog*>(sender());
    if (dialog) {
        bool promote = (dialog->buttonRole(button) == QMessageBox::YesRole);
        move(dialog->src(), dialog->dst(), promote);
        _current = nullptr;
    }
}


void Board::movePiece(const QByteArray &coord)
{
    movePieceByUsi(coord, _currentTurn, true);
}


void Board::movePieceByUsi(const QByteArray &coord, maru::Turn turn, bool update)
{
    if (coord.length() != 4 && coord.length() != 5) {
        return;
    }

    Piece *from = nullptr;
    if (std::isalpha(coord[0])) {
        // 持ち駒
        from = pieceInHand(turn, (Piece::Name)std::tolower(coord[0]));
    } else {
        from = piece(ShogiRecord::usiToCoord(coord.mid(0, 2)));
    }

    auto *to = piece(ShogiRecord::usiToCoord(coord.mid(2, 2)));
    bool promote = (coord.length() == 5 && coord[4] == '+');

    if (!from || !to) {
        return;
    }
    move(from, to, promote, update);
}


static auto search_piece = [](const QList<QGraphicsItem*> &items, int coord) -> Piece*
{
    for (auto *item : items) {
        Piece* p = dynamic_cast<Piece*>(item);
        if (p) {
            if (p->data(maru::Coord).toInt() == coord) {
                return p;
            }
        }
    }
    return nullptr;
};


const Piece *Board::piece(int coord) const
{
    return search_piece(items(), coord);
}


Piece *Board::piece(int coord)
{
    return search_piece(items(), coord);
}


QList<Piece*> Board::pieces(maru::Turn owner, Piece::Name name) const
{
    QList<Piece*> pieces;

    for (auto *piece : items()) {
        Piece* p = dynamic_cast<Piece*>(piece);
        if (p) {
            int crd = p->data(maru::Coord).toInt();
            if (p->name() == name && p->owner() == owner && crd >= 11 && crd <= 99) {
                pieces << (Piece*)p;
            }
        }
    }
    return pieces;
}


Piece *Board::pieceInHand(maru::Turn owner, Piece::Name name) const
{
    for (auto *p : piecesInHand(owner)) {
        if (p->name() == name) {
            return p;
        }
    }
    return nullptr;
}


QList<Piece*> Board::piecesInHand(maru::Turn owner) const
{
    QList<Piece*> pieces;

    for (auto *p : items()) {
        p = dynamic_cast<Piece*>(p);
        if (p) {
            if (p->data(maru::Coord).toInt() == owner) {
                pieces << (Piece*)p;
            }
        }
    }
    return pieces;
}


int Board::kingCoord(maru::Turn owner) const
{
    QList<Piece*> plist;
    plist << pieces(owner, Piece::KingOu);
    plist << pieces(owner, Piece::KingGyoku);
    const Piece *piece = plist.first();
    return piece->data(maru::Coord).toInt();
}


void Board::move(Piece *from, Piece *to, bool promote, bool update)
{
    if (from->name() == Piece::None) {
        return;
    }

    if (to->name() != Piece::None && from->owner() == to->owner()) {
        return;
    }

    from->setBadge(0);
    int fromcrd = from->data(maru::Coord).toInt();  // 元のマス位置
    int crd = to->data(maru::Coord).toInt();  // 先のマス位置
    if (!crd) {
        qDebug() << "items:" << items().count();
    }

    if (to->name() != Piece::None && from->owner() != to->owner()) {
        // 駒取得
        createPiece(from->owner(), to->originalName(), from->owner());
    }
    delete to;

    // 駒移動
    QString crdstr;
    if (!from->isPromoted() && promote) {
        // 成り
        createPiece(crd, from->promotedName(), from->owner());
        delete from;
        from = piece(crd);
        crdstr = QLatin1Char('+');
    } else {
        from->setData(maru::Coord, crd);
    }

    // 最終手
    setLastMove(crd);

    if (fromcrd >= 11 && fromcrd <= 99) {
        // マス目生成
        createPiece(fromcrd, Piece::None);
    }

    // 最終手
    _lastMove.first = from;
    _lastMove.second = crdstr + QString::number(fromcrd);  // 元のマス位置

    if (update) {
        updatePos();
        emit moved();
    }
}


void Board::setLastMove(int coord)
{
    int col = NUM_ROWS - (coord / 10);
    int row = (coord % 10) - 1;
    _lastMovedSquare->setPos(col * 60.0 + 30.0, row * 64.0 + 102.0);
    _lastMovedSquare->highlight();
}

/*
void Board::resetPosition(const QStringList &position)
{
    init();
    maru::Turn turn = maru::Sente;
    for (auto &pos : position) {
        movePieceByUsi(pos, turn, false);
        turn = (turn == maru::Sente) ? maru::Gote : maru::Sente;
    }
    _current = nullptr;

    _lastMovedSquare->hide();
    updatePos();
}
*/

using Coords = QVector<int>;  // マスの相対座標
using MovableCoords = QVector<Coords>;  // 駒の移動可能な相対座標

class MovableCoordsMap : public QMap<int, MovableCoords> {
public:
    MovableCoordsMap() :
        QMap<int, MovableCoords>()
    {
        //     手番          駒,                             移動可能マス(1マス),  長距離移動可能方向,  打てるマス
        insert(maru::Sente | Piece::KingOu,    MovableCoords{Coords{-11, -10, -9, -1, 1, 9, 10, 11}, Coords{}, Coords{}});
        insert(maru::Sente | Piece::KingGyoku, MovableCoords{Coords{-11, -10, -9, -1, 1, 9, 10, 11}, Coords{}, Coords{}});
        insert(maru::Sente | Piece::Rook,      MovableCoords{Coords{}, Coords{-10, -1, 1, 10}, Coords{1, 2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Sente | Piece::Bishop,    MovableCoords{Coords{}, Coords{-11, -9, 9, 11}, Coords{1, 2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Sente | Piece::Gold,      MovableCoords{Coords{-11, -10, -1, 1, 9, 10}, Coords{}, Coords{1, 2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Sente | Piece::Silver,    MovableCoords{Coords{-11, -9, -1, 9, 11}, Coords{}, Coords{1, 2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Sente | Piece::Knight,    MovableCoords{Coords{-12, 8}, Coords{}, Coords{3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Sente | Piece::Lance,     MovableCoords{Coords{}, Coords{-1}, Coords{2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Sente | Piece::Pawn,      MovableCoords{Coords{-1}, Coords{}, Coords{2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Sente | Piece::PromotedRook,   MovableCoords{Coords{-11, -9, 9, 11}, Coords{-10, -1, 1, 10}, Coords{}});
        insert(maru::Sente | Piece::PromotedBishop, MovableCoords{Coords{-10, -1, 1, 10}, Coords{-11, -9, 9, 11}, Coords{}});
        insert(maru::Sente | Piece::PromotedSilver, MovableCoords{Coords{-11, -10, -1, 1, 9, 10}, Coords{}, Coords{}});
        insert(maru::Sente | Piece::PromotedKnight, MovableCoords{Coords{-11, -10, -1, 1, 9, 10}, Coords{}, Coords{}});
        insert(maru::Sente | Piece::PromotedLance,  MovableCoords{Coords{-11, -10, -1, 1, 9, 10}, Coords{}, Coords{}});
        insert(maru::Sente | Piece::PromotedPawn,   MovableCoords{Coords{-11, -10, -1, 1, 9, 10}, Coords{}, Coords{}});

        insert(maru::Gote | Piece::KingOu,     MovableCoords{Coords{-11, -10, -9, -1, 1, 9, 10, 11}, Coords{}, Coords{}});
        insert(maru::Gote | Piece::KingGyoku,  MovableCoords{Coords{-11, -10, -9, -1, 1, 9, 10, 11}, Coords{}, Coords{}});
        insert(maru::Gote | Piece::Rook,       MovableCoords{Coords{}, Coords{-10, -1, 1, 10}, Coords{1, 2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Gote | Piece::Bishop,     MovableCoords{Coords{}, Coords{-11, -9, 9, 11}, Coords{1, 2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Gote | Piece::Gold,       MovableCoords{Coords{-10, -9, -1, 1, 10, 11}, Coords{}, Coords{1, 2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Gote | Piece::Silver,     MovableCoords{Coords{-11, -9, 1, 9, 11}, Coords{}, Coords{1, 2, 3, 4, 5, 6, 7, 8, 9}});
        insert(maru::Gote | Piece::Knight,     MovableCoords{Coords{-8, 12}, Coords{}, Coords{1, 2, 3, 4, 5, 6, 7}});
        insert(maru::Gote | Piece::Lance,      MovableCoords{Coords{}, Coords{1}, Coords{1, 2, 3, 4, 5, 6, 7, 8}});
        insert(maru::Gote | Piece::Pawn,       MovableCoords{Coords{1}, Coords{}, Coords{1, 2, 3, 4, 5, 6, 7, 8}});
        insert(maru::Gote | Piece::PromotedRook,   MovableCoords{Coords{-11, -9, 9, 11}, Coords{-10, -1, 1, 10}, Coords{}});
        insert(maru::Gote | Piece::PromotedBishop, MovableCoords{Coords{-10, -1, 1, 10}, Coords{-11, -9, 9, 11}, Coords{}});
        insert(maru::Gote | Piece::PromotedSilver, MovableCoords{Coords{-10, -9, -1, 1, 10, 11}, Coords{}, Coords{}});
        insert(maru::Gote | Piece::PromotedKnight, MovableCoords{Coords{-10, -9, -1, 1, 10, 11}, Coords{}, Coords{}});
        insert(maru::Gote | Piece::PromotedLance,  MovableCoords{Coords{-10, -9, -1, 1, 10, 11}, Coords{}, Coords{}});
        insert(maru::Gote | Piece::PromotedPawn,   MovableCoords{Coords{-10, -9, -1, 1, 10, 11}, Coords{}, Coords{}});
    }
};
Q_GLOBAL_STATIC(MovableCoordsMap, movableCoordsMap)


// 駒として移動できるマスか／駒が打てるかの判定
bool Board::canMove(const Piece *piece, int coord) const
{
    bool ret = false;

    if (!piece || piece->name() == Piece::None) {
        return ret;
    }

    MovableCoords crds = movableCoordsMap()->value(piece->owner() | piece->name());
    const int currentCrd = piece->data(maru::Coord).toInt();

    if (currentCrd < 11 || currentCrd > 99) {  // 駒打ち
        // マスに駒がないか
        auto *square = Board::piece(coord);
        if (square->name() != Piece::None) {
            return ret;
        }

        int row = coord % 10;
        ret = crds[2].contains(row);

    } else {
        // 駒移動
        ret = crds[0].contains(coord - currentCrd);  // 1マス移動として
        if (!ret) {
            // 長距離移動
            for (int d : crds[1]) {
                int crt = currentCrd;
                while (crt >= 11 && crt <= 99) {
                    crt += d;
                    if (crt == coord) {
                        ret = true;
                        break;
                    }

                    auto *p = Board::piece(crt);
                    if (!p || p->name() != Piece::None) {
                        break;
                    }
                }
            }
        }
    }
    return ret;
}


bool Board::isCheck() const
{
    auto *piece = lastMovedPiece().first;
    if (piece) {
        maru::Turn opponent = (piece->owner() == maru::Sente) ? maru::Gote : maru::Sente;
        return canMove(piece, kingCoord(opponent));
    }
    return false;
}

#if 0
QByteArray Board::toStaticSfen(int moveNumber) const
{
    return toStaticSfen(_currentTurn, moveNumber);
}

// 現局面をSFEN表現（指し手無し）  moveNumber: 開始局面は1、1手指した直後は2
QByteArray Board::toStaticSfen(maru::Turn turn, int moveNumber) const
{
    QByteArray sfen;
    sfen.reserve(128);

    // 駒の配置
    for (int i = 1; i < 10; i++) {
        int s = 0;
        for (int j = 9; j > 0; j--) {
            int crd =  i + j * 10;
            auto p = piece(crd)->sfen();
            if (p.isEmpty()) {
                s++;
            } else {
                if (s > 0) {
                    sfen += QByteArray::number(s);
                    s = 0;
                }
                sfen += p;
            }
        }

        if (s > 0) {
            sfen += QByteArray::number(s);
        }
        sfen += "/";
    }
    sfen[sfen.length() - 1] = ' ';

    // 手番
    sfen += (turn == maru::Sente) ? "b " : "w ";

    // 持ち駒カウント
    auto counts = [](const QList<Piece*> pieces) {
        QByteArray str;
        str.reserve(pieces.count() + 1);
        for (const auto *p : pieces) {
            str += p->sfen();
        }

        QByteArray names = "rbgsnlp";
        if (str.isUpper()) {
            names = names.toUpper();
        }

        QByteArray ret;
        ret.reserve(pieces.count() + 1);
        for (auto c : names) {
            int cnt = str.count(c);
            if (cnt == 0) {
                continue;
            }

            if (cnt > 1) {
                ret += QByteArray::number(cnt);
            }
            ret += str[str.indexOf(c, 0)];
        }
        return ret;
    };

    // 持ち駒
    QByteArray str = counts(piecesInHand(maru::Sente));
    str += counts(piecesInHand(maru::Gote));
    if (str.isEmpty()) {
        sfen += "-";  // 無し
    } else {
        sfen += str;
    }

    // 手数
    sfen += ' ';
    sfen += QByteArray::number(moveNumber);
    return sfen;
}
#endif

void Board::setSfen(const QByteArray &sfen, bool movable, const QByteArray &lastMove)
{
    qDebug() << "setSfen" << sfen;
    parse(sfen);
    setTurn(_currentTurn, movable);

    QByteArray move = lastMove;
    if (lastMove.isEmpty() && sfen.contains("moves")) {
        move = sfen.split(' ').last();
    }

    int crd = ShogiRecord::usiToCoord(move.mid(2, 2));
    if (crd > 0) {
        // 最終手
        setLastMove(crd);
    } else {
        _lastMovedSquare->hide();
    }
}


void Board::setTurn(maru::Turn turn, bool movable)
{
    _currentTurn = turn;
    _pieceMovable = movable;
    _current = nullptr;
    //qDebug() << "SFEN: " << qPrintable(toSfen());
}


void Board::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    //qDebug() << "Board::mousePressEvent: " << e->scenePos().x() << " " << e->scenePos().y();
    QGraphicsScene::mousePressEvent(e);
}


void Board::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    //qDebug() << "Board::mouseReleaseEvent: " << e->scenePos().x() << " " << e->scenePos().y();
    // Clear selected
    for (auto *item : QGraphicsScene::selectedItems()) {
        item->setSelected(false);
    }
    QGraphicsScene::mouseReleaseEvent(e);
}
