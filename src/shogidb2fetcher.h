#pragma once
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>


struct GameItem {
    QString title;
    QString url;
    QString published;
    QString id;
};


class ShogiDB2Fetcher : public QObject {
public:
    explicit ShogiDB2Fetcher(QObject *parent = nullptr) :
        QObject(parent)
    { }

    QList<GameItem> fetchRss();
    QString fetchCsa(const QString &url, int maxNext = 400);
    QByteArray getHttpReply(const QString &url, int timeout = 5000);

private:
    QList<GameItem> parseRss(const QByteArray &xml);

    QNetworkAccessManager _manager;
    //int _errorCount {0};
};
