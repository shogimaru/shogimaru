#pragma once
#include <QtSystemDetection>
#include <condition_variable>
#include <list>
#include <mutex>
#include <ostream>

class EngineProcess;


#ifdef Q_OS_WASM
class CBus {
public:
    void set(const std::string &cmd);
    std::string get(int msecs);
    std::list<std::string> getAll(int msecs);
    bool empty() const { return _commands.empty(); }

private:
    std::list<std::string> _commands;
    mutable std::mutex _mutex;
    mutable std::condition_variable _cond;

    CBus() { }
    CBus(const CBus &) = delete;
    CBus &operator=(const CBus &) = delete;
    friend class Command;
};
#endif

class Command {
public:
    // GUI side
    void request(const std::string &command);
    std::list<std::string> poll(int msecs = 1000);
    bool pollFor(const std::string &waitingResponse, int msecs, std::list<std::string> &error);
    void clearResponse(int msecs = 50);

    // Engine side
    std::string wait(int msecs = -1);
    void reply(const std::string &response);

    static Command &instance();
    static void setEngine(EngineProcess *engine);

private:
    static EngineProcess *engineProcess;

#ifdef Q_OS_WASM
    CBus _request;
    CBus _response;
#endif

    Command() { }
    Command(const Command &) = delete;
    Command &operator=(const Command &) = delete;
};
