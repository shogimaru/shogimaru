#include "global.h"
#include <QMap>
#include <random>

namespace maru {

std::random_device randev;
std::default_random_engine randeng(randev());
QMap<ParameterName, int64_t> globalIntParameters;
QMap<ParameterName, QString> globalStringParameters;


int64_t globalIntParameter(ParameterName name)
{
    return globalIntParameters.value(name);
}


void setGlobalIntParameter(ParameterName name, int64_t value)
{
    globalIntParameters.insert(name, value);
}


QString globalStringParameter(ParameterName name)
{
    return globalStringParameters.value(name);
}


void setGlobalStringParameter(ParameterName name, const QString &value)
{
    globalStringParameters.insert(name, value);
}


bool is_space(char c)
{
    return (c == '\r' || c == '\n' || c == ' ' || c == '\t');
}

/*!
  先頭・末尾から空白文字を削除
*/
std::string trim(const std::string &input)
{
    std::string str = input;
    int pos = 0;
    int len = str.length();

    while (pos < len && is_space(str[pos])) {
        pos++;
    }
    if (pos > 0) {
        str = str.substr(pos);
        len = str.length();
    }

    while (len > 0 && is_space(str[len - 1])) {
        len--;
    }

    str.resize(len);
    return str;
}


bool contains(const std::list<std::string> &stringlist, const std::string &str)
{
    for (auto &entry : stringlist) {
        if (entry == str) {
            return true;
        }
    }
    return false;
}


std::vector<std::string> split(const std::string &str, char sep, bool skipEmptyParts)
{
    std::vector<std::string> ret;
    std::string::size_type pos = 0;
    std::string::size_type idx;

    while ((idx = str.find(sep, pos)) != std::string::npos) {
        auto part = str.substr(pos, idx - pos);
        if (!skipEmptyParts || !part.empty()) {
            ret.push_back(part);
        }
        pos = idx + 1;
    }
    if (pos < str.length()) {
        auto part = str.substr(pos, str.length() - pos);
        if (!skipEmptyParts || !part.empty()) {
            ret.push_back(part);
        }
    }
    return ret;
}


int random(int min, int max)
{
    std::uniform_int_distribution<int> uniform(min, max);  // 乱数
    return uniform(randeng);
}

}
