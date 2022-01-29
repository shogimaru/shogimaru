#include "command.h"
#include "global.h"
#include <QDebug>
#include <thread>
#include <sys/time.h>


Command &Command::instance()
{
    static Command global;
    return global;
}
