#include "command.h"
#include "engine.h"
#include "engineprocess.h"
#include <QCoreApplication>
#include <QDebug>


Engine::Engine(QObject *parent) :
    QObject(parent),
    _timer(new QTimer(this))
{
    connect(_timer, &QTimer::timeout, this, &Engine::getResponse);
}


void Engine::openUsi(const QString &path)
{
    delete _engineContext;
    auto process = new EngineProcess(path, this);
    Command::setEngine(process);
    process->start();
    _engineContext = process;
}


void Engine::closeUsi()
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
