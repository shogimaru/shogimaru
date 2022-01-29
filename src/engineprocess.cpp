#include "engineprocess.h"
#include <QDebug>

constexpr auto ENGINE_PATH = "./engines/YaneuraOu/YaneuraOu-by-gcc";


EngineProcess::EngineProcess() :
    QProcess()
{
    setProgram(ENGINE_PATH);
}


void EngineProcess::start(QIODevice::OpenMode mode)
{
    if (state() == QProcess::NotRunning) {
        QProcess::start(mode);
        waitForStarted();
    }
}


void EngineProcess::terminate()
{
    QProcess::terminate();

    bool res = waitForFinished(500);
    if (!res) {
        QProcess::kill();
        waitForFinished(500);
    }
}


EngineProcess *EngineProcess::instance()
{
    static EngineProcess engineProcess;
    return &engineProcess;
}
