#include "usibus.h"
#include "command.h"

class command_buf : public std::basic_stringbuf<char>
{
public:
    int sync() override;
};


int command_buf::sync()
{
    int size = pptr() - pbase();
    if (size > 0) {
        std::string msg(pbase(), size);
        Command::instance().reply(msg);
        pbump(pbase() - pptr());    // reset
    }
    return 0;
}

// global usi::cmd variable
usi::UsiBus usi::cmd(new command_buf);
