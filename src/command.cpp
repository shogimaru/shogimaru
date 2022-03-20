#include "command.h"
#include "global.h"
#include <QDebug>
#include <thread>

EngineProcess *Command::engineProcess = nullptr;


Command &Command::instance()
{
    static Command global;
    return global;
}


void Command::setEngine(EngineProcess *engine)
{
    engineProcess = engine;
}
