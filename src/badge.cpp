#include "badge.h"
#include <QPainter>


Badge::Badge(int number, qreal angle, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    _number(number)
{
    if (angle > 0) {
        QTransform transform;
        transform.translate(24, 24);
        transform.rotate(angle);
        setTransform(transform);
    }
}


QRectF Badge::boundingRect() const
{
    return QRectF(0, 0, 24, 24);
}


void Badge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (_number > 0) {
        painter->setBrush(Qt::red);
        painter->drawEllipse(boundingRect());
        painter->setPen(Qt::white);
        painter->setFont(QFont("IPAPGothic", (_number > 9) ? 14 : 16));
        QRectF rect = (_number > 9) ? QRectF(-1, -2, 26, 26) : QRectF(-1, -3, 26, 26);
        painter->drawText(rect, Qt::AlignCenter, QString::number(_number));
    }
}


void Badge::setNumber(int number)
{
    _number = number;
}
