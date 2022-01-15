#pragma once

#include <QFile>


class File : public QFile {
public:
    File(const QString &name);
    virtual ~File();

    bool open(QIODevice::OpenMode mode) override;
    void close() override;
    bool flush();

    static void mountDevice();
};
