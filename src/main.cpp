#include "global.h"
#include "file.h"
#include "maincontroller.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFontDatabase>
#include <QTimer>
#include <QTranslator>
#include <locale>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Shogimaru");
    QCoreApplication::setApplicationVersion(maru::SHOGIMARU_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Shogi GUI supporting USI protocol");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    File::mountDevice();
    QTranslator translator;
    qDebug() << QLocale::system().name();
    QString ts = QString("message_") + QLocale::system().name();
    if (translator.load(ts, "assets/translations/")) {
        app.installTranslator(&translator);
    }

    // Set font
#ifdef Q_OS_WIN64
    QFont font("Yu Gothic UI");
    font.setPointSizeF(11.5);
#else
    int id = QFontDatabase::addApplicationFont("assets/fonts/ipagp.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont font(family, 12);
#endif
    QApplication::setFont(font);

    MainController control;
    control.show();

    int ret = app.exec();
    return ret;
}
