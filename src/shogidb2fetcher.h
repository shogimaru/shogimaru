#pragma once
#include <QObject>
#include <QString>


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
    QString fetch(const QString &pageUrl, int maxNext = 400);
};
