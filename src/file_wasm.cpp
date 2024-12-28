#include <emscripten.h>
#include "file.h"
#include <QFileInfo>

/*
  IDBFS:  /workspace/..
  組込FS: /assets/..
*/

const QLatin1String workspace("/workspace/");  // 書き込み可能


File::File(const QString &name) :
    QFile()
{
    QString path = name;
    if (QFileInfo(name).isRelative()) {
        path = workspace + name;
    }
    setFileName(path);
}


bool File::flush()
{
    QFile::flush();
    // qDebug() << "flush:" << QFileInfo(*this).absoluteFilePath();

    if (QFileInfo(*this).absoluteFilePath().startsWith(workspace)) {
        // syncfs
        EM_ASM(
            // then()に追加して順番に処理
            Module.syncfsQueue = Module.syncfsQueue.then(() => {
                return new Promise((resolve, reject) => {
                    FS.syncfs((err) => {
                        if (err) {
                            console.error("FS synchronization failed:", err);
                            reject(err);
                        } else {
                            console.log("FS synchronization completed");
                            resolve();
                        }
                    });
                });
            });
        );
    }
    return true;
}


void File::mountDevice()
{
    // /workspaceのマウント
    EM_ASM(
        const mountPoint = "/workspace";

        if (typeof Module.mountDone !== "undefined") {
            return;
        }

        Module.mountDone = 0;
        FS.mkdir(mountPoint);
        FS.mount(IDBFS, {}, mountPoint);
        Module.syncfsQueue = Promise.resolve();  // flush関数用

        // sync from persisted state into memory
        FS.syncfs(true, function(err) {
            // populate: ファイルシステム投入  （省略時:false）
            //   true:  IDBからファイルシステムへ同期
            //   false: ファイルシステムからIDBへ同期
            if (err) {
                console.error("IDBFS mount failed:", err);
            } else {
                Module.mountDone = 1;
                console.log("IDBFS mount completed");
            }
        });
    );
}


bool File::isDeviceMounted()
{
    static bool mountDone = 0;
    return mountDone ? mountDone : (mountDone = emscripten_run_script_int("Module.mountDone"));
}
