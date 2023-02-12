#include "engine.h"
#include "enginethread.h"
#include <QDebug>

Engine::EngineInfo Engine::wasmEngineInfo;


Engine::Engine(QObject *parent) :
    QObject(parent),
    _timer(new QTimer(this)),
    _errorTimer(new QTimer(this))
{
    _errorTimer->setSingleShot(true);  // シングルショット
    connect(_timer, &QTimer::timeout, this, &Engine::getResponse);
    connect(_errorTimer, &QTimer::timeout, this, &Engine::engineError);
}


bool Engine::openContext(const QString &)
{
    delete _engineContext;
    auto *thread = new EngineThread(this);
    thread->start();
    while (!thread->isRunning()) {
        QThread::msleep(10);
    }
    _engineContext = thread;
    return true;
}


void Engine::closeContext()
{
    if (_engineContext) {
        auto *thread = dynamic_cast<EngineThread *>(_engineContext);
        if (thread->isRunning()) {
            quit();
            thread->wait();
            thread->deleteLater();
        }
    }
    _engineContext = nullptr;
}


Engine::EngineInfo Engine::getEngineInfo(const QString &path)
{
    // メモリ使用量を抑えるために何度もエンジンを起動しない

    if (wasmEngineInfo.options.isEmpty()) {
        auto engine = new Engine;
        engine->open(path);
        engine->close();
        delete engine;
    }
    return wasmEngineInfo;
}
