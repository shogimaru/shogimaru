#include "user.h"
#include "file.h"
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

const QLatin1String jsonPath("user.json");
const QLatin1String Version("version");
const QLatin1String Nickname("nickname");
const QLatin1String Rating("rating");
const QLatin1String Wins("wins");
const QLatin1String Losses("losses");
const QLatin1String Draws("draws");
const QLatin1String Fouls("fouls");
const QLatin1String Scale("scale");
const QLatin1String Byoyomi("byoyomi");
const QLatin1String BasicTime("basicTime");
const QLatin1String AnalysisTimeSeconds("analysisTimeSeconds");
const QLatin1String AnalysisNodes("analysisNodes");
const QLatin1String AnalysisDepth("analysisDepth");

bool User::save()
{
    QJsonObject json;
    json[Version] = version();
    json[Nickname] = nickname();
    json[Rating] = rating();
    json[Wins] = wins();
    json[Losses] = losses();
    json[Draws] = draws();
    json[Fouls] = fouls();
    json[Scale] = scale();
    json[Byoyomi] = byoyomi();
    json[BasicTime] = basicTime();
    json[AnalysisTimeSeconds] = analysisTimeSeconds();
    json[AnalysisNodes] = analysisNodes();
    json[AnalysisDepth] = analysisDepth();

    File file(jsonPath);
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

    File file(jsonPath);
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

    if (json.contains(Fouls)) {
        user._fouls = json[Fouls].toInt();
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
        user._analysisNodes = json[AnalysisNodes].toInt();
    }

    if (json.contains(AnalysisDepth)) {
        user._analysisDepth = json[AnalysisDepth].toInt();
    }

    return user;
}
