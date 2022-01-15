#pragma once

#include <QThread>


class EngineThread : public QThread
{
    Q_OBJECT
public:
    EngineThread(QObject *parent) : QThread(parent) {}
    virtual ~EngineThread() {}

    void run() override;
};
