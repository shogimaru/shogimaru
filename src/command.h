#pragma once
#include <ostream>
#include <list>
#include <condition_variable>
#include <mutex>

class EngineProcess;


class CBus {
public:
    void set(const std::string &cmd);
    std::string get();
    std::list<std::string> getAll();
    bool wait(int msecs) const;

private:
    std::list<std::string> _commands;
    mutable std::mutex _mutex;
    mutable std::condition_variable _cond;

    CBus() {}
    friend class Command;
};


class Command {
public:
    // GUI side
    void request(const std::string &command);
    std::list<std::string> poll(int msecs = 1000);
    bool pollFor(const std::string &response, int msecs);
    void clearResponse(int msecs = 100);

    // Engine side
    std::string wait(int msecs = -1);
    void reply(const std::string &response);

    static Command &instance();
    static void setEngine(EngineProcess *engine);

private:
    static EngineProcess *engineProcess;

    CBus _request;
    CBus _response;
    Command() {}
};
