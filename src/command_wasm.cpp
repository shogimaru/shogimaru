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


std::string CBus::get()
{
    std::string ret;
    std::unique_lock<std::mutex> lock(_mutex);

    if (!_commands.empty()) {
        ret = _commands.front();
        _commands.pop_front();
    }
    return ret;
}


std::list<std::string> CBus::getAll()
{
    std::unique_lock<std::mutex> lock(_mutex);
    auto ret = _commands;  // copy
    _commands.clear();
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
    _cond.notify_one();
}

/*!
  コマンド待機
  msecs:ミリ秒  -1:無期限待機  0:ノンブロック
*/
bool CBus::wait(int msecs) const
{
    std::unique_lock<std::mutex> lock(_mutex);

    if (_commands.empty()) {
        if (msecs < 0) {
            _cond.wait(lock);
        } else if (msecs > 0) {
            _cond.wait_for(lock, std::chrono::milliseconds(msecs));
        } else {
            return false;
        }
    }
    return !_commands.empty();
}


std::string Command::wait(int msecs)
{
    return _request.wait(msecs) ? _request.get() : std::string();
}


void Command::reply(const std::string &response)
{
    if (!response.empty()) {
        //qDebug() << response.c_str();
        _response.set(response);
    }
}


void Command::request(const std::string &command)
{
    qDebug() << command.c_str();
    _request.set(command);
}


static std::string join(const std::list<std::string> &strs, const std::string &separator)
{
    std::string ret;
    for (auto &str : strs) {
        ret += str;
        ret += separator;
    }
    if (ret.size() > 0) {
        ret.resize(ret.size() - separator.size());
    }
    return ret;
}


std::list<std::string> Command::poll(int msecs)
{
    if (_response.wait(msecs)) {
        auto res = _response.getAll();
        qDebug() << "response" << join(res, " / ").c_str();
        return res;
    }
    return std::list<std::string>();
}


bool Command::pollFor(const std::string &response, int msecs)
{
    while (_response.wait(msecs)) {
        auto res = _response.get();
        qDebug() << "response" << res.c_str();
        if (res.find(response) == 0) {
            //qDebug() << "pollFor" << QString::fromStdString(response) << ":" << QString::fromStdString(res);
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

    int time = msecs;
    int64_t limit = currentMSecsSinceEpoch() + msecs;
    do {
        _response.wait(time);
        _response.getAll();
        time = limit - currentMSecsSinceEpoch();
    } while (time > 0);
}
