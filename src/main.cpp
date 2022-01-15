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
    qDebug() << std::locale().name().c_str();
    qDebug() << QLocale::system().name();
    QString ts = QString("message_") + QLocale::system().name();
    if (translator.load(ts, "assets/translations/")) {
        app.installTranslator(&translator);
    }

    // Set font
    int id = QFontDatabase::addApplicationFont("assets/fonts/ipagp.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    //std::cout << qPrintable(family) << std::endl;
    //QFont ipafont("IPAGothic");
    //QFont ipafont("IPAPGothic");
    QFont ipafont(family);
    QApplication::setFont(ipafont);

    MainController control;
    control.show();

    int ret = app.exec();
    return ret;
}
