#pragma once
#include <QCommandLineOption>
#include <QCoreApplication>

#ifdef Q_OS_WASM

#include <QMap>

class CommandLineParser {
public:
    void setApplicationDescription(const QString &) {}
    void addHelpOption() {}
    void addVersionOption() {}
    bool addOption(const QCommandLineOption &) { return true; }
    void process(const QCoreApplication &);
    QString	value(const QCommandLineOption &option) const;

private:
    QMap<QString, QString> _values;
};


#else

#include <QCommandLineParser>

class CommandLineParser : public QCommandLineParser {
};

#endif
