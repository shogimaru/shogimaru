#include "file.h"
#include "global.h"
#include "maincontroller.h"
#include "commandlineparser.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFontDatabase>
#include <QTimer>
#include <QTranslator>
#include <QStyleFactory>
#include <QStyleHints>
#include <locale>
#include <cstdlib>


int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("Shogimaru");
    QCoreApplication::setApplicationVersion(maru::SHOGIMARU_VERSION_STR);

    CommandLineParser parser;
    parser.setApplicationDescription("Shogi GUI supporting USI protocol");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption showRecord(QStringList() << "s" << "show-record", "Shows a shogi record of ShogiDB2.", "hash");
    parser.addOption(showRecord);
    parser.process(QCoreApplication(argc, argv));

    //-------------------------------------------
    // Access the window manager system from here
    //-------------------------------------------

    QApplication app(argc, argv);
    File::mountDevice();
    QTranslator translator;
    qDebug() << QLocale::system().name();
    QString ts = QString("message_") + QLocale::system().name();
    if (translator.load(ts, maru::appResourcePath("assets/translations/"))) {
        app.installTranslator(&translator);
    }

    // Light mode
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);

    // Style
    app.setStyle(QStyleFactory::create("Fusion"));

    // Set font
#ifdef Q_OS_WIN64
    QFont font("Yu Gothic UI");
    font.setPointSizeF(11.5);
#elif defined(Q_OS_DARWIN)
    QFont font("SF Pro Text");
    font.setPointSizeF(12);
#else
    int id = QFontDatabase::addApplicationFont(maru::appResourcePath("assets/fonts/ipagp.ttf"));
    QString family = QFontDatabase::applicationFontFamilies(id).value(0);
    QFont font(family, 12);
#endif
    QApplication::setFont(font);

    QString path = maru::appLocalDataLocation();
    QDir(path).mkpath(".");

    MainController control;
    control.show();

    auto hash = parser.value(showRecord);
    if (!hash.isEmpty()) {
        control.showUrlRecord(hash);
    }

    int ret = app.exec();
    std::exit(ret);
    return ret;
}
