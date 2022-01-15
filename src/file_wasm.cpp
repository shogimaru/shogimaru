#include <emscripten.h>
#include "file.h"
#include <QFileInfo>

const QLatin1String workspace("/workspace/");
static int mountDone = 0;


static void waitForMounted()
{
    if (!mountDone) {
        while (!(mountDone = emscripten_run_script_int("Module.mountDone"))) {
            emscripten_sleep(10);
        }
    }
}


File::File(const QString &name) :
    QFile()
{
    QString path = name;
    if (QFileInfo(name).isRelative()) {
        path = workspace + name;
    }
    setFileName(path);
}


File::~File()
{
    close();
}


bool File::open(QIODevice::OpenMode mode)
{
    waitForMounted();
    return QFile::open(mode);
}


void File::close()
{
    if (isOpen()) {
        QFile::close();
        flush();
    }
}


bool File::flush()
{
    waitForMounted();

    // syncfs
    EM_ASM(
        FS.syncfs(function(err) {
            // do nothing
        });
    );
    return true;
}


void File::mountDevice()
{
    EM_ASM(
        FS.mkdir('/workspace');
        FS.mount(IDBFS, {}, '/workspace');
        Module.mountDone = 0;

        // sync from persisted state into memory
        FS.syncfs(true, function(err) {
            // populate: ファイルシステム投入  （省略時:false）
            //   true:  IDBからファイルシステムへ同期
            //   false: ファイルシステムからIDBへ同期
            Module.mountDone = 1;
        });
    );
}
