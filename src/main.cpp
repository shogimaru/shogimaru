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

    // スタイル
    app.setStyle(QStyleFactory::create("Fusion"));
#if QT_VERSION >= 0x060800
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);  // 強制ライトモード
#else
    QPalette lightPalette;
    QColor fontColor(36, 36, 36);
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));     // 背景
    lightPalette.setColor(QPalette::Text, fontColor);                   // テキスト
    lightPalette.setColor(QPalette::WindowText, fontColor);             // テキスト
    lightPalette.setColor(QPalette::Base, Qt::white);                   // 入力フィールドの背景
    lightPalette.setColor(QPalette::AlternateBase, Qt::lightGray);      // 入力フィールドの別の背景
    lightPalette.setColor(QPalette::Button, QColor(246, 246, 246));     // ボタン背景
    lightPalette.setColor(QPalette::ButtonText, fontColor);             // ボタン文字
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));    // ハイライト色
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);        // ハイライト中の文字
    app.setPalette(lightPalette);
#endif

    // Set font
#ifdef Q_OS_WIN64
    QFont font("Yu Gothic UI");
    font.setPointSizeF(11.5);
#elif defined(Q_OS_DARWIN)
    QFont font("Hiragino Kaku Gothic ProN", 15);
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
