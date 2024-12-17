#pragma once
#include "global.h"
#include "player.h"
#include "ponderinfo.h"
#include <QElapsedTimer>
#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QTimer>
#include <QVector>

namespace Ui {
class MainWindow;
}

class Board;
class Piece;
class ChessClock;
class Recorder;
class Engine;
class StartDialog;
class StartDialog2;
class NicknameDialog;
class SettingsDialog;
class AnalysisDialog;
class RecordDialog;
class MyPage;
class EvaluationGraph;
class OperationButtonGroup;
class QAbstractButton;
class QComboBox;
class QMessageBox;
class ScoreItem;


class MainController : public QMainWindow {
    Q_OBJECT
public:
    // モード
    enum Mode : int {
        Watch,  // 棋譜再生
        Game,   // 通常対局
        Rating,  // レーティング対局
        Analyzing,  // 検討
        Edit,  // 編集
    };

    explicit MainController(QWidget *parent = nullptr);
    ~MainController();

    Player currentPlayer() const;
    void stopGame(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail);
    void record(const QPair<Piece *, QString> &move, bool check);
    void showResult(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail);
    void changeTurn();
    void setTurn(maru::Turn turn);
    void rotate(bool rotation);  // 上下回転
    void setSentePlayer(const Player &player);
    void setGotePlayer(const Player &player);
    QString eventName() const { return _eventName; }
    void setEventName(const QString &name);  // 棋戦名
    void showGameoverBox(const QString &msg) const;
    bool isIllegalMove();
    void showAnalyzingMoves(const QVector<ScoreItem> &scores, const QByteArray &sfen);
    void clear();
    void showUrlRecord(const QString &hash);

public slots:
    void newGame();
    void newRatingGame();
    void startGame();
    void move();
    void pondered(const PonderInfo &info);
    void resign();  // 投了確認
    void slotResign(QAbstractButton *button);  // 投了
    void retract();  // 待った
    void updateMainWindow();
    void updateBoard();
    void updateResize(int resizeMainWindow = false);
    void toggleRotate();  // 上下回転（トグル）
    void engineWin();  // エンジン勝ち宣言
    void engineResign();  // エンジン投了
    void slotRecordItemSelected();
    void gameoverTimeout();  // 時間切れ
    void updateRemainingTime();  // 残り時間表示更新
    void setCurrentRecordRow(int move);
    void slotSettingsAction();  // 設定ボタン
    void startAnalysis();  // 棋譜解析
    void startGo();
    void slotAnalysisAction();  // 解析ボタンクリック
    void openInfoBox();
    void loadSfen();
    void saveFile(const QString &filePath);
    void slotAnalysisTimeout();
    void engineError();
    void showSpinner();
    void hideSpinner();
    // 棋譜操作
    void showNext();
    void showNext10Moves();
    void showNextCandidate();
    void showPrevious();
    void showPrevious10Moves();
    void showPreviousCandidate();
    void showFirstPosition();
    void showLastPosition();

protected:
    void recordResult(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail);
    void setGraphScores();
    void displayTurn(maru::Turn turn);
    void slotPonderedItemSelected(int row, int column);
    void nextAnalysis();
    void stopAnalysis();
    bool startEngineForAnalysis(int moves);
    void stopEngineForAnalysis();
    void timerEvent(QTimerEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool openEngine();

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
    QString _eventName;
    StartDialog *_startDialog {nullptr};
    StartDialog2 *_startRatingDialog {nullptr};
    NicknameDialog *_nicknameDialog {nullptr};
    SettingsDialog *_settingsDialog {nullptr};
    AnalysisDialog *_analysisDialog {nullptr};
    RecordDialog *_recordDialog {nullptr};
    MyPage *_myPage {nullptr};
    QMessageBox *_infoBox {nullptr};
    EvaluationGraph *_graph {nullptr};
    OperationButtonGroup *_opeButtonGroup {nullptr};
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
    QLabel *_spinner {nullptr};
};
