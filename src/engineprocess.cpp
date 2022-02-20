#include "engineprocess.h"
#include <QFileInfo>
#include <QDebug>

constexpr auto ENGINE_PATH = "./engines/YaneuraOu/YaneuraOu-by-gcc";
//constexpr auto ENGINE_PATH = "engines/YaneuraOu/YaneuraOu_NNUE.exe";


EngineProcess::EngineProcess() :
    QProcess()
{
    if (QFileInfo(ENGINE_PATH).exists()) {
        setProgram(ENGINE_PATH);
    } else {
        qCritical() << "Not found such shogi engine:" << ENGINE_PATH;
    }
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
