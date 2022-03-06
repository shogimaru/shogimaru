#include "engine.h"
#include "enginethread.h"


Engine::Engine(QObject *parent) :
    QObject(parent),
    _timer(new QTimer(this)),
    _engineContext(new EngineThread(this))
{
    connect(_timer, &QTimer::timeout, this, &Engine::getResponse);
    connect(_engineContext, &QThread::finished, _engineContext, &QObject::deleteLater);
}


Engine::~Engine()
{
    closeUsi();
}


void Engine::openUsi(const QString &)
{
    auto *thread = dynamic_cast<EngineThread*>(_engineContext);
    thread->start();
    while (!thread->isRunning()) {
        QThread::msleep(10);
    }
}


void Engine::closeUsi()
{
    auto *thread = dynamic_cast<EngineThread*>(_engineContext);
    thread->quit();
    thread->wait();
}
