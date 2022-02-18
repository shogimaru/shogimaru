#include "maincontroller.h"
#include "file.h"
#include <QApplication>
#include <QFontDatabase>
#include <QTimer>
#include <QTranslator>
#include <QDebug>
#include <locale>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    File::mountDevice();

    QTranslator translator;
    qDebug() << QLocale::system().name();
    QString ts = QString("message_") + QLocale::system().name();
    if (translator.load(ts, "assets/translations/")) {
        app.installTranslator(&translator);
    }

    // Set font
#if defined(Q_OS_WASM)
    int id = QFontDatabase::addApplicationFont("assets/fonts/ipagp.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont ipafont(family);
#elif defined(Q_OS_WIN64)
    QFont ipafont("Meiryo");
#else
    QFont ipafont("PGothic");
#endif
    QApplication::setFont(ipafont);

    MainController control;
    control.show();

    int ret = app.exec();
    return ret;
}
