#include "maincontroller.h"
#include "messagebox.h"
#include "recorder.h"
#include "sfen.h"
#include <QFile>


void MainController::saveFile(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    Sfen sfen = _recorder->toSfen();
    sfen.setPlayers(_players[maru::Sente].name(), _players[maru::Gote].name());
    auto data = sfen.toCsa().toLocal8Bit();
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        int len = file.write(data);
        file.close();
        if (len == data.length()) {
            // Success
            MessageBox::information(tr("Save"), tr("Save completed."));
            return;
        }
    }

    // Write error
    MessageBox::information(tr("Save Error"), tr("Failed to save the file."));
}
