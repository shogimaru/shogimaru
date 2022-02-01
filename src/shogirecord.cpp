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
        insert(QLatin1String("K"),  QCoreApplication::translate("KanjiNameMap", "K"));  // 王
        insert(QLatin1String("R"),  QCoreApplication::translate("KanjiNameMap", "R"));  // 飛
        insert(QLatin1String("B"),  QCoreApplication::translate("KanjiNameMap", "B"));  // 角
        insert(QLatin1String("G"),  QCoreApplication::translate("KanjiNameMap", "G"));  // 金
        insert(QLatin1String("S"),  QCoreApplication::translate("KanjiNameMap", "S"));  // 銀
        insert(QLatin1String("N"),  QCoreApplication::translate("KanjiNameMap", "N"));  // 桂
        insert(QLatin1String("L"),  QCoreApplication::translate("KanjiNameMap", "L"));  // 香
        insert(QLatin1String("P"),  QCoreApplication::translate("KanjiNameMap", "P"));  // 歩
        insert(QLatin1String("+R"), QCoreApplication::translate("KanjiNameMap", "+R"));  // 龍
        insert(QLatin1String("+B"), QCoreApplication::translate("KanjiNameMap", "+B"));  // 馬
        insert(QLatin1String("+S"), QCoreApplication::translate("KanjiNameMap", "+S"));  // 成銀
        insert(QLatin1String("+N"), QCoreApplication::translate("KanjiNameMap", "+N"));  // 成桂
        insert(QLatin1String("+L"), QCoreApplication::translate("KanjiNameMap", "+L"));  // 成香
        insert(QLatin1String("+P"), QCoreApplication::translate("KanjiNameMap", "+P"));  // と
    }
};
Q_GLOBAL_STATIC(KanjiNameMap, kanjiNameMap)


QString ShogiRecord::kanjiName(const QString &piece)
{
    return kanjiNameMap()->value(piece.toUpper());
}

//
// 棋譜文字列
//
QString ShogiRecord::kifString(maru::Turn turn, const QByteArray &usi, const QByteArray &piece, int prevCoord, bool compact)
{
    static const bool langJa = QLocale::system().name().toLower().startsWith("ja");

    QString kifFormat = QObject::tr("%1%2%3%4%5");
    // 先手／後手
    QString bw = (turn == maru::Sente) ? QString::fromUtf8(u8"▲") : QString::fromUtf8(u8"△");

    // マス
    QString coord;
    int crd = ShogiRecord::usiToCoord(usi.mid(2, 2));
    if (crd == prevCoord) {
        if (langJa) {
            coord = (compact) ? QString::fromUtf8(u8"同") : QString::fromUtf8(u8"同  ");
        } else {
            coord  = "x";
            coord += ShogiRecord::kanji(crd);
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
