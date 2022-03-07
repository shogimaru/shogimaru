#include "engine.h"
#include "enginethread.h"


Engine::Engine(QObject *parent) :
    QObject(parent),
    _timer(new QTimer(this)),
    _engineContext(new EngineThread(this))
{
    connect(_timer, &QTimer::timeout, this, &Engine::getResponse);
}


void Engine::openUsi(const QString &)
{
    auto *thread = dynamic_cast<EngineThread *>(_engineContext);

    if (!thread->isRunning()) {
        thread->start();
        while (!thread->isRunning()) {
            QThread::msleep(10);
        }
    }
}


void Engine::closeUsi()
{
    auto *thread = dynamic_cast<EngineThread *>(_engineContext);

    if (thread->isRunning()) {
        quit();
        thread->terminate();
        thread->wait();
    }
}
