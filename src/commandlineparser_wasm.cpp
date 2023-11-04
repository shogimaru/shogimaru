#include "commandlineparser.h"
#include <emscripten/val.h>
#include <emscripten.h>


void CommandLineParser::process(const QCoreApplication &)
{
    emscripten::val location = emscripten::val::global("location");
    QString href = QString::fromStdString(location["href"].as<std::string>());
    int idx = href.indexOf("?");

    if (idx > 0) {
        QStringList params = href.mid(idx + 1).split("&");
        for (auto &param : params) {
            idx = param.indexOf("=");
            if (idx >= 0) {
                _values.insert(param.mid(0, idx), param.mid(idx + 1));
                qWarning() << param.mid(0, idx) << param.mid(idx + 1);
            } else {
                _values.insert(param, QString());
            }
        }
    }
}


QString	CommandLineParser::value(const QCommandLineOption &option) const
{
    return _values.value(option.names().value(0));
}
