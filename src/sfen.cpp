#include "sfen.h"
#include "shogirecord.h"
#include <QMap>
#include <QRegularExpression>
#include <QDebug>

static const QByteArray DefaultSfen("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
static const QByteArrayList sentePieces = {"K", "R", "B", "G", "S", "N", "L", "P", "+R", "+B", "+S", "+N", "+L", "+P"};
static const QByteArrayList gotePieces = {"k", "r", "b", "g", "s", "n", "l", "p", "+r", "+b", "+s", "+n", "+l", "+p"};

class KanjiNameMap : public QMap<QString, QString> {
public:
    KanjiNameMap() :
        QMap<QString, QString>()
    {
        insert(QLatin1String("k"),  QString::fromUtf8("玉"));
        insert(QLatin1String("r"),  QString::fromUtf8("飛"));
        insert(QLatin1String("b"),  QString::fromUtf8("角"));
        insert(QLatin1String("g"),  QString::fromUtf8("金"));
        insert(QLatin1String("s"),  QString::fromUtf8("銀"));
        insert(QLatin1String("n"),  QString::fromUtf8("桂"));
        insert(QLatin1String("l"),  QString::fromUtf8("香"));
        insert(QLatin1String("p"),  QString::fromUtf8("歩"));
        insert(QLatin1String("+r"), QString::fromUtf8("龍"));
        insert(QLatin1String("+b"), QString::fromUtf8("馬"));
        insert(QLatin1String("+s"), QString::fromUtf8("成銀"));
        insert(QLatin1String("+n"), QString::fromUtf8("成桂"));
        insert(QLatin1String("+l"), QString::fromUtf8("成香"));
        insert(QLatin1String("+p"), QString::fromUtf8("と"));
    }
};
Q_GLOBAL_STATIC(KanjiNameMap, kanjiNameMap)


Sfen::Sfen()
{
    parse(DefaultSfen);
}


Sfen::Sfen(const QByteArray &sfen)
{
    parse(sfen);
}


bool Sfen::parse(const QByteArray &s)
{
    if (s.isEmpty()) {
        return false;
    }

    _position.clear();
    _inHand.clear();

    QByteArrayList items = s.split(' ');
    QByteArray sfen = items.first();
    sfen.reserve(sfen.length() + 81);
    for (int i = 1; i < 10; i++) {
        sfen.replace(QByteArray::number(i), QByteArray(i, ' '));  // 数字をスペースに
    }
    const auto sfenList = sfen.split('/');

    // 盤上
    QByteArray name;
    name.reserve(4);
    int row = 0;
    for (auto &line : sfenList) {
        row++;
        if (row > 9) {
            qCritical() << "Rows exceeded";
            goto error;
        }

        int col = 10;
        for (auto it = line.begin(); it != line.end(); ++it) {
            col--;
            if (col < 1) {
                qCritical() << "Columns exceeded";
                goto error;
            }

            if (*it == ' ') {
                continue;
            }

            name.resize(1);
            name[0] = *it;
            if (*it == '+') {
                name += *(++it);
            }

            int coord = col * 10 + row;
            _position[coord] = name;

            if (!sentePieces.contains(name) && !gotePieces.contains(name)) {
                qCritical() << "Invalid piece string in hand" << name;
                goto error;
            }
        }
    }

    // 手番
    if (items.value(1) == "b") {
        _turn = maru::Sente;
    } else if (items.value(1) == "w") {
        _turn = maru::Gote;
    } else {
        qCritical() << "Invalid turn-char" << items.value(1);
        goto error;
    }

    // 持ち駒
    sfen = items.value(2);
    if (sfen.isEmpty()) {
        qCritical() << "In-hand character empty";
        goto error;
    }

    // 持ち駒の数字展開
    if (sfen != "-") {
        QByteArray d;
        d.reserve(4);
        for (auto it = sfen.begin(); it != sfen.end(); ++it) {
            if (*it >= '0' && *it <= '9') {
                d.resize(1);
                d[0] = *it++;
                if (*it >= '0' && *it <= '9') {
                    d += *it++;
                }
                int num = d.toInt();
                if (num < 1 || num > 18) {
                    qCritical() << "Invalid number of in-hand" << num;
                    goto error;
                }
                _inHand += QByteArray(num, (*it));
            } else {
                _inHand += *it;
            }
        }
    }

    // 持ち駒チェック
    if (QString(_inHand).count(QRegularExpression("[^rbgsnlp]", QRegularExpression::CaseInsensitiveOption)) > 0) {
        qCritical() << "Invalid in-hand character";
        goto error;
    }

    // 手数
    sfen = items.value(3);
    if (sfen.isEmpty()) {
        qCritical() << "Move number empty";
        goto error;
    }
    _counter = sfen.toInt();

    // 指し手展開
    if (!items.value(4).isEmpty() && items.value(4) != "moves") {
        qCritical() << "No 'moves' string";
        goto error;
    }

    for (const auto &mv : items.mid(5)) {
        if (mv.isEmpty()) {
            qCritical() << "Move-string empty";
            goto error;
        }
        auto p = move(mv);
        if (p.isEmpty()) {
            qCritical() << "Invalid move" << mv;
            goto error;
        }
    }
    return true;

error:
    _position.clear();
    _inHand.clear();
    _counter = 1;
    qCritical() << "SFEN parse error";
    return false;
}


QByteArray Sfen::name(int coord) const
{
    return _position.value(coord);
}


QString Sfen::kanjiName(int coord) const
{
    QString s = name(coord);
    return (s.isEmpty()) ? s : kanjiNameMap()->value(s);
}

// SFEN（指し手なし）
QByteArray Sfen::toSfen() const
{
    QByteArray sfen;
    sfen.reserve(80);

    // 駒の配置
    for (int i = 1; i < 10; i++) {
        int s = 0;
        for (int j = 9; j > 0; j--) {
            int crd = i + j * 10;
            auto p = _position.value(crd);
            if (p.isEmpty()) {
                s++;
            } else {
                if (s > 0) {
                    sfen += QByteArray::number(s);
                    s = 0;
                }
                sfen += p;
            }
        }

        if (s > 0) {
            sfen += QByteArray::number(s);
        }
        sfen += "/";
    }
    sfen[sfen.length() - 1] = ' ';

    // 手番
    sfen += (_turn == maru::Sente) ? "b " : "w ";

    auto counts = [](const QByteArray &str) {
        const QByteArray names = "RBGSNLPrbgsnlp";

        QByteArray ret;
        for (auto c : names) {
            int cnt = str.count(c);
            if (cnt == 0) {
                continue;
            }

            if (cnt > 1) {
                ret += QByteArray::number(cnt);
            }
            ret += str[str.indexOf(c, 0)];
        }
        return ret;
    };

    // 持ち駒
    QByteArray str = counts(_inHand);
    if (str.isEmpty()) {
        sfen += "-";  // 無し
    } else {
        sfen += str;
    }

    // 手数
    sfen += " ";
    sfen += QByteArray::number(_counter);
    return sfen;
}


QStringList Sfen::generateKif(const QByteArrayList &moves, bool compact) const
{
    QStringList kif;
    Sfen tmpsfen = *this;
    int prevCoord = -1;
    QString piece;

    for (const auto &part : moves) {
        if (part.length() < 4) {
            continue;
        }

        auto p = tmpsfen.move(part, prevCoord, compact);
        kif << p.first;
        prevCoord = p.second;
    }
    return kif;
}

//
// 駒移動
//   Return: 移動する駒(usi)
QByteArray Sfen::move(const QByteArray &usi)
{
    QByteArray piece;  // 移動する駒
    int crd = 0;
    //qDebug() << "inhand: " << _inHand;

    if (usi.isEmpty()) {
        return QByteArray();
    }

    if (std::isalpha(usi[0])) {
        // 駒打つ
        piece.resize(1);
        piece[0] = (_turn == maru::Sente) ? std::toupper(usi[0]) : std::tolower(usi[0]);
        int idx = _inHand.indexOf(piece);
        if (idx >= 0) {
            _inHand.remove(idx, 1);
        } else {
            return QByteArray();
        }
    } else {
        // 駒移動
        crd = ShogiRecord::usiToCoord(usi.mid(0, 2));
        if (!crd) {
            return QByteArray();
        }

        piece = _position.value(crd);
        if (piece.isEmpty()) {
            return piece;
        }

        // 先手の駒か後手の駒かチェック
        if (_turn == maru::Sente) {
            if (!sentePieces.contains(piece)) {
                return QByteArray();
            }
        } else {
            if (!gotePieces.contains(piece)) {
                return QByteArray();
            }
        }

        if (usi.length() > 4 && usi[4] == '+') {
            // 成り
            piece.prepend('+');
        }
        _position.take(crd);
    }

    // 移動先のマス
    crd = ShogiRecord::usiToCoord(usi.mid(2, 2));
    if (!crd) {
        return QByteArray();
    }

    QByteArray dst = _position.take(crd);  // 駒の有無
    if (!dst.isEmpty()) {
        if (dst[0] == '+' && dst.size() > 1) {
            dst[0] = dst[1];
            dst.resize(1);
        }
        _inHand += (_turn == maru::Sente) ? dst.toUpper() : dst.toLower();
    }

    _counter++;
    _position[crd] = piece;
    _turn = (_turn == maru::Sente) ? maru::Gote : maru::Sente;
    return piece;
}

//
// 駒移動
//   Return: KIF文字列, 元のマス目
QPair<QString, int> Sfen::move(const QByteArray &usi, int prevCoord, bool compact)
{
    maru::Turn turn = _turn;
    auto piece = move(usi);

    QString kif = ShogiRecord::kifString(turn, usi, piece, prevCoord, compact);
    int crd = ShogiRecord::usiToCoord(usi.mid(2, 2));
    return qMakePair(kif, crd);
}


Sfen Sfen::move(const QByteArrayList &usiList)
{
    for (auto &usi : usiList) {
        move(usi);
    }
    return *this;
}


QByteArray Sfen::csaToSfen(const QByteArray &csa)
{
    static const QMap<QByteArray, QByteArray> convert = {
        {"FU", "p"}, {"KY","l"}, {"KE", "n"}, {"GI","s"},
        {"KI", "g"}, {"KA","b"}, {"HI", "r"}, {"OU", "k"}
    };
    static const QByteArrayList promoted = {"TO","NY", "NK", "NG", "UM", "RY"};

    Sfen sfen;
    QByteArray movestr;

    for (const auto &str : csa.split('\n')) {
        auto line = str.trimmed();
        if (line.isEmpty()) {
            continue;
        }

        if (line.startsWith('+') || line.startsWith('-')) {
            // 指し手
            auto l12 = line.mid(1, 2);
            QByteArray move = ShogiRecord::coordToUsi(l12.toInt());
            QByteArray koma = line.mid(5, 2);

            if (move.isEmpty() && l12 == "00") {
                // 打つ
                auto p = convert.value(koma).toUpper();
                move += p;
                move += '*';
            }

            QByteArray mv = ShogiRecord::coordToUsi(line.mid(3, 2).toInt());
            if (mv.isEmpty()) {
                continue;
            }

            move += mv;

            if (promoted.contains(koma)) {
                // 成りチェック
                auto p = sfen._position.value(l12.toInt());
                if (!p.startsWith('+')) { // 成り駒でないなら
                    move += '+';
                }
            }

            if (move.length() == 4 || move.length() == 5) {
                movestr += move;
                movestr += ' ';
                sfen.move(move);
            }
            continue;
        }
        if (line.startsWith('V')) {
            // version
            continue;
        }
        if (line.startsWith('N')) {
            // player
            continue;
        }
        if (line.startsWith('$')) {
            // informations
            continue;
        }
        if (line.startsWith('P')) {
            // position
            continue;
        }
        if (line.startsWith('%')) {
            // 特殊な指し手／終局状況
            break;
        }
        if (line.startsWith('\'')) {
            // comment line
            continue;
        }

        qWarning() << "Unknown CSA commend:" << line;
    }

    QByteArray sfenstr = DefaultSfen;
    sfenstr += " moves ";
    sfenstr += movestr.trimmed();
    return sfenstr;
}


Sfen Sfen::fromCsa(const QByteArray &csa, bool *ok)
{
    Sfen sf;
    bool res = sf.parse(csaToSfen(csa));
    if (ok) {
        *ok = res;
    }
    return sf;
}


Sfen Sfen::fromSfen(const QByteArray &sfen, bool *ok)
{
    Sfen sf;
    bool res = sf.parse(sfen);
    if (ok) {
        *ok = res;
    }
    return sf;
}
