#include "engineprocess.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>


EngineProcess::EngineProcess(const QString &program, QObject *parent) :
    QProcess(parent)
{
    setProgram(program);
    setWorkingDirectory(QFileInfo(program).dir().absolutePath());
}


void EngineProcess::start()
{
    // auto data = EngineSettings::instance().currentEngine();
    // if (data.path.isEmpty()) {
    //     qCritical() << "No shogi engine";
    //     return;
    // }

    // if (QFileInfo(data.path).exists()) {
    //     setProgram(data.path);
    // } else {
    //     qCritical() << "Not found such shogi engine:" << data.path;
    //     return;
    // }

    if (state() == QProcess::NotRunning) {
        QProcess::start(QIODevice::ReadWrite);
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


// EngineProcess *EngineProcess::instance()
// {
//     static EngineProcess engineProcess;
//     return &engineProcess;
// }
