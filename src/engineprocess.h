#pragma once
#include <QProcess>


class EngineProcess : public QProcess {
    Q_OBJECT
public:
    virtual ~EngineProcess() {}
    void start(QIODevice::OpenMode mode = ReadWrite);

    static EngineProcess *instance();

public slots:
    void terminate();

private:
    EngineProcess();
};
