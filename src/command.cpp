#include "command.h"
#include "global.h"
#include <QDebug>
#include <thread>


Command &Command::instance()
{
    static Command global;
    return global;
}
