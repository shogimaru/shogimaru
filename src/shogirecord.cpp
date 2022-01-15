#include "shogirecord.h"
#include <QMap>


int ShogiRecord::usiToCoord(const QByteArray &str)
{
    if (str.length() != 2) {
        return 0;
    }

    int coord = str.mid(0, 1).toInt() * 10 + (str[1] - 'a' + 1);
    if (coord < 11 || coord > 99) {
        return 0;
    }
    return coord;
}


QByteArray ShogiRecord::coordToUsi(int coord)
{
    QByteArray usi;
    int d = coord / 10;
    if (d < 1 || d > 9) {
        return QByteArray();
    }

    usi += (char)('0' + d);
    d = coord % 10;
    if (!d) {
        return QByteArray();
    }
    usi += (char)(0x60 + d); // a,b,c,..
    return usi;
}


QString ShogiRecord::kanji(int coord)
{
    static QString ColKanji = QString::fromUtf8("０１２３４５６７８９");
    static QString RowKanji = QString::fromUtf8("〇一二三四五六七八九");

    QString res;
    int col = qMin(coord / 10, 9);
    int row = coord % 10;
    res += ColKanji.mid(col, 1);
    res += RowKanji.mid(row, 1);
    return res;
}


class KanjiNameMap : public QMap<QString, QString> {
public:
    KanjiNameMap() :
        QMap<QString, QString>()
    {
        insert(QLatin1String("k"),  QString::fromUtf8(u8"玉"));
        insert(QLatin1String("r"),  QString::fromUtf8(u8"飛"));
        insert(QLatin1String("b"),  QString::fromUtf8(u8"角"));
        insert(QLatin1String("g"),  QString::fromUtf8(u8"金"));
        insert(QLatin1String("s"),  QString::fromUtf8(u8"銀"));
        insert(QLatin1String("n"),  QString::fromUtf8(u8"桂"));
        insert(QLatin1String("l"),  QString::fromUtf8(u8"香"));
        insert(QLatin1String("p"),  QString::fromUtf8(u8"歩"));
        insert(QLatin1String("+r"), QString::fromUtf8(u8"龍"));
        insert(QLatin1String("+b"), QString::fromUtf8(u8"馬"));
        insert(QLatin1String("+s"), QString::fromUtf8(u8"成銀"));
        insert(QLatin1String("+n"), QString::fromUtf8(u8"成桂"));
        insert(QLatin1String("+l"), QString::fromUtf8(u8"成香"));
        insert(QLatin1String("+p"), QString::fromUtf8(u8"と"));
    }
};
Q_GLOBAL_STATIC(KanjiNameMap, kanjiNameMap)


QString ShogiRecord::kanjiName(const QString &piece)
{
    return kanjiNameMap()->value(piece.toLower());
}
