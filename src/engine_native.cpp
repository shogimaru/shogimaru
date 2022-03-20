#include "command.h"
#include "engine.h"
#include "engineprocess.h"
#include <QCoreApplication>
#include <QDebug>


Engine::Engine(QObject *parent) :
    QObject(parent),
    _timer(new QTimer(this)),
    _errorTimer(new QTimer(this))
{
    _errorTimer->setSingleShot(true);  // シングルショット
    connect(_timer, &QTimer::timeout, this, &Engine::getResponse);
    connect(_errorTimer, &QTimer::timeout, this, &Engine::engineError);
}


bool Engine::openContext(const QString &path)
{
    delete _engineContext;
    auto process = new EngineProcess(path, this);
    process->start();

    bool ret = process->waitForStarted(5000);
    if (ret) {
        Command::setEngine(process);
        _engineContext = process;
    } else {
        delete process;
    }
    return ret;
}


void Engine::closeContext()
{
    if (_engineContext) {
        auto *process = dynamic_cast<EngineProcess *>(_engineContext);
        process->terminate();
        process->waitForFinished(5000);
        delete _engineContext;
    }
    _engineContext = nullptr;
    Command::setEngine(nullptr);
}
