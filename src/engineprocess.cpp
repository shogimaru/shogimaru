#include "engineprocess.h"
#include "enginesettings.h"
#include <QDebug>
#include <QFileInfo>


EngineProcess::EngineProcess() :
    QProcess()
{
}


void EngineProcess::start(QIODevice::OpenMode mode)
{
    auto data = EngineSettings::instance().currentEngine();
    if (data.path.isEmpty()) {
        qCritical() << "No shogi engine";
        return;
    }

    if (QFileInfo(data.path).exists()) {
        setProgram(data.path);
    } else {
        qCritical() << "Not found such shogi engine:" << data.path;
        return;
    }

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
