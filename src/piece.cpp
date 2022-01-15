#include "piece.h"
#include "badge.h"
#include <QRectF>
#include <QVector>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsScene>

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

        insert(Piece::KingOu | Piece::Upward_R, "upward_r_kingou.png");
        insert(Piece::KingGyoku | Piece::Upward_R, "upward_r_kinggyoku.png");
        insert(Piece::Rook | Piece::Upward_R, "upward_r_rook.png");
        insert(Piece::Bishop | Piece::Upward_R, "upward_r_bishop.png");
        insert(Piece::Gold | Piece::Upward_R, "upward_r_gold.png");
        insert(Piece::Silver | Piece::Upward_R, "upward_r_silver.png");
        insert(Piece::Knight | Piece::Upward_R, "upward_r_knight.png");
        insert(Piece::Lance | Piece::Upward_R, "upward_r_lance.png");
        insert(Piece::Pawn | Piece::Upward_R, "upward_r_pawn.png");
        insert(Piece::PromotedRook | Piece::Upward_R, "upward_r_promotedrook.png");
        insert(Piece::PromotedBishop | Piece::Upward_R, "upward_r_promotedbishop.png");
        insert(Piece::PromotedSilver | Piece::Upward_R, "upward_r_promotedsilver.png");
        insert(Piece::PromotedKnight | Piece::Upward_R, "upward_r_promotedknight.png");
        insert(Piece::PromotedLance | Piece::Upward_R, "upward_r_promotedlance.png");
        insert(Piece::PromotedPawn | Piece::Upward_R, "upward_r_promotedpawn.png");
        insert(Piece::KingOu | Piece::Downward_R, "downward_r_kingou.png");
        insert(Piece::KingGyoku | Piece::Downward_R, "downward_r_kinggyoku.png");
        insert(Piece::Rook | Piece::Downward_R, "downward_r_rook.png");
        insert(Piece::Bishop | Piece::Downward_R, "downward_r_bishop.png");
        insert(Piece::Gold | Piece::Downward_R, "downward_r_gold.png");
        insert(Piece::Silver | Piece::Downward_R, "downward_r_silver.png");
        insert(Piece::Knight | Piece::Downward_R, "downward_r_knight.png");
        insert(Piece::Lance | Piece::Downward_R, "downward_r_lance.png");
        insert(Piece::Pawn | Piece::Downward_R, "downward_r_pawn.png");
        insert(Piece::PromotedRook | Piece::Downward_R, "downward_r_promotedrook.png");
        insert(Piece::PromotedBishop | Piece::Downward_R, "downward_r_promotedbishop.png");
        insert(Piece::PromotedSilver | Piece::Downward_R, "downward_r_promotedsilver.png");
        insert(Piece::PromotedKnight | Piece::Downward_R, "downward_r_promotedknight.png");
        insert(Piece::PromotedLance | Piece::Downward_R, "downward_r_promotedlance.png");
        insert(Piece::PromotedPawn | Piece::Downward_R, "downward_r_promotedpawn.png");
    }
};
Q_GLOBAL_STATIC(ImageFileMap, imageFileMap)


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
    _name(name),
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
    auto img = imageFileMap()->value(_name | _orient | (showRotated ? Rotated : 0));
    if (!img.isEmpty()) {
        const QString dir = QLatin1String("assets/images/");
        setPixmap(QPixmap(dir + img));
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
    case Rook:      // 飛
    case Bishop:    // 角
    case Silver:    // 銀
    case Knight:    // 桂
    case Lance:     // 香
    case Pawn:      // 歩
        return (Name)(('+' << 8) | name);
    default:
        return name;
    }
}


Piece::Name Piece::originalName() const
{
    // switch (_name) {
    // case PromotedRook:      // 龍
    // case PromotedBishop:    // 馬
    // case PromotedSilver:    // 成銀
    // case PromotedKnight:    // 成桂
    // case PromotedLance:     // 成香
    // case PromotedPawn:      // と金
    //     return (Name)((int)_name >> 2);
    // default:
    //     return _name;
    // }
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
        insert(Piece::KingOu, QString::fromUtf8("王"));
        insert(Piece::KingGyoku, QString::fromUtf8("玉"));
        insert(Piece::Rook, QString::fromUtf8("飛"));
        insert(Piece::Bishop, QString::fromUtf8("角"));
        insert(Piece::Gold, QString::fromUtf8("金"));
        insert(Piece::Silver, QString::fromUtf8("銀"));
        insert(Piece::Knight, QString::fromUtf8("桂"));
        insert(Piece::Lance, QString::fromUtf8("香"));
        insert(Piece::Pawn, QString::fromUtf8("歩"));
        insert(Piece::PromotedRook, QString::fromUtf8("龍"));
        insert(Piece::PromotedBishop, QString::fromUtf8("馬"));
        insert(Piece::PromotedSilver, QString::fromUtf8("成銀"));
        insert(Piece::PromotedKnight, QString::fromUtf8("成桂"));
        insert(Piece::PromotedLance, QString::fromUtf8("成香"));
        insert(Piece::PromotedPawn, QString::fromUtf8("と"));
    }
};
Q_GLOBAL_STATIC(KanjiMap, kanjiMap)


QString Piece::kanjiName() const
{
    return kanjiMap()->value(name());
}

QString Piece::kanjiName(Name name)
{
    return kanjiMap()->value(name);
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
