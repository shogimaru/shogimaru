#include "westerntabstyle.h"
#include <QStyleOption>
#include <QPainter>


QSize WesternTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
    const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab) {
        return QSize(110, 50);
    }
    return s;
}


void WesternTabStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
    const QWidget *widget) const
{
    if (element == CE_TabBarTabLabel) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QStyleOptionTab opt(*tab);
            const QRect &rect = opt.rect;
            painter->save();

            painter->translate(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2); // 中心へ移動
            painter->translate(-rect.width() / 2, -rect.height() / 2); // 描画位置を調整
            QRect newRect(0, 0, rect.width(), rect.height());
            painter->setPen(QColor(24, 24, 24));
            painter->drawText(newRect, Qt::AlignCenter, opt.text);  // テキストの描画

            painter->restore();  // 元の状態に戻す
        }
    } else if (element == QStyle::CE_TabBarTabShape) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QStyleOptionTab opt(*tab);
            painter->save();

            if (opt.state & QStyle::State_Selected) {
                painter->setBrush(Qt::white); // 選択タブの背景色
            } else {
                painter->setBrush(QColor(229, 229, 229)); // 非選択タブの背景色
            }
            painter->setPen(QColor(180, 180, 180));
            QRect expandedRect = opt.rect.adjusted(0, 0, 1, -1); // 調整
            painter->drawRoundedRect(expandedRect, 2, 2);

            painter->restore(); // 元の状態に戻す
        }
    } else {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}
