#include "engine.h"
#include "enginethread.h"


Engine::Engine(QObject *parent) :
    QObject(parent),
    _timer(new QTimer(this)),
    _engineThread(new EngineThread(this))
{
    connect(_timer, &QTimer::timeout, this, &Engine::getResponse);
    connect(_engineThread, &QThread::finished, _engineThread, &QObject::deleteLater);
    init();
}


Engine::~Engine()
{
    _engineThread->terminate();
    _engineThread->wait();
}


void Engine::start()
{
    _engineThread->start();
    while (!_engineThread->isRunning()) {
        QThread::msleep(10);
    }
}
