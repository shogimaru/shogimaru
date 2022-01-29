#pragma once
#include <QString>
#include <string>
#include <vector>
#include <list>
#include <functional>


namespace maru {

enum Turn : int {
    Sente = 0x1000,
    Gote  = 0x2000,
};

// プレイヤ
enum PlayerType : int {
    Human,
    Computer,
};

// 試合結果
enum GameResult: int {
    None = 0,
    Win,   // 勝利
    Loss,  // 敗北
    Foul,  // 反則負け
    Draw,  // 引き分け
};

// 結果詳細
enum ResultDetail : int {
    Abort = 0x10,    // 中断
    Win_Declare,     // 入玉勝ち宣言
    Loss_Resign,     // 投了
    Foul_OutOfTime,  // 時間切れ
    Foul_TwoPawns,   // 二歩
    Foul_DropPawnMate, // 打ち歩詰め
    Foul_OverlookedCheck,  // 王手放置
    Foul_PerpetualCheck,  // 連続王手の千日手
    Draw_Repetition,  // 千日手
    Draw_Impasse,     // 持将棋（双方入玉）
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

std::string trim(const std::string &input);
bool contains(const std::list<std::string> &stringlist, const std::string &str);
std::vector<std::string> split(const std::string &str, char sep, bool skipEmptyParts = false);
int random(int min, int max);

}
