#include "kifu.h"
#include "file.h"
#include "global.h"
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

const QLatin1String jsonName("kifu.dat");
const QLatin1String Sfen("sfen");
const QLatin1String Sente("sente");
const QLatin1String Gote("gote");
const QLatin1String User("user");
const QLatin1String Winner("winner");
const QLatin1String Detail("detail");
const QLatin1String Rating("rating");
const QLatin1String Version("v");
const QLatin1String Ctime("ctime");


static QString jsonPath()
{
#ifdef Q_OS_WASM
    return jsonName;
#else
    return QDir(maru::appLocalDataLocation()).absoluteFilePath(jsonName);
#endif
}


bool Kifu::saveAppend(const Kifu &kifu)
{
    QJsonObject json;
    json[Sfen] = kifu.sfen;
    json[Sente] = kifu.sente;
    json[Gote] = kifu.gote;
    json[User] = kifu.user;
    json[Winner] = kifu.winner;
    json[Detail] = kifu.detail;
    json[Rating] = kifu.rating;
    json[Version] = kifu.v;
    json[Ctime] = kifu.ctime.toSecsSinceEpoch();

    File file(jsonPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        return false;
    }

    int len = file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    file.write("\n");
    file.close();
    return len > 0;
}


QList<Kifu> Kifu::load(int maxCount)
{
    QList<Kifu> kifuList;

    File file(jsonPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return kifuList;
    }

    while (maxCount <= 0 || kifuList.count() < maxCount) {
        QByteArray ba = file.readLine();
        if (ba.trimmed().isEmpty()) {
            break;
        }

        auto json = QJsonDocument::fromJson(ba).object();
        if (json.isEmpty()) {
            continue;
        }

        Kifu kifu;
        kifu.sfen = json[Sfen].toString();
        kifu.sente = json[Sente].toString();
        kifu.gote = json[Gote].toString();
        kifu.user = json[User].toString();
        kifu.winner = json[Winner].toString();
        kifu.detail = json[Detail].toString();
        kifu.rating = json[Rating].toInt();
        kifu.v = json[Version].toInt();
        kifu.ctime = QDateTime::fromSecsSinceEpoch(json[Ctime].toVariant().toLongLong());  // 64bit
        kifuList << kifu;
    }

    return kifuList;
}
