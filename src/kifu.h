#pragma once

#include <QString>
#include <QList>
#include <QDateTime>


class Kifu {
public:
    QString sfen;    // SFEN形式
    QString sente;   // 先手 名前
    QString gote;    // 後手 名前
    QString user;    // "s" or "g"
    QString winner;  // "s", "g" or ""
    QString detail;  // 内容
    int rating {0};  // 新レーティング値
    QDateTime ctime {QDateTime::currentDateTime()};
    int v {1};

    static QList<Kifu> load(int maxCount = 0);
    static bool saveAppend(const Kifu &kifu);

    Kifu() {}
    ~Kifu() {}
    Kifu(const Kifu &other) = default;
    Kifu &operator=(const Kifu &other) = default;
};
