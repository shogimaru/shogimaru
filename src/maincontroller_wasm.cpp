#include "maincontroller.h"
#include "recorder.h"
#include "sfen.h"
#include <emscripten.h>


void MainController::saveFile(const QString &filePath)
{
    Sfen sfen(_recorder->sfenMoves(99999));
    sfen.setPlayers(_players[maru::Sente].name(), _players[maru::Gote].name());

    auto sendTextFile = [](const char *buf, size_t length, const char *fileName, size_t fileNameLength) {
        EM_ASM({
            const textData = UTF8ToString($0, $1);
            const filename = UTF8ToString($2, $3);
            const blob = new Blob([textData], { type : "text/plain" });
            const url = URL.createObjectURL(blob);
            const a = document.createElement("a");
            document.body.appendChild(a);
            a.style = "display:none";
            a.download = filename;
            a.href = url;
            a.click();
            a.remove();
            URL.revokeObjectURL(url);
        },
            buf, length, fileName, fileNameLength);
    };

    QByteArray data = sfen.toCsa().toUtf8();
    QByteArray fileName = filePath.toUtf8();
    sendTextFile(data.constData(), data.size(), fileName.constData(), fileName.size());
}
