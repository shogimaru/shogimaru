#include "shogidb2fetcher.h"
#include <QCoreApplication>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QWebSocket>
#include <QEventLoop>
#include <QTimer>
#include <QXmlStreamReader>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <stdexcept>
#include <map>

static const QString BASE_URL = QStringLiteral("https://shogidb2.com");

static const QByteArray USER_AGENT =
    "Mozilla/5.0 (X11; Linux x86_64) "
    "AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/122.0.0.0 Safari/537.36";

struct LiveViewRoot {
    QString id;
    QString session;
    QString staticValue;
    QString view;
};

struct PlayerNames {
    QString sente {QStringLiteral("SENTE")};
    QString gote {QStringLiteral("GOTE")};
};


static QString extractGameId(const QString &link)
{
    static const QRegularExpression re(R"(/games/([^/?#]+))");
    const auto m = re.match(link);
    if (!m.hasMatch()) {
        return {};
    }
    return m.captured(1);
}

static QByteArray fetchUrl(const QUrl &url, int timeout = 5000)
{
    QNetworkAccessManager manager;

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Mozilla/5.0 (compatible; shogidb2-rss-fetcher/1.0)");
    req.setRawHeader("Accept", "application/rss+xml, application/xml, text/xml, */*");

    QNetworkReply *reply = manager.get(req);

    QByteArray body;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(timeout);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
        if (reply->error() != QNetworkReply::NoError) {
            const QString err = reply->errorString();
            qCritical() << err.toStdString();
            //throw std::runtime_error(err.toStdString());
        } else {
            body = reply->readAll();
        }
    } else {
        reply->abort();
        qCritical() <<"Network timeout";
        //throw std::runtime_error("Network timeout");
    }

    reply->deleteLater();
    return body;
}

static QList<GameItem> parseRss(const QByteArray &xml)
{
    QList<GameItem> games;
    QXmlStreamReader reader(xml);

    GameItem current;
    bool inItem = false;
    QString currentElement;

    while (!reader.atEnd()) {

        if (games.count() >= 20) {
            break;
        }

        const auto token = reader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            const QStringView name = reader.name();

            if (name == u"item") {
                inItem = true;
                current = GameItem{};
                currentElement.clear();
            } else if (inItem) {
                currentElement = name.toString();
            }
        } else if (token == QXmlStreamReader::Characters) {
            if (!inItem || reader.isWhitespace()) {
                continue;
            }

            const QString value = reader.text().toString().trimmed();

            if (currentElement == "title") {
                current.title += value;
            } else if (currentElement == "link") {
                current.url += value;
            } else if (currentElement == "pubDate") {
                current.published += value;
            // } else if (currentElement == "description") {
            //     current.description += value;
            }
        } else if (token == QXmlStreamReader::EndElement) {
            const QStringView name = reader.name();

            if (name == u"item") {
                current.id = extractGameId(current.url);
                games.append(current);

                inItem = false;
                currentElement.clear();
            } else if (inItem && name.toString() == currentElement) {
                currentElement.clear();
            }
        }
    }

    if (reader.hasError()) {
        qCritical() << reader.errorString();
        //throw std::runtime_error(reader.errorString().toStdString());
    }

    return games;
}

static QString htmlUnescape(QString s)
{
    s.replace("&amp;", "&");
    s.replace("&lt;", "<");
    s.replace("&gt;", ">");
    s.replace("&quot;", "\"");
    s.replace("&#39;", "'");
    s.replace("&#x27;", "'");
    return s;
}

// static QString stripHtmlTags(QString html)
// {
//     html.replace(QRegularExpression(R"(<script[^>]*>.*?</script>)",
//                                     QRegularExpression::CaseInsensitiveOption |
//                                     QRegularExpression::DotMatchesEverythingOption),
//                  " ");

//     html.replace(QRegularExpression(R"(<style[^>]*>.*?</style>)",
//                                     QRegularExpression::CaseInsensitiveOption |
//                                     QRegularExpression::DotMatchesEverythingOption),
//                  " ");

//     html.replace(QRegularExpression(R"(<[^>]+>)"), " ");
//     html = htmlUnescape(html);
//     html.replace(QRegularExpression(R"(\s+)"), " ");
//     return html.trimmed();
// }

// static QString normalizeGameId(const QString &url)
// {
//     static const QRegularExpression re(R"(/games/([^/?#]+))");
//     const auto m = re.match(url);
//     if (!m.hasMatch()) {
//         throw std::runtime_error("URL から game_id を抽出できませんでした");
//     }
//     return m.captured(1);
// }

// static QString withSuffix(const QString &path, const QString &suffix)
// {
//     QFileInfo fi(path);
//     const QString dir = fi.absolutePath();
//     const QString base = fi.completeBaseName();
//     return dir + "/" + base + suffix;
// }

// static QString removeSuffix(const QString &path)
// {
//     QFileInfo fi(path);
//     const QString dir = fi.absolutePath();
//     const QString base = fi.completeBaseName();
//     return dir + "/" + base;
// }

static QNetworkRequest makeHtmlRequest(const QUrl &url)
{
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", USER_AGENT);
    req.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    req.setRawHeader("Accept-Language", "ja,en-US;q=0.9,en;q=0.8");
    req.setRawHeader("Referer", (BASE_URL + "/").toUtf8());
    req.setRawHeader("Cache-Control", "no-cache");
    req.setRawHeader("Pragma", "no-cache");
    return req;
}

static QString fetchHtml(QNetworkAccessManager &nam, const QString &url)
{
    QNetworkReply *reply = nam.get(makeHtmlRequest(QUrl(url)));

    QByteArray body;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&] {
        reply->abort();
        loop.quit();
    });

    timer.start(30000);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        const QString err = reply->errorString();
        qCritical() << "HTTP error: " << err;
        //throw std::runtime_error(("HTTP error: " + err).toStdString());
    } else {
        body = reply->readAll();
    }

    reply->deleteLater();

    return QString::fromUtf8(body);
}

static QString extractCsrfToken(const QString &html)
{
    const QList<QRegularExpression> patterns = {
        QRegularExpression(R"(<meta[^>]+name=["']csrf-token["'][^>]+content=["']([^"']+)["'])",
                           QRegularExpression::CaseInsensitiveOption |
                           QRegularExpression::DotMatchesEverythingOption),
        QRegularExpression(R"(csrf[_-]?token["']?\s*[:=]\s*["']([^"']+)["'])",
                           QRegularExpression::CaseInsensitiveOption |
                           QRegularExpression::DotMatchesEverythingOption),
        QRegularExpression(R"(_csrf_token=([A-Za-z0-9\-_]+))",
                           QRegularExpression::CaseInsensitiveOption)
    };

    for (const auto &re : patterns) {
        const auto m = re.match(html);
        if (m.hasMatch()) {
            return htmlUnescape(m.captured(1));
        }
    }
    return {};
}

static QStringList extractTrackStatic(const QString &html)
{
    QStringList assets;

    {
        QRegularExpression re(
            R"(https://shogidb2\.com/assets/[^"']+\.(?:css|js)\?vsn=[^"']+)",
            QRegularExpression::CaseInsensitiveOption);
        auto it = re.globalMatch(html);
        while (it.hasNext()) {
            const auto m = it.next();
            const QString x = m.captured(0);
            if (!assets.contains(x)) {
                assets.append(x);
            }
        }
    }

    {
        QRegularExpression re(
            R"(/assets/[^"']+\.(?:css|js)\?vsn=[^"']+)",
            QRegularExpression::CaseInsensitiveOption);
        auto it = re.globalMatch(html);
        while (it.hasNext()) {
            const auto m = it.next();
            const QString x = BASE_URL + m.captured(0);
            if (!assets.contains(x)) {
                assets.append(x);
            }
        }
    }

    return assets;
}

static QString attrValue(const QString &tag, const QString &attr)
{
    QRegularExpression re(
        QString(R"(%1=["']([^"']*)["'])").arg(QRegularExpression::escape(attr)),
        QRegularExpression::CaseInsensitiveOption);
    const auto m = re.match(tag);
    if (m.hasMatch()) {
        return htmlUnescape(m.captured(1));
    }
    return {};
}

static LiveViewRoot extractLiveViewRoot(const QString &html)
{
    {
        QRegularExpression re(
            R"(<[^>]+data-phx-session=["'][^"']+["'][^>]*>)",
            QRegularExpression::CaseInsensitiveOption |
            QRegularExpression::DotMatchesEverythingOption);
        const auto m = re.match(html);
        if (m.hasMatch()) {
            const QString tag = m.captured(0);
            return {
                attrValue(tag, "id"),
                attrValue(tag, "data-phx-session"),
                attrValue(tag, "data-phx-static"),
                attrValue(tag, "data-phx-view")
            };
        }
    }

    {
        QRegularExpression re(
            R"(<[^>]+data-phx-static=["'][^"']+["'][^>]*>)",
            QRegularExpression::CaseInsensitiveOption |
            QRegularExpression::DotMatchesEverythingOption);
        const auto m = re.match(html);
        if (m.hasMatch()) {
            const QString tag = m.captured(0);
            return {
                attrValue(tag, "id"),
                attrValue(tag, "data-phx-session"),
                attrValue(tag, "data-phx-static"),
                attrValue(tag, "data-phx-view")
            };
        }
    }

    QString id;
    QString session;
    QString staticValue;

    {
        QRegularExpression re(R"(id=["'](phx-[^"']+)["'])",
                              QRegularExpression::CaseInsensitiveOption);
        const auto m = re.match(html);
        if (m.hasMatch()) {
            id = htmlUnescape(m.captured(1));
        }
    }

    {
        QRegularExpression re(R"(data-phx-session=["']([^"']+)["'])",
                              QRegularExpression::CaseInsensitiveOption);
        const auto m = re.match(html);
        if (m.hasMatch()) {
            session = htmlUnescape(m.captured(1));
        }
    }

    {
        QRegularExpression re(R"(data-phx-static=["']([^"']+)["'])",
                              QRegularExpression::CaseInsensitiveOption);
        const auto m = re.match(html);
        if (m.hasMatch()) {
            staticValue = htmlUnescape(m.captured(1));
        }
    }

    if (id.isEmpty() && session.isEmpty() && staticValue.isEmpty()) {
        qCritical() << "LiveView route could not be found in the HTML";
        //throw std::runtime_error("LiveView route could not be found in the HTML");
    }

    return {id, session, staticValue, {}};
}

static QString normalizeHtmlText(QString s)
{
    s.replace(QRegularExpression(R"(<[^>]+>)"), " ");
    s = htmlUnescape(s);
    s.replace(QRegularExpression(R"(\s+)"), " ");
    return s.trimmed();
}

static QString extractTableValueByHeader(const QString &html, const QString &header)
{
    const QString pattern =
        R"(<tr[^>]*>\s*<th[^>]*>\s*)" +
        QRegularExpression::escape(header) +
        R"(\s*</th>\s*<td[^>]*>(.*?)</td>\s*</tr>)";

    QRegularExpression re(
        pattern,
        QRegularExpression::CaseInsensitiveOption |
        QRegularExpression::DotMatchesEverythingOption
    );

    const auto m = re.match(html);
    if (!m.hasMatch()) {
        return {};
    }

    return normalizeHtmlText(m.captured(1));
}

static PlayerNames extractPlayerNames(const QString &html)
{
    PlayerNames names;

    const QString sente = extractTableValueByHeader(html, QStringLiteral("先手"));
    const QString gote  = extractTableValueByHeader(html, QStringLiteral("後手"));

    if (!sente.isEmpty()) {
        names.sente = sente;
    }

    if (!gote.isEmpty()) {
        names.gote = gote;
    }

    return names;
}

static QString guessLvTopic(const LiveViewRoot &root)
{
    if (root.id.isEmpty()) {
        qCritical() << "LiveView root ID could not be identified";
        //throw std::runtime_error("LiveView root ID could not be identified");
    }
    return "lv:" + root.id;
}

static QString buildWsUrl(const QString &csrfToken, const QStringList &trackStatic, const QString &pageUrl)
{
    QUrl url("wss://shogidb2.com/live/websocket");
    QUrlQuery q;

    if (!csrfToken.isEmpty()) {
        q.addQueryItem("_csrf_token", csrfToken);
    }

    q.addQueryItem("locale", "ja-JP");

    for (int i = 0; i < trackStatic.size(); ++i) {
        q.addQueryItem(QString("_track_static[%1]").arg(i), trackStatic[i]);
    }

    q.addQueryItem("_mounts", "0");
    q.addQueryItem("_mount_attempts", "0");
    q.addQueryItem("_live_referer", pageUrl);
    q.addQueryItem("vsn", "2.0.0");

    url.setQuery(q);
    return url.toString();
}

static QJsonArray parseFrame(const QString &raw)
{
    QJsonArray arr;
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8(), &err);

    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        qCritical() << ("unexpected frame JSON: " + err.errorString());
        //throw std::runtime_error(("unexpected frame JSON: " + err.errorString()).toStdString());
        return arr;
    }

    arr = doc.array();
    if (arr.size() != 5) {
        qCritical() << "unexpected frame length";
        //throw std::runtime_error("unexpected frame length");
    }

    return arr;
}

static QString buildCookieHeader(QNetworkAccessManager &nam, const QUrl &url)
{
    auto *jar = nam.cookieJar();
    if (!jar) {
        return {};
    }

    const QList<QNetworkCookie> cookies = jar->cookiesForUrl(url);
    QStringList parts;
    for (const auto &c : cookies) {
        parts << QString::fromUtf8(c.name()) + "=" + QString::fromUtf8(c.value());
    }
    return parts.join("; ");
}

static void collectInteresting(const QJsonValue &v, QJsonObject &bucket);

static void appendToBucket(QJsonObject &bucket, const QString &key, const QJsonValue &value)
{
    QJsonArray arr = bucket.value(key).toArray();
    arr.append(value);
    bucket[key] = arr;
}

static void collectInterestingObject(const QJsonObject &obj, QJsonObject &bucket)
{
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        const QString key = it.key();
        const QString lower = key.toLower();
        const QJsonValue value = it.value();

        if (lower.contains("move")) {
            appendToBucket(bucket, "moves", value);
        }
        if (lower.contains("sfen")) {
            appendToBucket(bucket, "sfen", value);
        }
        if (lower == "game" ||
            lower == "record" ||
            lower == "kifu" ||
            lower == "rendered" ||
            lower == "diff" ||
            lower == "assigns") {
            appendToBucket(bucket, lower, value);
        }

        collectInteresting(value, bucket);
    }
}

static void collectInteresting(const QJsonValue &v, QJsonObject &bucket)
{
    if (v.isObject()) {
        collectInterestingObject(v.toObject(), bucket);
    } else if (v.isArray()) {
        const QJsonArray arr = v.toArray();
        for (const auto &x : arr) {
            collectInteresting(x, bucket);
        }
    }
}

static void extractMoveRecordsWalk(const QJsonValue &v, QJsonArray &found)
{
    if (v.isArray()) {
        const QJsonArray arr = v.toArray();

        bool candidateArray = !arr.isEmpty();
        for (const auto &item : arr) {
            if (!item.isObject()) {
                candidateArray = false;
                break;
            }
        }

        if (candidateArray) {
            bool hasMoveRecord = false;
            for (const auto &item : arr) {
                const QJsonObject obj = item.toObject();
                if (obj.contains("csa") && obj.contains("num")) {
                    hasMoveRecord = true;
                    break;
                }
            }

            if (hasMoveRecord) {
                for (const auto &item : arr) {
                    const QJsonObject obj = item.toObject();
                    if (obj.contains("csa") && obj.contains("num")) {
                        found.append(obj);
                    }
                }
            }
        }

        for (const auto &x : arr) {
            extractMoveRecordsWalk(x, found);
        }
    } else if (v.isObject()) {
        const QJsonObject obj = v.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            extractMoveRecordsWalk(it.value(), found);
        }
    }
}

static QJsonArray extractMoveRecordsFromObj(const QJsonValue &obj)
{
    QJsonArray found;
    extractMoveRecordsWalk(obj, found);
    return found;
}

static bool hasToryo(const QJsonArray &moves)
{
    for (const auto &v : moves) {
        if (!v.isObject()) {
            continue;
        }

        const QJsonObject obj = v.toObject();
        QString csa = obj.value("csa").toString().trimmed();

        if (csa.compare("%TORYO", Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}

static QJsonArray dedupeAndSortMoves(const QJsonArray &moves)
{
    std::map<int, QJsonObject> byNum;

    for (const auto &v : moves) {
        if (!v.isObject()) {
            continue;
        }

        const QJsonObject obj = v.toObject();
        if (!obj.contains("csa") || !obj.contains("num")) {
            continue;
        }

        bool ok = false;
        int num = 0;

        if (obj.value("num").isDouble()) {
            num = obj.value("num").toInt();
            ok = true;
        } else if (obj.value("num").isString()) {
            num = obj.value("num").toString().toInt(&ok);
        }

        if (!ok) {
            continue;
        }

        byNum[num] = obj;
    }

    QJsonArray result;
    for (const auto &[_, obj] : byNum) {
        result.append(obj);
    }

    return result;
}

static QString buildCsaFromMoves(const QJsonArray &moves, const PlayerNames &players)
{
    if (moves.isEmpty()) {
        qCritical() << "No moves that can be converted to CSA";
        //throw std::runtime_error("No moves that can be converted to CSA");
        return QString();
    }

    QStringList lines;
    lines << "V2.2";
    lines << "N+" + players.sente;
    lines << "N-" + players.gote;
    lines << "PI";
    lines << "+";

    for (const auto &v : moves) {
        const QJsonObject obj = v.toObject();
        const QString csa = obj.value("csa").toString().trimmed();
        if (!csa.isEmpty()) {
            lines << csa;
        }
    }

    return lines.join('\n') + "\n";
}


QList<GameItem> ShogiDB2Fetcher::fetchRss()
{
    const QString url = "https://shogidb2.com/rss";
    QByteArray xml = fetchUrl(QUrl(url));
    return parseRss(xml);
}


QString ShogiDB2Fetcher::fetch(const QString &pageUrl, int maxNext)
{
    QNetworkAccessManager nam;
    nam.setCookieJar(new QNetworkCookieJar(&nam));

    const QString htmlText = fetchHtml(nam, pageUrl);
    const PlayerNames players = extractPlayerNames(htmlText);

    const QString csrfToken = extractCsrfToken(htmlText);
    const LiveViewRoot lvRoot = extractLiveViewRoot(htmlText);

    const QStringList trackStatic = extractTrackStatic(htmlText);
    const QString topic = guessLvTopic(lvRoot);
    const QString wsUrl = buildWsUrl(csrfToken, trackStatic, pageUrl);
    const QString cookieHeader = buildCookieHeader(nam, QUrl(BASE_URL));

    QJsonArray trackStaticJson;
    for (const auto &x : trackStatic) {
        trackStaticJson.append(x);
    }

    QJsonArray cookieNames;
    for (const auto &c : nam.cookieJar()->cookiesForUrl(QUrl(BASE_URL))) {
        cookieNames.append(QString::fromUtf8(c.name()));
    }

    QJsonObject bucket;
    bucket["moves"] = QJsonArray();
    bucket["sfen"] = QJsonArray();
    bucket["game"] = QJsonArray();
    bucket["record"] = QJsonArray();
    bucket["kifu"] = QJsonArray();
    bucket["rendered"] = QJsonArray();
    bucket["diff"] = QJsonArray();
    bucket["assigns"] = QJsonArray();

    QJsonArray allMoveRecords;

    int refNo = 1;
    QString joinRef;
    QSet<QString> sentEventRefs;

    auto nextRef = [&]() -> QString {
        return QString::number(refNo++);
    };

    auto sendFrame = [&](QWebSocket &ws, const QJsonArray &frame) {
        QJsonDocument doc(frame);
        const QString text = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        ws.sendTextMessage(text);
        //appendJsonLine(wslogPath, "send", frame);
    };

    auto extendMoveRecords = [&](const QJsonValue &v) {
        const QJsonArray arr = extractMoveRecordsFromObj(v);
        for (const auto &x : arr) {
            allMoveRecords.append(x);
        }
    };

    QWebSocket ws;
    QEventLoop loop;

    bool openOk = false;
    bool finished = false;
    bool joined = false;
    QString errorMessage;
    int nextCount = 0;
    int idleLoops = 0;

    QTimer idleTimer;
    idleTimer.setInterval(8000);
    idleTimer.setSingleShot(true);

    auto restartIdleTimer = [&]() {
        idleTimer.start();
    };

    auto sendNext = [&]() {
        if (hasToryo(allMoveRecords)) {
            finished = true;
            loop.quit();
            return;
        }

        if (nextCount >= maxNext) {
            finished = true;
            loop.quit();
            return;
        }

        const QString evRef = nextRef();
        sentEventRefs.insert(evRef);

        QJsonObject value;
        QJsonObject payload;
        payload["type"] = "click";
        payload["event"] = "next";
        payload["value"] = value;

        QJsonArray frame;
        frame.append(joinRef);
        frame.append(evRef);
        frame.append(topic);
        frame.append("event");
        frame.append(payload);

        sendFrame(ws, frame);
        ++nextCount;
    };

    QObject::connect(&idleTimer, &QTimer::timeout, [&] {
        ++idleLoops;

        if (idleLoops > 3) {
            finished = true;
            loop.quit();
            return;
        }

        const QString hbRef = nextRef();

        QJsonArray hb;
        hb.append(QJsonValue::Null);
        hb.append(hbRef);
        hb.append("phoenix");
        hb.append("heartbeat");
        hb.append(QJsonObject());

        sendFrame(ws, hb);
        restartIdleTimer();
    });

    QObject::connect(&ws, &QWebSocket::connected, [&] {
        openOk = true;
        joinRef = nextRef();

        QJsonObject params;
        params["_csrf_token"] = csrfToken;
        params["_mounts"] = 0;

        QJsonArray track;
        for (const auto &x : trackStatic) {
            track.append(x);
        }
        params["_track_static"] = track;

        QJsonObject joinPayload;
        joinPayload["url"] = pageUrl;
        joinPayload["params"] = params;
        joinPayload["session"] = lvRoot.session;
        joinPayload["static"] = lvRoot.staticValue;

        QJsonArray joinFrame;
        joinFrame.append(joinRef);
        joinFrame.append(joinRef);
        joinFrame.append(topic);
        joinFrame.append("phx_join");
        joinFrame.append(joinPayload);

        sendFrame(ws, joinFrame);
        restartIdleTimer();
    });

    QObject::connect(&ws, &QWebSocket::textMessageReceived, [&](const QString &message) {
        try {
            idleLoops = 0;
            restartIdleTimer();

            const QJsonArray frame = parseFrame(message);
            //appendJsonLine(wslogPath, "recv", frame);

            const QString fRef = frame.at(1).toString();
            const QString fTopic = frame.at(2).toString();
            const QString fEvent = frame.at(3).toString();
            const QJsonValue fPayload = frame.at(4);

            collectInteresting(fPayload, bucket);
            extendMoveRecords(fPayload);

            if (hasToryo(allMoveRecords)) {
                finished = true;
                loop.quit();
                return;
            }

            if (fTopic == topic && fEvent == "phx_reply") {
                const QJsonObject payloadObj = fPayload.toObject();
                const QString status = payloadObj.value("status").toString();
                const QJsonValue response = payloadObj.value("response");

                if (fRef == joinRef) {
                    if (status != "ok") {
                        QJsonDocument doc(payloadObj);
                        qCritical() << ("join failed: " + QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
                        //throw std::runtime_error(("join failed: " + QString::fromUtf8(doc.toJson(QJsonDocument::Compact))).toStdString());
                        return;
                    }

                    joined = true;
                    //qDebug() << "DEBUG join ok";

                    extendMoveRecords(response);
                    collectInteresting(response, bucket);

                    sendNext();
                    return;
                }

                if (sentEventRefs.contains(fRef)) {
                    extendMoveRecords(response);
                    collectInteresting(response, bucket);

                    if (nextCount < maxNext) {
                        sendNext();
                    } else {
                        finished = true;
                        loop.quit();
                    }
                    return;
                }
            } else if (fTopic == "phoenix" && fEvent == "heartbeat") {
                const QString hbRef = nextRef();

                QJsonArray hb;
                hb.append(QJsonValue::Null);
                hb.append(hbRef);
                hb.append("phoenix");
                hb.append("heartbeat");
                hb.append(QJsonObject());

                sendFrame(ws, hb);
            }
        } catch (const std::exception &e) {
            errorMessage = e.what();
            loop.quit();
        }
    });

    QObject::connect(&ws, &QWebSocket::disconnected, [&] {
        if (!finished && errorMessage.isEmpty()) {
            errorMessage = "WebSocket disconnected";
        }
        loop.quit();
    });

    QObject::connect(&ws, &QWebSocket::errorOccurred, [&](QAbstractSocket::SocketError) {
        errorMessage = ws.errorString();
        loop.quit();
    });

    QNetworkRequest wsReq {QUrl(wsUrl)};
    wsReq.setRawHeader("User-Agent", USER_AGENT);
    wsReq.setRawHeader("Origin", BASE_URL.toUtf8());
    wsReq.setRawHeader("Referer", pageUrl.toUtf8());
    wsReq.setRawHeader("Cookie", cookieHeader.toUtf8());
    wsReq.setRawHeader("Cache-Control", "no-cache");
    wsReq.setRawHeader("Pragma", "no-cache");

    ws.open(wsReq);
    loop.exec();
    idleTimer.stop();
    ws.close();

    if (!errorMessage.isEmpty()) {
        qCritical() << errorMessage;
        //throw std::runtime_error(errorMessage.toStdString());
        return QString();
    }

    const QJsonArray dedupedMoves = dedupeAndSortMoves(allMoveRecords);

    QJsonArray movesWrapper;
    movesWrapper.append(dedupedMoves);
    bucket["moves"] = movesWrapper;

    if (dedupedMoves.isEmpty()) {
        qCritical() << "Could not extract 'moves'";
        //throw std::runtime_error(QString("Could not extract 'moves'").toStdString());
    }

    return buildCsaFromMoves(dedupedMoves, players);
}

