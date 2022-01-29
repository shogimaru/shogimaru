#include "engine.h"
#include "engineprocess.h"
#include <QCoreApplication>
#include <QDebug>


Engine::Engine(QObject *parent) :
    QObject(parent),
    _timer(new QTimer(this))
{
    connect(_timer, &QTimer::timeout, this, &Engine::getResponse);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, EngineProcess::instance(), &EngineProcess::terminate);
    init();
}


Engine::~Engine()
{
    EngineProcess::instance()->terminate();
    EngineProcess::instance()->waitForFinished(5000);
}


void Engine::start()
{
    EngineProcess::instance()->start();
}
