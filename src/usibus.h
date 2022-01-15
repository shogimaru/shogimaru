#pragma once
#include <ostream>
#include <sstream>

namespace usi
{

class UsiBus : public std::ostream
{
public:
    explicit UsiBus(std::stringbuf* buf) : std::ostream(buf) {}
    virtual ~UsiBus() {}
};

extern UsiBus cmd;

} // namespace usi
