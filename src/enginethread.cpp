#include <enginethread.h>
#include <QDebug>

extern int YaneuraOu();


void EngineThread::run()
{
    YaneuraOu();
}


void EngineThread::start()
{
    QThread::start();
}


void EngineThread::terminate()
{
    QThread::terminate();
}
