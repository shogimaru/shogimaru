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
#ifdef Q_OS_WIN64
    QFont font("Meiryo", 12);
#else
    int id = QFontDatabase::addApplicationFont("assets/fonts/ipagp.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont font(family);
#endif
    QApplication::setFont(font);

    MainController control;
    control.show();

    int ret = app.exec();
    return ret;
}
