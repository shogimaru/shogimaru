#include <QProxyStyle>

class QStyleOption;


class WesternTabStyle : public QProxyStyle {
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
        const QSize &size, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
        const QWidget *widget) const override;
};
