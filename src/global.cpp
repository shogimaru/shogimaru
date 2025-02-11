#include "global.h"
#include <QApplication>
#include <QLabel>
#include <QMap>
#include <QStandardPaths>
#include <QDir>
#include <QWidget>
#include <random>
#ifndef Q_OS_WIN
#include <iconv.h>
#endif

namespace maru {

std::random_device randev;
std::default_random_engine randeng(randev());
QMap<ParameterName, int64_t> globalIntParameters;
QMap<ParameterName, QString> globalStringParameters;


int64_t globalIntParameter(ParameterName name)
{
    return globalIntParameters.value(name);
}


void setGlobalIntParameter(ParameterName name, int64_t value)
{
    globalIntParameters.insert(name, value);
}


QString globalStringParameter(ParameterName name)
{
    return globalStringParameters.value(name);
}


void setGlobalStringParameter(ParameterName name, const QString &value)
{
    globalStringParameters.insert(name, value);
}


bool is_space(char c)
{
    return (c == '\r' || c == '\n' || c == ' ' || c == '\t');
}


std::string toLower(const std::string &str)
{
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c) { return std::tolower(c); });
    return ret;
}

/*!
  先頭・末尾から空白文字を削除
*/
std::string trim(const std::string &input)
{
    std::string str = input;
    int pos = 0;
    int len = (int)str.length();

    while (pos < len && is_space(str[pos])) {
        pos++;
    }
    if (pos > 0) {
        str = str.substr(pos);
        len = (int)str.length();
    }

    while (len > 0 && is_space(str[len - 1])) {
        len--;
    }

    str.resize(len);
    return str;
}


bool contains(const std::list<std::string> &stringlist, const std::string &str)
{
    for (auto &entry : stringlist) {
        if (entry == str) {
            return true;
        }
    }
    return false;
}


std::string join(const std::list<std::string> &stringlist, const std::string &separator)
{
    std::string ret;
    for (auto &str : stringlist) {
        ret += str;
        ret += separator;
    }
    if (ret.size() > 0) {
        ret.resize(ret.size() - separator.size());
    }
    return ret;
}


std::vector<std::string> split(const std::string &str, char sep, bool skipEmptyParts)
{
    std::vector<std::string> ret;
    std::string::size_type pos = 0;
    std::string::size_type idx;

    while ((idx = str.find(sep, pos)) != std::string::npos) {
        auto part = str.substr(pos, idx - pos);
        if (!skipEmptyParts || !part.empty()) {
            ret.push_back(part);
        }
        pos = idx + 1;
    }
    if (pos < str.length()) {
        auto part = str.substr(pos, str.length() - pos);
        if (!skipEmptyParts || !part.empty()) {
            ret.push_back(part);
        }
    }
    return ret;
}


int random(int min, int max)
{
    std::uniform_int_distribution<int> uniform(min, max);  // 乱数
    return uniform(randeng);
}


QString elideText(const QString &text, int width, const QFont &font)
{
    return QFontMetrics(font).elidedText(text, Qt::ElideRight, width);
};


QString elideText(const QString &text, const QLabel *label)
{
    return QFontMetrics(label->font()).elidedText(text, Qt::ElideRight, label->width());
}

QWidget *mainWindow()
{
    static QWidget *widget = []() {
        for (auto *w : QApplication::topLevelWidgets()) {
            if (w->objectName() == "MainWindow") {
                return w;
            }
        }
        std::abort();
        return (QWidget *)nullptr;
    }();
    return widget;
}

bool isLocaleLangJapanese()
{
    static const bool langJa = QLocale::system().name().toLower().startsWith("ja");
    return langJa;
}

QString appLocalDataLocation()
{
#ifdef Q_OS_WASM
    return QLatin1String("/workspace/");
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#endif
}

QString appResourcePath(const QString &relativePath)
{
#ifdef Q_OS_WASM
    return QLatin1String("/") + relativePath;
#elif defined(Q_OS_DARWIN)
    QString resourcesPath = QCoreApplication::applicationDirPath() + "/../Resources/";
    return QDir(resourcesPath).absolutePath() + "/" + relativePath;
#else
    return QCoreApplication::applicationDirPath() + "/" + relativePath;
#endif
}

bool isDarkMode()
{
    auto palette = dynamic_cast<QApplication*>(QCoreApplication::instance())->palette();
    // Window背景色とWindowTextの色を比較してモードを推測
    QColor backgroundColor = palette.color(QPalette::Window);
    QColor textColor = palette.color(QPalette::WindowText);
    return backgroundColor.lightness() < textColor.lightness();
}

}
