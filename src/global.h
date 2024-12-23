#pragma once
#include <QString>
#include <functional>
#include <list>
#include <string>
#include <vector>

class QWidget;
class QLabel;
class QFont;

namespace maru {

// バージョン
constexpr auto SHOGIMARU_VERSION_STR = "1.5.1";

// 手番
enum Turn : int {
    Sente = 0x1000,  // 先手
    Gote = 0x2000,   // 後手
    TurnMask = 0xF000,
};

// 時間方式
enum TimeMethod : int {
    Byoyomi = 0,  // 通常
    Fischer = 1,  // フィッシャー（1手毎加算）
};

// 開始局面
enum StartPosision : int {
    Initial = 0,  // 初期局面
    Current,      // 現在の局面
};

// プレイヤ
enum PlayerType : int {
    Human,
    Computer,
};

// 試合結果
enum GameResult : int {
    Win = 1,  // 勝利
    Loss,  // 敗北
    Draw,  // 引き分け
    Illegal,  // 反則（負け）
    Abort,  // 中断
    ResultMask = 0xF,
};

// 結果詳細
enum ResultDetail : int {
    Win_Declare = 1 << 4,  // 入玉勝ち宣言
    Loss_Resign = 2 << 4,  // 投了
    Draw_Repetition = 3 << 4,  // 千日手
    Draw_Impasse = 4 << 4,  // 持将棋（双方入玉）
    Illegal_OutOfTime = 5 << 4,  // 時間切れ
    Illegal_TwoPawns = 6 << 4,  // 二歩
    Illegal_DropPawnMate = 7 << 4,  // 打ち歩詰め
    Illegal_OverlookedCheck = 8 << 4,  // 王手放置
    Illegal_PerpetualCheck = 9 << 4,  // 連続王手の千日手
    Illegal_Other = 10 << 4,  // 反則その他
    Abort_GameAborted = 11 << 4,  // 中断
    DetailMask = 0xF0,
};

// COMレベル
enum ComputerLevel : int {
    R3000 = 3000,
    R2800 = 2800,
    R2600 = 2600,
    R2400 = 2400,
    R2200 = 2200,
    R2000 = 2000,
    R1800 = 1800,
    R1600 = 1600,
    R1400 = 1400,
    R1200 = 1200,
    R1000 = 1000,
};

// Data key
enum Key : int {
    Coord,
};

enum ParameterName : int {
    Hoge,
};


int64_t globalIntParameter(ParameterName name);
void setGlobalIntParameter(ParameterName name, int64_t value);
QString globalStringParameter(ParameterName name);
void setGlobalStringParameter(ParameterName name, const QString &value);

// std::string用
std::string toLower(const std::string &str);
std::string trim(const std::string &input);
bool contains(const std::list<std::string> &stringlist, const std::string &str);
std::string join(const std::list<std::string> &stringlist, const std::string &separator);
std::vector<std::string> split(const std::string &str, char sep, bool skipEmptyParts = false);

int random(int min, int max);
QString elideText(const QString &text, int width, const QFont &font);
QString elideText(const QString &text, const QLabel *label);
QWidget *mainWindow();
bool isLocaleLangJapanese();
QString appLocalDataLocation();
QString appResourcePath(const QString &relativePath);
bool isDarkMode();

}
