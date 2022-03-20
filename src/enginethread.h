#pragma once
#include "engine.h"
#include <QThread>


class EngineThread : public QThread, public Engine::EngineContext {
    Q_OBJECT
public:
    EngineThread(QObject *parent) :
        QThread(parent) { }
    virtual ~EngineThread() { }

    void start() override;
    void terminate() override;
    void run() override;
};
