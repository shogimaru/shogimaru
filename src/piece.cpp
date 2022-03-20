#include "piece.h"
#include "badge.h"
#include "user.h"
#include <QCoreApplication>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QRectF>
#include <QVector>

static volatile bool showRotated = false;

class ImageFileMap : public QMap<int, QString> {
public:
    ImageFileMap() :
        QMap<int, QString>()
    {
        insert(Piece::KingOu | Piece::Upward, "upward_kingou.png");
        insert(Piece::KingGyoku | Piece::Upward, "upward_kinggyoku.png");
        insert(Piece::Rook | Piece::Upward, "upward_rook.png");
        insert(Piece::Bishop | Piece::Upward, "upward_bishop.png");
        insert(Piece::Gold | Piece::Upward, "upward_gold.png");
        insert(Piece::Silver | Piece::Upward, "upward_silver.png");
        insert(Piece::Knight | Piece::Upward, "upward_knight.png");
        insert(Piece::Lance | Piece::Upward, "upward_lance.png");
        insert(Piece::Pawn | Piece::Upward, "upward_pawn.png");
        insert(Piece::PromotedRook | Piece::Upward, "upward_promotedrook.png");
        insert(Piece::PromotedBishop | Piece::Upward, "upward_promotedbishop.png");
        insert(Piece::PromotedSilver | Piece::Upward, "upward_promotedsilver.png");
        insert(Piece::PromotedKnight | Piece::Upward, "upward_promotedknight.png");
        insert(Piece::PromotedLance | Piece::Upward, "upward_promotedlance.png");
        insert(Piece::PromotedPawn | Piece::Upward, "upward_promotedpawn.png");
        insert(Piece::KingOu | Piece::Downward, "downward_kingou.png");
        insert(Piece::KingGyoku | Piece::Downward, "downward_kinggyoku.png");
        insert(Piece::Rook | Piece::Downward, "downward_rook.png");
        insert(Piece::Bishop | Piece::Downward, "downward_bishop.png");
        insert(Piece::Gold | Piece::Downward, "downward_gold.png");
        insert(Piece::Silver | Piece::Downward, "downward_silver.png");
        insert(Piece::Knight | Piece::Downward, "downward_knight.png");
        insert(Piece::Lance | Piece::Downward, "downward_lance.png");
        insert(Piece::Pawn | Piece::Downward, "downward_pawn.png");
        insert(Piece::PromotedRook | Piece::Downward, "downward_promotedrook.png");
        insert(Piece::PromotedBishop | Piece::Downward, "downward_promotedbishop.png");
        insert(Piece::PromotedSilver | Piece::Downward, "downward_promotedsilver.png");
        insert(Piece::PromotedKnight | Piece::Downward, "downward_promotedknight.png");
        insert(Piece::PromotedLance | Piece::Downward, "downward_promotedlance.png");
        insert(Piece::PromotedPawn | Piece::Downward, "downward_promotedpawn.png");
        // 回転
        insert(Piece::KingOu | Piece::Upward_R, "downward_kingou.png");
        insert(Piece::KingGyoku | Piece::Upward_R, "downward_kinggyoku.png");
        insert(Piece::Rook | Piece::Upward_R, "downward_rook.png");
        insert(Piece::Bishop | Piece::Upward_R, "downward_bishop.png");
        insert(Piece::Gold | Piece::Upward_R, "downward_gold.png");
        insert(Piece::Silver | Piece::Upward_R, "downward_silver.png");
        insert(Piece::Knight | Piece::Upward_R, "downward_knight.png");
        insert(Piece::Lance | Piece::Upward_R, "downward_lance.png");
        insert(Piece::Pawn | Piece::Upward_R, "downward_pawn.png");
        insert(Piece::PromotedRook | Piece::Upward_R, "downward_promotedrook.png");
        insert(Piece::PromotedBishop | Piece::Upward_R, "downward_promotedbishop.png");
        insert(Piece::PromotedSilver | Piece::Upward_R, "downward_promotedsilver.png");
        insert(Piece::PromotedKnight | Piece::Upward_R, "downward_promotedknight.png");
        insert(Piece::PromotedLance | Piece::Upward_R, "downward_promotedlance.png");
        insert(Piece::PromotedPawn | Piece::Upward_R, "downward_promotedpawn.png");
        insert(Piece::KingOu | Piece::Downward_R, "upward_kingou.png");
        insert(Piece::KingGyoku | Piece::Downward_R, "upward_kinggyoku.png");
        insert(Piece::Rook | Piece::Downward_R, "upward_rook.png");
        insert(Piece::Bishop | Piece::Downward_R, "upward_bishop.png");
        insert(Piece::Gold | Piece::Downward_R, "upward_gold.png");
        insert(Piece::Silver | Piece::Downward_R, "upward_silver.png");
        insert(Piece::Knight | Piece::Downward_R, "upward_knight.png");
        insert(Piece::Lance | Piece::Downward_R, "upward_lance.png");
        insert(Piece::Pawn | Piece::Downward_R, "upward_pawn.png");
        insert(Piece::PromotedRook | Piece::Downward_R, "upward_promotedrook.png");
        insert(Piece::PromotedBishop | Piece::Downward_R, "upward_promotedbishop.png");
        insert(Piece::PromotedSilver | Piece::Downward_R, "upward_promotedsilver.png");
        insert(Piece::PromotedKnight | Piece::Downward_R, "upward_promotedknight.png");
        insert(Piece::PromotedLance | Piece::Downward_R, "upward_promotedlance.png");
        insert(Piece::PromotedPawn | Piece::Downward_R, "upward_promotedpawn.png");
    }
};


static QPixmap piecePixmap(int piece, int type)
{
    static QMap<int, QPixmap> pixmapMap;
    static ImageFileMap imageFileMap;

    if (!(piece & Piece::NameMask)) {
        return QPixmap();
    }

    int key = (type << 24) | piece;
    QPixmap pix = pixmapMap.value(key);
    if (pix.isNull()) {
        auto img = imageFileMap.value(piece);
        if (!img.isEmpty()) {
            const QString dir = QLatin1String("assets/images/") + QString::number(type) + "/";
            pix = QPixmap(dir + img);
            if (piece & Piece::Rotated) {
                // 180度回転
                QTransform transform;
                transform.translate(pix.width(), pix.height());
                transform.rotate(180);
                pix = pix.transformed(transform);
            }
            pixmapMap.insert(key, pix);
        } else {
            qDebug() << "piece:" << piece << "type:" << type;
        }
    }
    return pix;
}


Piece::Piece(QGraphicsItem *parent) :
    QObject(),
    QGraphicsPixmapItem(parent),
    _name(Name::None)
{
    // 透明駒
    static QPixmap square = []() {
        QImage image(QSize(60, 64), QImage::Format_ARGB32_Premultiplied);
        image.fill(qRgba(0, 0, 0, 1));  // ほぼ透明
        return QPixmap::fromImage(image);
    }();

    setPixmap(square);
    setFlags(QGraphicsItem::ItemIsSelectable);
}


Piece::Piece(Name name, maru::Turn owner, QGraphicsItem *parent) :
    QObject(),
    QGraphicsPixmapItem(parent),
    _name((owner == maru::Gote && name == KingGyoku) ? KingOu : name),  // 後手は王にする
    _owner(owner),
    _orient((_owner == maru::Sente) ? Upward : Downward),
    _badge(new Badge(0, (_orient == Upward) ? 0 : 180, this))
{
    setTransformationMode(Qt::SmoothTransformation);
    reload();
}


Piece::~Piece()
{
    delete _badge;
}


QByteArray Piece::sfen() const
{
    QByteArray str;

    if (name() == None) {
        return str;
    }

    int n = (name() == KingOu) ? KingGyoku : name();
    if (n & 0xFF00) {
        str += (char)((n & 0xFF00) >> 8);  // '+'
    }
    str += (char)(n & 0xFF);  // 駒
    str = (owner() == maru::Sente) ? str.toUpper() : str.toLower();
    return str;
}


// QRectF Piece::boundingRect() const
// {
//     //return QRectF(-30, -32, 60, 64);  // 回転を中心の持ってくるため
// }


// 再描画
void Piece::reload()
{
    auto pix = piecePixmap(_name | _orient | (showRotated ? Rotated : 0), User::load().pieceType());
    if (!pix.isNull()) {
        setPixmap(pix);
        setFlags(QGraphicsItem::ItemIsSelectable);
    }

    if (_badge) {
        if (_orient == Upward) {
            _badge->setPos(boundingRect().width() - _badge->boundingRect().width() - 1, 0);
        } else {
            _badge->setPos(0, boundingRect().height() - _badge->boundingRect().height() - 1);
        }
    }
}


void Piece::advance(int phase)
{
    if (!phase) {
        reload();
    }
}


bool Piece::isPromoted() const
{
    return ('+' << 8) & _name;
}


Piece::Name Piece::promotedName() const
{
    return promotedName(_name);
}


Piece::Name Piece::promotedName(Name name)
{
    switch (name) {
    case Rook:  // 飛
    case Bishop:  // 角
    case Silver:  // 銀
    case Knight:  // 桂
    case Lance:  // 香
    case Pawn:  // 歩
        return (Name)(('+' << 8) | name);
    default:
        return name;
    }
}


Piece::Name Piece::originalName() const
{
    return (Name)(_name & 0xff);
}


void Piece::setBadge(int number)
{
    if (_badge) {
        _badge->setNumber(number);
    }
}


// QRectF Piece::boundingRect() const
// {
//     return QGraphicsPixmapItem::boundingRect();
// }


void Piece::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    // std::cout << "piece::mousePressEvent: " << (char)_name << " " << _owner << std::endl;
    // std::cout << "piece::mousePressEvent: " << (int)e->pos().x() << " " << (int)e->pos().y()  << std::endl;
    QGraphicsPixmapItem::mousePressEvent(e);
}


// void Piece::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
// {
//     std::cout << "piece::mouseMoveEvent: " << (char)_name << " " << _owner << std::endl;
//     std::cout << "boundingRect: " << boundingRect().width() << " " << boundingRect().height() << std::endl;
//     QGraphicsPixmapItem::mouseMoveEvent(e);
// }


void Piece::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && (e->buttonDownPos(Qt::LeftButton) - e->pos()).manhattanLength() <= 20) {
        setSelected(true);
        emit clicked();
    } else {
        QGraphicsPixmapItem::mouseReleaseEvent(e);
    }
}


void Piece::setRotation(bool rotate)
{
    showRotated = rotate;
}


class KanjiMap : public QMap<Piece::Name, QString> {
public:
    KanjiMap() :
        QMap<Piece::Name, QString>()
    {
        insert(Piece::KingOu, QCoreApplication::translate("KanjiMap", "K", "Ou"));  // 王
        insert(Piece::KingGyoku, QCoreApplication::translate("KanjiMap", "K", "Gyoku"));  // 玉
        insert(Piece::Rook, QCoreApplication::translate("KanjiMap", "R"));  // 飛
        insert(Piece::Bishop, QCoreApplication::translate("KanjiMap", "B"));  // 角
        insert(Piece::Gold, QCoreApplication::translate("KanjiMap", "G"));  // 金
        insert(Piece::Silver, QCoreApplication::translate("KanjiMap", "S"));  // 銀
        insert(Piece::Knight, QCoreApplication::translate("KanjiMap", "N"));  // 桂
        insert(Piece::Lance, QCoreApplication::translate("KanjiMap", "L"));  // 香
        insert(Piece::Pawn, QCoreApplication::translate("KanjiMap", "P"));  // 歩
        insert(Piece::PromotedRook, QCoreApplication::translate("KanjiMap", "+R"));  // 龍
        insert(Piece::PromotedBishop, QCoreApplication::translate("KanjiMap", "+B"));  // 馬
        insert(Piece::PromotedSilver, QCoreApplication::translate("KanjiMap", "+S"));  // 成銀
        insert(Piece::PromotedKnight, QCoreApplication::translate("KanjiMap", "+N"));  // 成桂
        insert(Piece::PromotedLance, QCoreApplication::translate("KanjiMap", "+L"));  // 成香
        insert(Piece::PromotedPawn, QCoreApplication::translate("KanjiMap", "+P"));  // と
    }
};
static const KanjiMap &kanjiMap()
{
    static KanjiMap map;
    return map;
}


QString Piece::kanjiName() const
{
    return kanjiMap().value(name());
}

QString Piece::kanjiName(Name name)
{
    return kanjiMap().value(name);
}


QPair<Piece::Name, maru::Turn> Piece::name(const QByteArray &piece)
{
    if (piece.isEmpty()) {
        return QPair<Piece::Name, maru::Turn>(None, maru::Sente);
    }

    bool promoted = false;
    char p = piece[0];

    if (p == '+' && piece.length() > 1) {
        p = piece[1];
        promoted = true;
    }
    return name(p, promoted);
}


QPair<Piece::Name, maru::Turn> Piece::name(char p, bool promoted)
{
    Name n = None;
    maru::Turn turn = (p >= 'A' && p <= 'Z') ? maru::Sente : maru::Gote;
    p = std::tolower(p);

    switch (p) {
    case KingOu:
    case KingGyoku:
    case Rook:
    case Bishop:
    case Gold:
    case Silver:
    case Knight:
    case Lance:
    case Pawn:
        n = (promoted) ? promotedName((Name)p) : (Name)p;
        break;
    default:
        return QPair<Piece::Name, maru::Turn>(None, maru::Sente);
    }

    return QPair<Piece::Name, maru::Turn>(n, turn);
}
