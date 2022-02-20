#include "command.h"
#include "engineprocess.h"
#include "global.h"
#include <QDebug>


std::string CBus::get()
{
    // do nothing
    return std::string();
}


std::list<std::string> CBus::getAll()
{
    // do nothing
    return std::list<std::string>();
}


void CBus::set(const std::string &)
{
    // do nothing
}

/*!
  コマンド待機
  msecs:ミリ秒  -1:無期限待機  0:ノンブロック
*/
bool CBus::wait(int) const
{
    // do nothing
    return true;
}


void Command::request(const std::string &command)
{
    //qDebug() << "request" << command.c_str();
    EngineProcess::instance()->write(command.c_str(), command.length());
    EngineProcess::instance()->write("\n");
    EngineProcess::instance()->waitForBytesWritten(1000);
}


std::list<std::string> Command::poll(int msecs)
{
    std::list<std::string> responses;

    if (EngineProcess::instance()->canReadLine()
        || (EngineProcess::instance()->waitForReadyRead(msecs) && EngineProcess::instance()->canReadLine())) {
        do {
            auto line = EngineProcess::instance()->readLine().trimmed();
            if (!line.isEmpty()) {
                //qDebug() << "response" << line;
                responses.push_back(line.data());
            }
        } while (EngineProcess::instance()->canReadLine());
    }
    return responses;
}


bool Command::pollFor(const std::string &response, int msecs)
{
    bool ret = false;

    while (EngineProcess::instance()->canReadLine()
        || (EngineProcess::instance()->waitForReadyRead(msecs) && EngineProcess::instance()->canReadLine())) {
        auto line = EngineProcess::instance()->readLine().trimmed();
        //qDebug() << "pollFor" << line;
        if (line.startsWith(response.c_str())) {
            break;
        }
    }
    return ret;
}


void Command::clearResponse(int msecs)
{
    if (EngineProcess::instance()->waitForReadyRead(msecs)) {
        EngineProcess::instance()->readAll();
    }
}


std::string Command::wait(int)
{
    // do nothing
    return std::string();
}


void Command::reply(const std::string &)
{
    // do nothing
}
