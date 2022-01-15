#pragma once
#include <QGraphicsObject>


class Badge : public QGraphicsObject {
    Q_OBJECT
public:
    Badge(int number = 0, qreal angle = 0, QGraphicsItem *parent = nullptr);

    void setNumber(int number);
    int number() const { return _number; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    int _number;
};
