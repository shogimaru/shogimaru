#include <enginethread.h>
#include <QDebug>

extern int yaneuraOu();


void EngineThread::run()
{
    yaneuraOu();
}


void EngineThread::start()
{
    QThread::start();
}


void EngineThread::terminate()
{
    QThread::terminate();
}
