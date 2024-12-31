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
