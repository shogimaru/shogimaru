#include "file.h"


File::File(const QString &name) :
    QFile(name)
{ }


File::~File()
{ }


bool File::open(QIODevice::OpenMode mode)
{
    return QFile::open(mode);
}


void File::close()
{
    QFile::close();
}


bool File::flush()
{
    return QFile::flush();
}


void File::mountDevice()
{ }
