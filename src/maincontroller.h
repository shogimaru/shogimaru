#pragma once
#include "global.h"
#include "ponderinfo.h"
#include "player.h"
#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QVector>
#include <QElapsedTimer>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class Board;
class Piece;
class ChessClock;
class Recorder;
class Engine;
class NicknameDialog;
class AnalysisDialog;
class RecordDialog;
class StartDialog2;
class MyPage;
class EvaluationGraph;
class QAbstractButton;
class QComboBox;
class QMessageBox;
class ScoreItem;


class MainController : public QMainWindow
{
    Q_OBJECT
public:
    // モード
    enum Mode : int {
        Watch,  // 棋譜再生
        Rating, // 対局（レーティング戦）
        Analyzing,  // 検討
        Edit,   // 編集
    };

    explicit MainController(QWidget *parent = nullptr);
    ~MainController();

    Player currentPlayer() const;
    void stopGame(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail);
    void record(const QPair<Piece*, QString> &move, bool check);
    void showResult(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail);
    void changeTurn();
    void setTurn(maru::Turn turn);
    void rotate(bool rotation);  // 上下回転
    void setSentePlayer(const Player &player);
    void setGotePlayer(const Player &player);
    void showGameoverBox(const QString &msg) const;
    bool isIllegalMove();
    void showAnalyzingMoves(const QVector<ScoreItem> &scores, const QByteArray &sfen);
    void clear();

public slots:
    void newRatingGame();
    void startGame();
    void move();
    void pondered(const PonderInfo &info);
    void resign();  // 投了確認
    void slotResign(QAbstractButton *button); // 投了
    void retract();  // 待った
    void updateBoard();
    void updateResize(int resizeMainWindow = false);
    void toggleRotate();  // 上下回転（トグル）
    void engineWin();  // エンジン勝ち宣言
    void engineResign();  // エンジン投了
    void slotRecordItemSelected();
    void gameoverTimeout();   // 時間切れ
    void updateRemainingTime();  // 残り時間表示更新
    void setCurrentRecordRow(int move);
    void startAnalyzing();  // 棋譜解析
    void slotAnalysisAction();  // 解析ボタンクリック
    void openInfoBox();
    void loadSfen();
    void saveFile(const QString &filePath);
    void slotAnalysisTimeout();

protected:
    void recordResult(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail);
    void setGraphScores();
    void displayTurn(maru::Turn turn);
    void slotPonderedItemSelected(int row, int column);
    void nextAnalysis();
    void timerEvent(QTimerEvent *event) override;

signals:
    void timeout();

private:
    void createToolBar();
    void createInfoBox();
    void updateButtonStates();
    void showRemainingTime(maru::Turn turn, int time, int byoyomi);
    void showAnalysisInfo();

    Ui::MainWindow *_ui {nullptr};
    Board *_board {nullptr};
    QComboBox *_boardScaleBox {nullptr};
    ChessClock *_clock {nullptr};
    Recorder *_recorder {nullptr};
    Mode _mode {Watch};
    QMap<maru::Turn, Player> _players;
    NicknameDialog *_nicknameDialog {nullptr};
    AnalysisDialog *_analysisDialog {nullptr};
    RecordDialog *_recordDialog {nullptr};
    StartDialog2 *_startDialog {nullptr};
    MyPage *_myPage {nullptr};
    QMessageBox *_infoBox {nullptr};
    EvaluationGraph *_graph {nullptr};
    bool _rotated {false};  // 上下回転表示
    bool _ponderFlag {true};  // 先読み
    int _boardScale {0};
    int _analysisMoves {0};
    int _analysisTimerId {0};
    QElapsedTimer _analysisTimer;
    QElapsedTimer _elapsedTimer;
    QTimer _ponderTimer;
    PonderInfo _lastPonder;
    qint64 _lastPvShownTime {0};
};
