#include "sfen.h"
#include "shogirecord.h"
#include <QDebug>
#include <QMap>
#include <QRegularExpression>

static const QByteArray DefaultSfen("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
static const QByteArrayList sentePieces = {"K", "R", "B", "G", "S", "N", "L", "P", "+R", "+B", "+S", "+N", "+L", "+P"};
static const QByteArrayList gotePieces = {"k", "r", "b", "g", "s", "n", "l", "p", "+r", "+b", "+s", "+n", "+l", "+p"};


Sfen::Sfen(const QByteArray &sfen)
{
    parse(sfen);
}


void Sfen::clear()
{
    _position.clear();
    _inHand.clear();
    _turn = maru::Sente;
    _moves.clear();
    _counter = 1;
    _players.first.clear();
    _players.second.clear();
    _eventName.clear();
    _gameResult = 0;
}


bool Sfen::parse(const QByteArray &s)
{
    if (s.isEmpty()) {
        return false;
    }

    clear();

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

        if (mv.startsWith("rep_")) {
            // 千日手
            break;
        }

        auto p = move(mv);
        if (p.isEmpty()) {
            qWarning() << "Invalid move" << mv;
            goto error;
        }
    }
    return true;

error:
    clear();
    qWarning() << "SFEN parse error";
    return false;
}


bool Sfen::isEmpty() const
{
    return _position.isEmpty() && _inHand.isEmpty();
}


QByteArray Sfen::name(int coord) const
{
    return _position.value(coord);
}


QString Sfen::kanjiName(int coord) const
{
    //static KanjiNameMap kanjiNameMap;
    QString s = name(coord);
    return (s.isEmpty()) ? s : ShogiRecord::kanjiName(s);
}

// SFEN（指し手なし）
QByteArray Sfen::toSfen() const
{
    QByteArray sfen;
    sfen.reserve(80);

    if (isEmpty()) {
        return sfen;
    }

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


QByteArray Sfen::toUsi() const  // 指し手あり
{
    QByteArray usi = defaultPostion();

    if (!_moves.isEmpty()) {
        usi += " moves ";
        for (auto &mv : _moves) {
            usi += mv.second;
            usi += " ";
        }
        usi.chop(1);
    }
    return usi;
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

    _moves << qMakePair(piece, usi);
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


void Sfen::setPlayers(const QString &sente, const QString &gote)
{
    _players.first = sente;
    _players.second = gote;
}


QByteArray Sfen::defaultPostion()
{
    return DefaultSfen;
}


Sfen Sfen::fromCsa(const QString &csa, bool *ok)
{
    static const QMap<QString, QString> convert = {
        {"FU", "p"}, {"KY", "l"}, {"KE", "n"}, {"GI", "s"},
        {"KI", "g"}, {"KA", "b"}, {"HI", "r"}, {"OU", "k"}};
    static const QStringList promoted = {"TO", "NY", "NK", "NG", "UM", "RY"};

    Sfen sfen(DefaultSfen);  // TODO 駒落ちに未対応
    QString senteName;
    QString goteName;
    QString event;
    maru::Turn turn = maru::Sente;

    if (ok) {
        *ok = false;
    }

    for (const auto &str : csa.split('\n')) {
        auto line = str.trimmed();
        if (line.isEmpty()) {
            continue;
        }

        if (line.startsWith('+') || line.startsWith('-')) {
            if (line == "+") {
                turn = maru::Sente;  // 先手番
                continue;
            }
            if (line == "-") {
                turn = maru::Gote;  // 後手番
                continue;
            }

            // 指し手
            auto l12 = line.mid(1, 2);
            QByteArray move = ShogiRecord::coordToUsi(l12.toInt());
            QString koma = line.mid(5, 2);

            if (move.isEmpty() && l12 == "00") {
                // 打つ
                auto p = convert.value(koma).toUpper();
                move += p.toLatin1();
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
                if (!p.startsWith('+')) {  // 成り駒でないなら
                    move += '+';
                }
            }

            if (move.length() == 4 || move.length() == 5) {
                sfen.move(move);
                turn = (line.startsWith('+')) ? maru::Gote : maru::Sente;  // 次の手番
            } else {
                // Error
                qCritical() << "Error notation:" << move;
                return sfen;
            }
            continue;
        }
        if (line.startsWith('V')) {
            // version
            continue;
        }
        if (line.startsWith('N')) {
            // player
            if (line.startsWith("N+")) {
                senteName = line.mid(2);
            } else if (line.startsWith("N-")) {
                goteName = line.mid(2);
            } else {
                //
            }
            continue;
        }
        if (line.startsWith('$')) {
            // 棋戦名
            if (line.startsWith("$EVENT:")) {
                event = line.mid(7).trimmed();
            }
            continue;
        }
        if (line.startsWith('P')) {
            // position
            continue;
        }
        if (line.startsWith('%')) {
            // 特殊な指し手／終局状況
            // %TORYO 投了
            // %CHUDAN 中断
            // %SENNICHITE 千日手
            // %TIME_UP 手番側が時間切れで負け
            // %ILLEGAL_MOVE 手番側の反則負け
            // %+ILLEGAL_ACTION 先手(下手)の反則行為により、後手(上手)の勝ち
            // %-ILLEGAL_ACTION 後手(上手)の反則行為により、先手(下手)の勝ち
            // %JISHOGI 持将棋
            // %KACHI (入玉で)勝ちの宣言
            // %HIKIWAKE (入玉で)引き分けの宣言
            // %MATTA 待った
            // %TSUMI 詰み
            // %FUZUMI 不詰
            // %ERROR エラー
            maru::GameResult result;
            maru::ResultDetail detail;

            if (line == "%TORYO" || line == "%TSUMI") {
                result = maru::Loss;
                detail = maru::Loss_Resign;
            } else if (line == "%CHUDAN") {
                result = maru::Abort;
                detail = maru::Abort_GameAborted;
            } else if (line == "%SENNICHITE") {
                result = maru::Draw;
                detail = maru::Draw_Repetition;
            } else if (line == "%TIME_UP") {
                result = maru::Illegal;
                detail = maru::Illegal_OutOfTime;
            } else if (line == "%ILLEGAL_MOVE") {
                result = maru::Illegal;
                detail = maru::Illegal_Other;
            } else if (line == "%+ILLEGAL_ACTION") {
                turn = maru::Sente;
                result = maru::Illegal;
                detail = maru::Illegal_Other;
            } else if (line == "%-ILLEGAL_ACTION") {
                turn = maru::Gote;
                result = maru::Illegal;
                detail = maru::Illegal_Other;
            } else if (line == "%JISHOGI" || line == "%HIKIWAKE") {
                result = maru::Draw;
                detail = maru::Draw_Impasse;
            } else if (line == "%KACHI") {
                result = maru::Win;
                detail = maru::Win_Declare;
            } else if (line == "%MATTA") {
                result = maru::Illegal;
                detail = maru::Illegal_Other;
            } else if (line == "%ERROR" || line == "%FUZUMI") {
                result = maru::Abort;
                detail = maru::Abort_GameAborted;
            } else {
                result = maru::Abort;
                detail = maru::Abort_GameAborted;
            }

            sfen.setGameResult(turn, result, detail);
            break;
        }
        if (line.startsWith('T')) {
            // 消費時間
            continue;
        }
        if (line.startsWith('\'')) {
            // comment line
            continue;
        }

        qWarning() << "Unknown CSA commend:" << line;
        return sfen;
    }

    // 対局者
    sfen.setPlayers(senteName, goteName);
    // 棋戦名
    sfen.setEventName(event);

    if (ok) {
        *ok = true;
    }
    return sfen;
}


QString Sfen::toCsa() const
{
    static const QMap<QString, QString> convert = {
        {"p", "FU"}, {"l", "KY"}, {"n", "KE"}, {"s", "GI"},
        {"g", "KI"}, {"b", "KA"}, {"r", "HI"}, {"k", "OU"},
        {"+p", "TO"}, {"+l", "NY"}, {"+n", "NK"}, {"+s", "NG"},
        {"+b", "UM"}, {"+r", "RY"}};

    QString csa;

    csa.reserve(1024);
    csa += "V2.2\n";
    csa += "N+";
    csa += _players.first;
    csa += '\n';
    csa += "N-";
    csa += _players.second;
    csa += '\n';
    csa += "$EVENT:";
    csa += _eventName;
    csa += '\n';
    // 開始局面
    csa += "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n";
    csa += "P2 * -HI *  *  *  *  * -KA * \n";
    csa += "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n";
    csa += "P4 *  *  *  *  *  *  *  *  * \n";
    csa += "P5 *  *  *  *  *  *  *  *  * \n";
    csa += "P6 *  *  *  *  *  *  *  *  * \n";
    csa += "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n";
    csa += "P8 * +KA *  *  *  *  * +HI * \n";
    csa += "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n";
    csa += "+\n";  // 手番表記
    bool senteTurn = true;
    for (auto &mv : _moves) {
        csa += (senteTurn) ? "+" : "-";  // 指し手手番
        senteTurn = !senteTurn;
        if (mv.second.isEmpty()) {
            continue;
        }
        const auto &c = mv.second[0];
        if (c >= 'A' && c <= 'Z') {
            csa += "00";
        } else {
            csa += QString::number(ShogiRecord::usiToCoord(mv.second.mid(0, 2)));  // 移動前
        }
        csa += QString::number(ShogiRecord::usiToCoord(mv.second.mid(2, 2)));  // 移動後

        // 駒名
        csa += convert.value(mv.first.toLower());  // 移動後の駒
        csa += "\n";
    }
    // 終局
    csa += gemeResultCsa();
    csa += "\n";
    return csa;
}


Sfen Sfen::fromSfen(const QByteArray &sfen, bool *ok)
{
    Sfen sf;

    auto sfstr = sfen.split(' ');
    if (sfstr.value(0).toLower() == "position") {
        sfstr.takeFirst();
    }

    if (sfstr.value(0).toLower() == "startpos") {
        sfstr[0] = Sfen::defaultPostion();
    }

    bool res = sf.parse(sfstr.join(' '));
    if (ok) {
        *ok = res;
    }
    return sf;
}


QPair<maru::GameResult, maru::ResultDetail> Sfen::gameResult() const
{
    int res = _gameResult & maru::ResultMask;
    int detail = _gameResult & maru::DetailMask;
    return qMakePair((maru::GameResult)res, (maru::ResultDetail)detail);
}


void Sfen::setGameResult(maru::GameResult result, maru::ResultDetail detail)
{
    _gameResult = _turn | result | detail;
}


void Sfen::setGameResult(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail)
{
    _gameResult = turn | result | detail;
}


QString Sfen::gemeResultCsa() const
{
    /*
    %TORYO 投了
    %CHUDAN 中断
    %SENNICHITE 千日手
    %TIME_UP 手番側が時間切れで負け
    %ILLEGAL_MOVE 手番側の反則負け、反則の内容はコメントで記録する
    %+ILLEGAL_ACTION 先手(下手)の反則行為により、後手(上手)の勝ち
    %-ILLEGAL_ACTION 後手(上手)の反則行為により、先手(下手)の勝ち
    %JISHOGI 持将棋
    %KACHI (入玉で)勝ちの宣言
    */
    int turn = _gameResult & maru::TurnMask;
    int detail = _gameResult & maru::DetailMask;

    switch (detail) {
    case maru::Win_Declare:
        return QLatin1String("%KACHI");
    case maru::Loss_Resign:
        return QLatin1String("%TORYO");
    case maru::Draw_Repetition:
        return QLatin1String("%SENNICHITE");
    case maru::Draw_Impasse:
        return QLatin1String("%JISHOGI");
    case maru::Illegal_OutOfTime:
        return QLatin1String("%TIME_UP");
    case maru::Illegal_TwoPawns:  // 二歩
    case maru::Illegal_DropPawnMate:  // 打ち歩詰め
    case maru::Illegal_OverlookedCheck:  // 王手放置
    case maru::Illegal_PerpetualCheck:  // 連続王手の千日手
        return (turn == maru::Sente) ? QLatin1String("+ILLEGAL_ACTION") : QLatin1String("%-ILLEGAL_ACTION");
    case maru::Abort_GameAborted:
        return QLatin1String("%CHUDAN");
    default:
        return QLatin1String("%CHUDAN");
    }
}
