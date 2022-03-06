#pragma once
#include "engine.h"
#include <QProcess>


class EngineProcess : public Engine::EngineContext, public QProcess {
public:
    EngineProcess(const QString &program, QObject *parent = nullptr);
    virtual ~EngineProcess() {}

    void start() override;
    void terminate() override;
};
