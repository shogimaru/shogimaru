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
    //connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, EngineProcess::instance(), &EngineProcess::terminate);
}


Engine::~Engine()
{
    close();
}


void Engine::openUsi(const QString &path)
{
    delete _engineContext;
    auto process = new EngineProcess(path, this);
    Command::setEngine(process);
    _engineContext = process;
    _engineContext->start();
}


void Engine::closeUsi()
{
    if (_engineContext) {
        auto *process = dynamic_cast<EngineProcess *>(_engineContext);
        if (process) {
            process->terminate();
            process->waitForFinished(5000);
        } else {
            qCritical() << "Intenal error";
        }
        qDebug() << "Engine::close()";
        delete _engineContext;
    }
    _engineContext = nullptr;
    Command::setEngine(nullptr);
}
