#include "command.h"
#include "engine.h"
#include "engineprocess.h"
#include "messagebox.h"
#include <QCoreApplication>
#include <QProcessEnvironment>
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

#ifdef Q_OS_WIN
    const QChar Delimiter(';');
#else
    const QChar Delimiter(':');
#endif

    // 環境変数設定
    if (!_environment.isEmpty()) {
        auto env = process->processEnvironment();
        for (auto &variable : _environment) {
            auto var = variable.toMap();
            QString name = var.value("name").toString();
            if (!name.isEmpty()) {
                QString val = env.value(name);
                val.prepend(var.value("value").toString() + Delimiter);
                env.insert(name, val);
            }
        }
        process->setProcessEnvironment(env);
    }

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


// Get Engine information, name, author, USI default options, etc.
Engine::EngineInfo Engine::getEngineInfo(const QString &path, const QVariantList &environment)
{
    Engine::EngineInfo info;
    auto engine = new Engine;
    engine->setEnvironment(environment);

    if (engine->open(path)) {
        info.name = engine->name();
        info.path = path;
        info.author = engine->author();
        info.options = engine->_usiDefaultOptions;
    } else {
        MessageBox::information(tr("Engine error"), tr("Failed to start the engine"));
    }
    engine->close();
    delete engine;
    return info;
}
