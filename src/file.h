#pragma once

#include <QFile>


class File : public QFile {
public:
    File(const QString &name);
    bool flush();

    static void mountDevice();
    static bool isDeviceMounted();
};
