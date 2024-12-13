#include "command.h"
#include "global.h"
#include <QDebug>
#include <sys/time.h>


static int64_t currentMSecsSinceEpoch()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

// コマンド取得
//  msecs:ミリ秒  -1:無期限待機  0:ノンブロック
std::string CBus::get(int msecs)
{
    std::string ret;
    std::unique_lock<std::mutex> lock(_mutex);

    if (empty()) {
        if (msecs < 0) {
            _cond.wait(lock);
        } else if (msecs > 0) {
            _cond.wait_for(lock, std::chrono::milliseconds(msecs));
        } else {
            return ret;
        }
    }

    if (!empty()) {
        ret = _commands.front();
        _commands.pop_front();
    }
    return ret;
}

// コマンド取得
//  msecs:ミリ秒  -1:無期限待機  0:ノンブロック
std::list<std::string> CBus::getAll(int msecs)
{
    std::list<std::string> ret;
    std::unique_lock<std::mutex> lock(_mutex);

    if (empty()) {
        if (msecs < 0) {
            _cond.wait(lock);
        } else if (msecs > 0) {
            _cond.wait_for(lock, std::chrono::milliseconds(msecs));
        } else {
            return ret;
        }
    }

    if (!empty()) {
        ret = _commands;  // copy
        _commands.clear();
    }
    return ret;
}


void CBus::set(const std::string &cmd)
{
    auto cmds = maru::split(maru::trim(cmd), '\n');
    if (cmds.empty()) {
        return;
    }

    std::unique_lock<std::mutex> lock(_mutex);
    for (auto &s : cmds) {
        _commands.push_back(s);
    }
    lock.unlock();
    _cond.notify_one();
}


std::string Command::wait(int msecs)
{
    return _request.get(msecs);
}


void Command::reply(const std::string &response)
{
    auto res = maru::trim(response);
    if (!res.empty()) {
        _response.set(res);
    }
}


void Command::request(const std::string &command)
{
    _request.set(command);
}


std::list<std::string> Command::poll(int msecs)
{
    int ms = msecs;
    int64_t end = currentMSecsSinceEpoch() + msecs;

    do {
        auto res = _response.getAll(ms);
        if (!res.empty()) {
            return res;
        }
        ms = end - currentMSecsSinceEpoch();
    } while (ms > 0);
    return std::list<std::string>();
}


bool Command::pollFor(const std::string &waitingResponse, int msecs, std::list<std::string> &response)
{
    response.clear();
    int ms = msecs;
    int64_t end = currentMSecsSinceEpoch() + msecs;

    do {
        auto res = _response.get(ms);
        if (!res.empty()) {
            response.push_back(res);

            if (res.find(waitingResponse) == 0) {
                return true;
            }
        }
        ms = end - currentMSecsSinceEpoch();
    } while (ms > 0);
    return false;
}


void Command::clearResponse(int msecs)
{
    if (msecs < 0) {
        return;
    }

    const int64_t end = currentMSecsSinceEpoch() + msecs;
    int ms = msecs;

    do {
        _response.getAll(ms);
        ms = end - currentMSecsSinceEpoch();
    } while (ms > 0);
}
