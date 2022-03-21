#include "shogirecord.h"
#include <QMap>
#include <QCoreApplication>
#include <QLocale>


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
    static QString ColChars = QCoreApplication::translate("ShogiRecord", "0123456789", "ColChars");  // ０１２３４５６７８９
    static QString RowChars = QCoreApplication::translate("ShogiRecord", "0123456789", "RowChars");  // 〇一二三四五六七八九

    QString res;
    int col = qMin(coord / 10, 9);
    int row = coord % 10;
    res += ColChars.mid(col, 1);
    res += RowChars.mid(row, 1);
    return res;
}


class KanjiNameMap : public QMap<QString, QString> {
public:
    KanjiNameMap() :
        QMap<QString, QString>()
    {
        insert(QLatin1String("K"),  QObject::tr("K"));  // 王
        insert(QLatin1String("R"),  QObject::tr("R"));  // 飛
        insert(QLatin1String("B"),  QObject::tr("B"));  // 角
        insert(QLatin1String("G"),  QObject::tr("G"));  // 金
        insert(QLatin1String("S"),  QObject::tr("S"));  // 銀
        insert(QLatin1String("N"),  QObject::tr("N"));  // 桂
        insert(QLatin1String("L"),  QObject::tr("L"));  // 香
        insert(QLatin1String("P"),  QObject::tr("P"));  // 歩
        insert(QLatin1String("+R"), QObject::tr("+R"));  // 龍
        insert(QLatin1String("+B"), QObject::tr("+B"));  // 馬
        insert(QLatin1String("+S"), QObject::tr("+S"));  // 成銀
        insert(QLatin1String("+N"), QObject::tr("+N"));  // 成桂
        insert(QLatin1String("+L"), QObject::tr("+L"));  // 成香
        insert(QLatin1String("+P"), QObject::tr("+P"));  // と
    }
};


QString ShogiRecord::kanjiName(const QString &piece)
{
    static KanjiNameMap kanjiNameMap;
    return kanjiNameMap.value(piece.toUpper());
}

//
// 棋譜文字列
//
QString ShogiRecord::kifString(maru::Turn turn, const QByteArray &usi, const QByteArray &piece, int prevCoord, bool compact)
{
    QString kifFormat = QObject::tr("%1%2%3%4%5");
    // 先手／後手
    QString bw = (turn == maru::Sente) ? QString::fromUtf8(u8"▲") : QString::fromUtf8(u8"△");

    // マス
    QString coord;
    int crd = ShogiRecord::usiToCoord(usi.mid(2, 2));
    if (crd == prevCoord) {
        if (maru::isLocaleLangJapanese()) {
            coord = (compact) ? QString::fromUtf8(u8"同") : QString::fromUtf8(u8"同  ");
        } else {
            coord  = "x";
            coord += QString::number(crd);
        }
    } else {
        coord = ShogiRecord::kanji(crd);
    }

    QString pc;  // 駒名称
    QString prm;  // 成
    if (usi.length() == 5 && usi[4] == QLatin1Char('+')) {
        // 成り
        pc = ShogiRecord::kanjiName(piece.mid(1, 1));  // 元の駒
        prm = QObject::tr("+");
    } else {
        pc = kanjiName(piece);
    }

    // 打つ
    QString drp;
    if (std::isalpha(usi[0])) {
        drp = QObject::tr("*");
    }

    QString kif = QObject::tr("%1%2%3%4%5", "char order").arg(bw, pc, drp, coord, prm);  // 英語の場合
    return kif;
}
