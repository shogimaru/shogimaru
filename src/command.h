#pragma once
#include <condition_variable>
#include <list>
#include <mutex>
#include <ostream>

class EngineProcess;
class CBus;


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

    CBus *_request {nullptr};   // For wasm use
    CBus *_response {nullptr};  // For wasm use

    Command();
    ~Command();
    Command(const Command &) = delete;
    Command &operator=(const Command &) = delete;
};
