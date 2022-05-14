#include "user.h"
#include "file.h"
#include "global.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

const QLatin1String jsonName("user.json");
const QLatin1String Version("version");
const QLatin1String Nickname("nickname");
const QLatin1String Rating("rating");
const QLatin1String Wins("wins");
const QLatin1String Losses("losses");
const QLatin1String Draws("draws");
const QLatin1String Illegal("illegal");
const QLatin1String Scale("scale");
const QLatin1String Byoyomi("byoyomi");
const QLatin1String BasicTime("basicTime");
const QLatin1String AnalysisTimeSeconds("analysisTimeSeconds");
const QLatin1String AnalysisNodes("analysisNodes");
const QLatin1String AnalysisDepth("analysisDepth");
const QLatin1String SoundEnable("soundEnable");
const QLatin1String PieceType("pieceType");


static QString jsonPath()
{
#ifdef Q_OS_WASM
    return jsonName;
#else
    return QDir(maru::appLocalDataLocation()).absoluteFilePath(jsonName);
#endif
}


User::User()
{
    if (!maru::isLocaleLangJapanese()) {
        _pieceType = 2;  // 英語圏のデフォルト駒：一字彫駒
    }
}


bool User::save()
{
    QJsonObject json;
    json[Version] = version();
    json[Nickname] = nickname();
    json[Rating] = rating();
    json[Wins] = wins();
    json[Losses] = losses();
    json[Draws] = draws();
    json[Illegal] = illegal();
    json[Scale] = scale();
    json[Byoyomi] = byoyomi();
    json[BasicTime] = basicTime();
    json[AnalysisTimeSeconds] = analysisTimeSeconds();
    json[AnalysisNodes] = analysisNodes();
    json[AnalysisDepth] = analysisDepth();
    json[SoundEnable] = soundEnable();
    json[PieceType] = pieceType();

    File file(jsonPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    int len = file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    file.close();
    return len > 0;
}


User &User::load()
{
    static User user;

    if (!user.isEmpty()) {
        return user;
    }

    File file(jsonPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return user;
    }

    QByteArray ba = file.readAll();
    file.close();
    if (ba.isEmpty()) {
        return user;
    }

    auto json = QJsonDocument::fromJson(ba).object();

    if (json.contains(Version)) {
        user._version = json[Version].toInt();
    }

    if (json.contains(Nickname)) {
        user._nickname = json[Nickname].toString();
    }

    if (json.contains(Rating)) {
        user._rating = json[Rating].toInt();
    }

    if (json.contains(Wins)) {
        user._wins = json[Wins].toInt();
    }

    if (json.contains(Losses)) {
        user._losses = json[Losses].toInt();
    }

    if (json.contains(Draws)) {
        user._draws = json[Draws].toInt();
    }

    if (json.contains(Illegal)) {
        user._illegal = json[Illegal].toInt();
    }

    if (json.contains(Scale)) {
        user._scale = json[Scale].toInt();
    }

    if (json.contains(Byoyomi)) {
        user._byoyomi = json[Byoyomi].toInt();
    }

    if (json.contains(BasicTime)) {
        user._basicTime = json[BasicTime].toInt();
    }

    if (json.contains(AnalysisTimeSeconds)) {
        user._analysisTimeSeconds = json[AnalysisTimeSeconds].toInt();
    }

    if (json.contains(AnalysisNodes)) {
#if QT_VERSION < 0x060000
        user._analysisNodes = json[AnalysisNodes].toVariant().toLongLong();
#else
        user._analysisNodes = json[AnalysisNodes].toInteger();
#endif
    }

    if (json.contains(AnalysisDepth)) {
        user._analysisDepth = json[AnalysisDepth].toInt();
    }

    if (json.contains(SoundEnable)) {
        user._soundEnable = json[SoundEnable].toBool();
    }

    if (json.contains(PieceType)) {
        user._pieceType = json[PieceType].toInt();
    }

    return user;
}
