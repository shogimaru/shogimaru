#include "ponderinfo.h"
#include <QDebug>


PonderInfo::PonderInfo(const QByteArray &info)
{
    parse(info);
}


void PonderInfo::parse(const QByteArray &info)
{
    // bound評価関数
    auto evalbound = [](const QByteArray &str) {
        if (str == "lowerbound") {
            return -1;
        } else if (str == "upperbound") {
            return 1;
        } else {
            return 0;
        }
    };

    //qDebug() << "raw info:" << info;

    QByteArrayList parts = info.split(' ');
    QMutableListIterator<QByteArray> i(parts);
    while (i.hasNext()) {
        if (i.next().isEmpty()) {
            i.remove();  // 空を削除
        }
    }

    auto it = parts.cbegin();
    while (it != parts.cend()) {
        if (*it == "info") {
            // do nothing
        } else if (*it == "depth") {
            depth = (*(++it)).toInt();
        } else if (*it == "seldepth") {
            seldepth = (*(++it)).toInt();
        } else if (*it == "score") {
            ++it;
            if (*it == "cp") {
                scoreCp = (*(++it)).toInt();
                bound = evalbound(*(++it));
                if (!bound) {
                    continue;
                }

            } else if (*it == "mate") {
                auto c = *(++it);
                if (c == "+") {
                    mate = 1;  // 詰みありエンジン勝ち（手数不明）
                } else if (c == "-") {
                    mate = -1;  // 詰みありエンジン負け（手数不明）
                } else {
                    mateCount = std::abs(c.toInt());
                    if (mateCount > 0) {
                        mate = c.toInt() / mateCount;
                    }
                }

                bound = evalbound(*(++it));
                if (!bound) {
                    continue;
                }

            } else {
                continue;
            }
        } else if (*it == "nodes") {
            nodes = (*(++it)).toInt();
        } else if (*it == "nps") {
            nps = (*(++it)).toInt();
        } else if (*it == "hashfull") {
            hashfull = (*(++it)).toInt();
        } else if (*it == "time") {
            time = (*(++it)).toInt();
        } else if (*it == "pv") {
            while (++it != parts.cend()) {
                if (*it == "none") {
                    continue;
                }
                if (it->startsWith("(")) {
                    continue;
                }
                pv.append(*it);
            }
            //std::cout << "pv: " << qPrintable(pv.join(" ")) << std::endl;
            break;
        } else if (*it == "multipv") {
            multipv = (*(++it)).toInt();
        } else if (*it == "currmove") {
            currmove = (*(++it));
        } else if (*it == "string") {
            while (++it != parts.cend()) {
                message += *it;
                message += ' ';
            }
            message.chop(1);
            break;
        } else {
            qDebug() << "Parse Error!!!!!!!!";
        }
        ++it;
    }
    //qDebug() << "score:" << scoreCp << "np:" << np << "depth:" << depth;
}


void PonderInfo::clear()
{
    operator=(PonderInfo());
}

/*
static QString tojp(const QString &str)
{
    int pos = 0;

    if (str.length() != 2) {
        return pos;
    }

    pos = str.mid(0, 1).toInt() * 10 + (str[1].toLatin1() - 'a' + 1);
    return pos;
}


QString PonderInfo::pvJp(const QString &position)
{
    QString pvstr;

}
*/
