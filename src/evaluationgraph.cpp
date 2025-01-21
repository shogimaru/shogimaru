#include "evaluationgraph.h"
#include "global.h"
#include <QPainter>
#include <QPainterPath>
#include <QStringList>
#include <QPointF>
#include <QMouseEvent>
#include <QDebug>

const QMargins margin(15, 5, 5, 15);

// 0: first move
void EvaluationGraph::setCurrentMove(int move)
{
    //qDebug() << "setCurrentMove:" << move;
    if (move >= 0 && move < _scores.count()) {
        if (move != _current) {
            emit currentMoveChanged(move);
            update();
        }
        _current = move;
    }
}


void EvaluationGraph::setScores(const QList<int> scores)
{
    _scores = scores;
}


void EvaluationGraph::addScore(int score)
{
    _scores.append(score);
}


void EvaluationGraph::clear()
{
    _scores.clear();
    _points.clear();
    _current = -1;
    update();
}


void EvaluationGraph::paintEvent(QPaintEvent *)
{
    QStringList scaleTexts;
    if (_percentage) {
        scaleTexts = {"%", "75", "50", "75", ""};
    } else {
        scaleTexts = {"4k", "2k", "0", "2k", "4k"};
    }

    QPainter painter(this);
    // Background
    auto rect = parentWidget()->rect();
    auto size = parentWidget()->size() - QSize(1, 1);
    painter.setPen(Qt::lightGray);
    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, size.width(), size.height());

    // Graph Rect
    auto grphRect = rect - margin;
    //qDebug() << rect << grphRect;
    painter.setPen(QColor("#DDD"));
    painter.drawRect(grphRect);
    for (int i = 1; i < 4; i++) {
        QPoint add(0, i * grphRect.height() / 4);
        painter.drawLine(grphRect.topLeft() + add, grphRect.topRight() + add);
    }

    // Label
    painter.setFont(QFont("Helvetica", 8));
    painter.setPen(Qt::black);
    QRect textRect(1, 0, margin.left() - 2, 12);
    for (auto text : scaleTexts) {
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, text);
        //qDebug() << textRect;
        textRect.adjust(0, grphRect.height() / 4, 0, grphRect.height() / 4);
    }

    // Graph
    if (!_scores.isEmpty()) {
        _points.clear();

        painter.setPen(QPen(Qt::red, 3));
        const int max = (_percentage) ? 100 : 4000;
        const float dx = qMin(3.0f, grphRect.width() / (float)_scores.count());  // Increased difference
        const int sx = grphRect.left() + 2;  // start x-point
        int yzero = grphRect.center().y();

        QPainterPath path(QPointF(sx, yzero));
        _points.append(QPoint(sx, yzero));

        for (int i = 0; i < _scores.count(); i++) {
            int eval = (_percentage) ? (maru::percentage(_scores[i]) - 50) * 2 : _scores[i];
            int x = sx + i * dx;
            int y = qBound(grphRect.y(), yzero - eval * grphRect.height() / 2 / max, grphRect.y() + grphRect.height());
            //qDebug() << x << y;
            path.lineTo(x, y);

            _points.append(QPoint(x, y));
        }
        painter.drawPath(path);

        if (_current >= 0) {
            painter.setPen(QPen(Qt::blue, 1));
            int x = sx + _current * dx;
            painter.drawLine(x, grphRect.y(), x, grphRect.y() + grphRect.height());
        }
    }
}


void EvaluationGraph::mousePressEvent(QMouseEvent *e)
{
    auto grphRect = parentWidget()->rect() - margin;
    if (grphRect.contains(e->pos())) {
        //qDebug() << "mousePress:" << e->pos();
        for (int i = 0; i < _points.count(); i++) {
            if (_points[i].x() > e->pos().x()) {
                qDebug() << "move:" << i;
                setCurrentMove(i);
                break;
            }
        }
    }
}
