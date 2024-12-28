#include "file.h"


File::File(const QString &name) :
    QFile(name)
{ }


bool File::flush()
{
    return QFile::flush();
}


void File::mountDevice() { }

bool File::isDeviceMounted() { return true; }
