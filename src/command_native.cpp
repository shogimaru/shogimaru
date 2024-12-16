#include "command.h"
#include "engineprocess.h"
#include "global.h"
#include <QDebug>
#include <QDateTime>


Command::Command() {}

Command::~Command() {}


void Command::request(const std::string &command)
{
    //qDebug() << "request" << command.c_str();
    Command::engineProcess->write(command.c_str(), command.length());
    Command::engineProcess->write("\n");
    Command::engineProcess->waitForBytesWritten(1000);
}


std::list<std::string> Command::poll(int msecs)
{
    std::list<std::string> responses;

    if (Command::engineProcess->canReadLine()
        || (Command::engineProcess->waitForReadyRead(msecs) && Command::engineProcess->canReadLine())) {
        do {
            auto line = Command::engineProcess->readLine().trimmed();
            if (!line.isEmpty()) {
                //qDebug() << "poll" << line;
                responses.push_back(line.data());
            }
        } while (Command::engineProcess->canReadLine());
    }
    return responses;
}


bool Command::pollFor(const std::string &waitingResponse, int msecs, std::list<std::string> &response)
{
    response.clear();

    while (Command::engineProcess->canReadLine()
        || (Command::engineProcess->waitForReadyRead(msecs) && Command::engineProcess->canReadLine())) {
        auto line = Command::engineProcess->readLine().trimmed();
        //qDebug() << "pollFor" << line;
        response.push_back(line.toStdString());

        if (line.startsWith(waitingResponse.c_str())) {
            return true;
        }
    }
    return false;
}


void Command::clearResponse(int msecs)
{
    if (msecs < 0) {
        return;
    }

    const int64_t end = QDateTime::currentMSecsSinceEpoch() + msecs;
    int ms = msecs;

    do {
        if (Command::engineProcess->waitForReadyRead(ms)) {
            Command::engineProcess->readAll();
        }
        ms = end - QDateTime::currentMSecsSinceEpoch();
    } while (ms > 0);
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
