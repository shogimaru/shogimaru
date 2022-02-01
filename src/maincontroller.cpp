#include "maincontroller.h"
#include "ui_mainwindow.h"
#include "piece.h"
#include "board.h"
#include "shogirecord.h"
#include "engine.h"
#include "chessclock.h"
#include "nicknamedialog.h"
#include "analysisdialog.h"
#include "notationdialog.h"
#include "startdialog2.h"
#include "mypage.h"
#include "messagebox.h"
#include "sfen.h"
#include "recorder.h"
#include "user.h"
#include "kifu.h"
#include "sound.h"
#include "evaluationgraph.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTransform>
#include <QComboBox>
#include <QVariant>
#include <QDebug>
#include <cmath>
#include <algorithm>

const QSize BaseMainWindowSize (960, 820);
const QList<int> RatingList = {maru::R1000, maru::R1200, maru::R1400, maru::R1600, maru::R1800, maru::R2000, maru::R2200, maru::R2400, maru::R2600, maru::R2800, maru::R3000};


class EngineLevelMap : public QMap<int, int> {
public:
    EngineLevelMap() :
        QMap<int, int>()
    {
        insert(maru::R1000,  0);
        insert(maru::R1200,  2);
        insert(maru::R1400,  4);
        insert(maru::R1600,  6);
        insert(maru::R1800,  8);
        insert(maru::R2000, 10);
        insert(maru::R2200, 12);
        insert(maru::R2400, 14);
        insert(maru::R2600, 16);
        insert(maru::R2800, 18);
        insert(maru::R3000, 20);
    }
};
Q_GLOBAL_STATIC(EngineLevelMap, engineLevelMap)


class EngineLevelName : public QMap<int, QString> {
public:
    EngineLevelName() :
        QMap<int, QString>()
    {
        insert(maru::R1000, QString("R1000"));
        insert(maru::R1200, QString("R1200"));
        insert(maru::R1400, QString("R1400"));
        insert(maru::R1600, QString("R1600"));
        insert(maru::R1800, QString("R1800"));
        insert(maru::R2000, QString("R2000"));
        insert(maru::R2200, QString("R2200"));
        insert(maru::R2400, QString("R2400"));
        insert(maru::R2600, QString("R2600"));
        insert(maru::R2800, QString("R2800"));
        insert(maru::R3000, QString("R3000"));
    }
};
Q_GLOBAL_STATIC(EngineLevelName, engineLevelName)


MainController::MainController(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _board(new Board),
    _boardScaleBox(new QComboBox(this)),
    _clock(new ChessClock(this)),
    _recorder(new Recorder),
    _nicknameDialog(new NicknameDialog(this)),
    _analysisDialog(new AnalysisDialog(this)),
    _notationDialog(new NotationDialog(this)),
    _startDialog(new StartDialog2(this)),
    _myPage(new MyPage(this)),
    _infoBox(new QMessageBox(this)),
    _graph(new EvaluationGraph)
{
    _ui->setupUi(this);
    _ui->centralWidget->setLayout(_ui->mainVLayout);  // レイアウト
    // 評価グラフ
    _graph->setParent(_ui->graphWidget);
    _graph->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    setWindowTitle("Shogimaru");
    _ui->label->setText("<b>" + tr("Shogimaru") + "<\b>");

    // Toolbar
    createToolBar();

    // Infomation Box
    createInfoBox();

    auto boardRect = _ui->boardView->rect();
    int w = boardRect.width();
    int h = boardRect.height();

    // 将棋盤
    _board->setSceneRect(boardRect);
    _ui->boardView->setRenderHints(QPainter::SmoothPixmapTransform);
    _ui->boardView->setBackgroundBrush(Board::board().scaled(w, h));
    _ui->boardView->setScene(_board);

    // メッセージテーブルウィジット
    _ui->messageTableWidget->setGridStyle(Qt::NoPen);  // 罫線非表示
    _ui->messageTableWidget->setTextElideMode(Qt::ElideNone);  // 省略文字
    _ui->messageTableWidget->setWordWrap(false);
    _ui->messageTableWidget->setColumnWidth(0, 52);  // 1列目の幅
    _ui->messageTableWidget->setColumnWidth(1, 90);  // 2列目の幅

    // 情報行
    _ui->infoLine->setReadOnly(true);
    _ui->infoLine->hide();

    connect(_ui->newAction, &QAction::triggered, _startDialog, &QDialog::open);
    connect(_ui->analysisAction, &QAction::triggered, this, &MainController::slotAnalysisAction);
    connect(_startDialog, &QDialog::accepted, this, &MainController::newRatingGame);
    connect(_nicknameDialog, &QDialog::accepted, this, &MainController::newRatingGame);
    connect(_analysisDialog, &QDialog::accepted, this, &MainController::startAnalyzing);
    connect(_ui->notationAction, &QAction::triggered, _notationDialog, &NotationDialog::open);
    //connect(_ui->retractButton, &QPushButton::clicked, this, &MainController::retract); // 待った
    connect(_ui->rotateAction, &QAction::triggered, this, &MainController::toggleRotate);
    connect(_ui->resignAction, &QAction::triggered, this, &MainController::resign);
    connect(_ui->myPageAction, &QAction::triggered, _myPage, &MyPage::open);
    connect(_ui->infoAction, &QAction::triggered, this, &MainController::openInfoBox);
    connect(_ui->recordWidget, &QListWidget::itemSelectionChanged, this, &MainController::slotRecordItemSelected);
    connect(&Engine::instance(), &Engine::bestMove, _board, &Board::movePiece);
    connect(&Engine::instance(), &Engine::pondering, this, &MainController::pondered);
    connect(&Engine::instance(), &Engine::win, this, &MainController::engineWin);
    connect(&Engine::instance(), &Engine::resign, this, &MainController::engineResign);
    connect(_clock, &ChessClock::secondElapsed, this, &MainController::updateRemainingTime);
    connect(_clock, &ChessClock::timeout, this, &MainController::gameoverTimeout);
    connect(_board, &Board::moved, this, &MainController::move);
    connect(_graph, &EvaluationGraph::currentMoveChanged, this, &MainController::setCurrentRecordRow);
    connect(_ui->recordWidget, &QListWidget::currentRowChanged, _graph, &EvaluationGraph::setCurrentMove);
    connect(_ui->messageTableWidget, &QTableWidget::currentCellChanged, this, &MainController::slotPonderedItemSelected);

    auto scaleBoard = [=](int index) {  // 拡大率変更
        int scale = _boardScaleBox->itemData(index).toInt();
        if (scale > 0) {
            _boardScale = scale;
            auto &user = User::load();
            if (user.scale() != _boardScale) {
                user.setScale(_boardScale);
                user.save();
                updateResize(true);
                updateBoard();
            }
        }
    };
    connect(_boardScaleBox, QOverload<int>::of(&QComboBox::currentIndexChanged), scaleBoard);

    // ロード
    auto &user = User::load();
    for (int i = 0; i < _boardScaleBox->count(); i++) {
        if (_boardScaleBox->itemData(i).toInt() == user.scale()) {
            _boardScaleBox->setCurrentIndex(i);
            scaleBoard(i); // シグナルが飛ばないケースを避けるため必ず呼ぶ
            break;
        }
    }

    updateButtonStates();
    updateBoard();
}


MainController::~MainController()
{
    delete _nicknameDialog;
    delete _startDialog;
    delete _recorder;
    delete _clock;
    delete _board;
    delete _ui;
}


void MainController::setSentePlayer(const Player &player)
{
    _players[maru::Sente] = player;
    _ui->senteLabel->setText(QString::fromUtf8(u8"▲ ") + player.name());
}


void MainController::setGotePlayer(const Player &player)
{
    _players[maru::Gote] = player;
    _ui->goteLabel->setText(QString::fromUtf8(u8"△ ") + player.name());
}


void MainController::createToolBar()
{
    _ui->toolBar->addAction(_ui->newAction);
    _ui->toolBar->addSeparator();
    _ui->toolBar->addAction(_ui->resignAction);
    _ui->toolBar->addSeparator();
    _ui->toolBar->addAction(_ui->analysisAction);
    _ui->toolBar->addSeparator();
    _ui->toolBar->addAction(_ui->notationAction);
    _ui->toolBar->addSeparator();
    _ui->toolBar->addAction(_ui->rotateAction);
    _ui->toolBar->addSeparator();

    for (int i = 40; i <= 200; i += 20) {
        _boardScaleBox->addItem(QString::number(i) + "%",  i);
    }
    _ui->toolBar->addWidget(_boardScaleBox);
    _ui->toolBar->addSeparator();
    _ui->toolBar->addAction(_ui->myPageAction);
    _ui->toolBar->addSeparator();
    _ui->toolBar->addAction(_ui->infoAction);
}


void MainController::createInfoBox()
{
    _infoBox->setTextFormat(Qt::RichText);
    _infoBox->setStyleSheet("QLabel{min-width: 360px;}");

    QString text = QString("<h3>") + tr("Shogimaru") + "</h3><br><table><tbody>";
    text += QString("<tr><td>") + tr("Version") + "</td><td>1.0</td>";
    text += QString("<tr><td>") + tr("YaneuraOu") + "</td><td>" + "7.00" + "</td>";
    text += QString("<tr><td>Qt</td><td>") + QT_VERSION_STR + "</td>";
    text += QString("<tr><td>") + tr("Platform") + "</td><td>" + QSysInfo::prettyProductName() + "</td>";
    text += QString("</tbody></table>");

    _infoBox->setText(text);
    _infoBox->setStandardButtons(QMessageBox::Ok);
}


void MainController::openInfoBox()
{
    _infoBox->setMaximumWidth(360);
    _infoBox->open();
}


void MainController::updateButtonStates()
{
    switch (_mode) {
    case Watch: // 棋譜再生
        _ui->newAction->setEnabled(true);
        _ui->resignAction->setDisabled(true);
        _ui->analysisAction->setText(QCoreApplication::translate("MainWindow", "Analysis", nullptr));
        _ui->analysisAction->setEnabled(_recorder->count() > 1);
        _ui->notationAction->setEnabled(true);
        _ui->senteFrame->setStyleSheet("border: 1px solid silver");
        _ui->goteFrame->setStyleSheet("border: 1px solid silver");
        _ui->recordWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        _ui->messageTableWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        _ui->messageTableWidget->show();
        _graph->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        _graph->show();
        _ui->infoLine->hide();
        break;

    case Rating: // 対局
        _ui->newAction->setDisabled(true);
        _ui->resignAction->setEnabled(true);
        _ui->analysisAction->setText(QCoreApplication::translate("MainWindow", "Analysis", nullptr));
        _ui->analysisAction->setEnabled(false);
        _ui->notationAction->setEnabled(false);
        _ui->recordWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
        _ui->messageTableWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
        _ui->messageTableWidget->hide();
        _graph->setAttribute(Qt::WA_TransparentForMouseEvents);
        _graph->hide();
        _ui->infoLine->hide();
        break;

    case Analyzing: // 検討
        _ui->newAction->setEnabled(false);
        _ui->resignAction->setDisabled(true);
        _ui->analysisAction->setText(tr("Stop"));
        _ui->analysisAction->setEnabled(true);
        _ui->notationAction->setEnabled(false);
        _ui->senteFrame->setStyleSheet("border: 1px solid silver");
        _ui->goteFrame->setStyleSheet("border: 1px solid silver");
        _ui->recordWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
        _ui->messageTableWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
        _ui->messageTableWidget->show();
        _graph->setAttribute(Qt::WA_TransparentForMouseEvents);
        _graph->show();
        _ui->infoLine->show();
        break;

    case Edit: // 編集
        _ui->newAction->setEnabled(true);
        _ui->resignAction->setDisabled(true);
        _ui->analysisAction->setText(QCoreApplication::translate("MainWindow", "Analysis", nullptr));
        _ui->analysisAction->setEnabled(false);
        _ui->notationAction->setEnabled(true);
        _ui->senteFrame->setStyleSheet("border: 1px solid silver");
        _ui->goteFrame->setStyleSheet("border: 1px solid silver");
        _ui->recordWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        _ui->messageTableWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        _ui->messageTableWidget->show();
        _graph->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        _graph->show();
        _ui->infoLine->hide();
        break;

    default:
        break;
    }
}


void MainController::newRatingGame()
{
    // ユーザ情報
    _nicknameDialog->hide();

    // 持ち時間チェック
    int basicTime = _startDialog->basicTime() * 60000;
    int byoyomi = _startDialog->byoyomi() * 1000;
    if (basicTime == 0 && byoyomi == 0) {
        _startDialog->open();
        return;
    }

    User &user = User::load();

    // 空チェック
    if (user.nickname().isEmpty()) {
        _nicknameDialog->open();
        return;
    }

    // エンジンレベル
    int comRating = 0;
    if (user.rating() > maru::R3000) {
        comRating = maru::R3000;
    } else if (user.rating() < maru::R1000) {
        comRating = maru::R1000;
    } else {
        do {
            int idx = maru::random(0, RatingList.count() - 1);
            comRating = RatingList[idx];
        } while (std::abs(user.rating() - comRating) > 300);
    }

    int engineLevel = engineLevelMap()->value(comRating, 0);
    Engine::instance().setSkillLevel(engineLevel);
    Player computer(maru::Computer, engineLevelName()->value(comRating), comRating);
    Player human(maru::Human, user.nickname(), user.rating());

    QString msg;
    if (maru::random(0, 1)) {
        msg = tr("Your turn is gote (white).");
        setSentePlayer(computer);
        setGotePlayer(human);
    } else {
        msg = tr("Your turn is sente (black).");
        setSentePlayer(human);
        setGotePlayer(computer);
    }

    msg += "\n\n";
    msg += tr("Time control:%1min  Byoyomi:%2sec").arg(_startDialog->basicTime()).arg(_startDialog->byoyomi());
    msg += "\n\n";
    msg += tr("Good Luck!");
    MessageBox::information(tr("Game Start"), msg, this, SLOT(startGame()));
}


void MainController::startGame()
{
    // 初期局面
    static const QByteArray firstPosition = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";

    // // 千日手
    // static const QByteArray firstPosition =
    //     "l5knl/"
    //     "4r1g2/"
    //     "1Bn1gpsp1/"
    //     "p1pps1p1p/"
    //     "1p5P1/"
    //     "P1P1SPP1P/"
    //     "1PSPP4/"
    //     "2G2G3/"
    //     "LNK4RL "
    //     "b Pbn 1";

    // // 連続王手千日手
    // static const QByteArray firstPosition =
    //     "l4g2l/"
    //     "5g3/"
    //     "3p1p1k1/"
    //     "p1p3p1p/"
    //     "1p3P3/"
    //     "P1P1pn2+b/"
    //     "1PGP2PPN/"
    //     "L5SK1/"
    //     "5G2L "
    //     "b RSSPPPrbsnn 1";

    // // 打ち歩詰めチェック
    // static const QByteArray firstPosition =
    //     "l7l/"
    //     "4S1g2/"
    //     "3pp2p1/"
    //     "5Bp1p/"
    //     "P1SG1R3/"
    //     "2SP1G2P/"
    //     "KPN4k1/"
    //     "2P2PP2/"
    //     "Lr4S1L "
    //     "b NPPPbgnnpppp 1";

    // // 最後の審判
    // static const QByteArray firstPosition =
    //     "-+P-pS--+PR/"
    //     "--n--S-lg/"
    //     "-l---p-p-/"
    //     "-G--n-pS-/"
    //     "N-p--o---/"
    //     "---S--l--/"
    //     "----K-lgP/"
    //     "---P-+p--p/"
    //     "----Pg-PN "
    //     "b BPrb4p 1";

    // static QByteArray firstPosition = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2g2f 8c8d 2f2e 8d8e 6i7h 4a3b 3i3h 7a7b 9g9f 3c3d 2e2d 2c2d 2h2d 8e8f 8g8f 8b8f P*8g 8f8d 2d2h P*2c 7g7f 5a4b 3g3f 6c6d 3h3g 7c7d 3g4f 8a7c 2i3g 6a6b 4i4h 2b8h+ 7i8h 3a2b 8h7g 7b6c 2h2i 8d8a 5i6h 9c9d 8g8f 1c1d 6g6f 2b3c 1g1f 4c4d 3f3e 3d3e 4f3e P*3d 3e4f 7d7e 7f7e 6d6e B*5f 5c5d 6f6e 6b5c 1f1e 1d1e P*3e 3c2d 3e3d P*3f 3g2e 4d4e P*2b 3b2b 5f4e P*8h 7h8h 3f3g+ 4f3g 5c4d 4e5f 5d5e 5f7h 7c6e P*1b 1a1b P*1c 1b1c 2e1c+ 2a1c L*4f P*4e 4f4e N*5f 7h5f 5e5f 4e4d P*4c 4d4c+ 4b4c N*5e 4c5d 5g5f P*6g 6h6g P*6f 6g6f 6e7g+ G*6e 5d4d 6f7g S*5d P*6f 5d6e 6f6e 6c5d S*7f L*6b N*5g 4d5c 3g4f 9d9e 9f9e 9a9e P*1d 9e9i+ 1d1c+ G*9f 8h9g 9i8i 9g9f B*9i 7g6h L*6f 6h5h 9i7g+ 5g4e 5d4e G*4c 5c5d 4f4e 5d4e 4g4f 4e5f";
    // firstPosition = Sfen(firstPosition).toSfen();

    int basicTime = _startDialog->basicTime() * 60000;
    int byoyomi = _startDialog->byoyomi() * 1000;

    maru::Turn turn = maru::Sente;
    _clock->setTime(basicTime, byoyomi);
    _ponderFlag = (_players[maru::Sente].type() != _players[maru::Gote].type());
    _recorder->clear();

    if (_players[maru::Sente].type() == maru::Computer || _players[maru::Gote].type() == maru::Computer) {
        // 初期局面から開始
        Engine::instance().setStartPosition(firstPosition);
        if (!_board->startGame(firstPosition)) {
            qCritical() << "start game error" << __FILE__ << __LINE__;
        }
        _recorder->setFirstPosition(firstPosition);

        int slowMover = qBound(10, basicTime / 60000, 100); // 序盤重視率
        Engine::instance().newGame(slowMover);
    }

    _rotated = (_players[maru::Sente].type() == maru::Computer && _players[maru::Gote].type() == maru::Human);  // 反転有無
    updateBoard();
    _ui->recordWidget->clear();
    _ui->messageTableWidget->clear();

    // 対局開始
    _mode = Rating;
    _clock->start(turn);
    setTurn(turn);

    // 棋譜記録
    auto *item = new QListWidgetItem(tr("  0 Start"), _ui->recordWidget);
    _ui->recordWidget->addItem(item);
    _ui->recordWidget->scrollToItem(item);

    updateButtonStates();
    showRemainingTime(maru::Sente, basicTime, byoyomi);
    showRemainingTime(maru::Gote, basicTime, byoyomi);
}


class ResultCode : public QMap<int, QString> {
public:
    ResultCode() :
        QMap<int, QString>()
    {
        // Results
        insert(maru::Win,  QLatin1String("win"));
        insert(maru::Loss, QLatin1String("loss"));
        insert(maru::Foul, QLatin1String("foul"));
        insert(maru::Draw, QLatin1String("draw"));
        // Details
        insert(maru::Abort, QLatin1String("abort"));
        insert(maru::Win_Declare, QLatin1String("declare"));
        insert(maru::Loss_Resign, QLatin1String("resign"));
        insert(maru::Foul_OutOfTime, QLatin1String("out_of_time"));
        insert(maru::Foul_TwoPawns, QLatin1String("two_pawns"));
        insert(maru::Foul_DropPawnMate, QLatin1String("drop_pawn_mate"));
        insert(maru::Foul_OverlookedCheck, QLatin1String("overlooked_check"));
        insert(maru::Foul_PerpetualCheck, QLatin1String("perpetual_check"));
        insert(maru::Draw_Repetition, QLatin1String("repetition"));
        insert(maru::Draw_Impasse, QLatin1String("impasse"));
    }
};
Q_GLOBAL_STATIC(ResultCode, resultCode)

class ResultString : public QMap<int, QString> {
public:
    ResultString() :
        QMap<int, QString>()
    {
        // Results
        insert(maru::Win,  QObject::tr("win"));  // 勝ち
        insert(maru::Loss, QObject::tr("lose"));  // 負け
        insert(maru::Foul, QObject::tr("foul"));  // 反則
        insert(maru::Draw, QObject::tr("draw"));  // 引き分け
        // Details
        insert(maru::Abort, QObject::tr("Abort"));  // 中断
        insert(maru::Win_Declare, QObject::tr("Declare"));  // 入玉宣言
        insert(maru::Loss_Resign, QObject::tr(""));  // 投了
        insert(maru::Foul_OutOfTime, QObject::tr("Foul - out of time"));  // 反則（時間切れ）
        insert(maru::Foul_TwoPawns, QObject::tr("Foul - two pawns"));  // 反則（二歩）
        insert(maru::Foul_DropPawnMate, QObject::tr("Foul - drop pawn mate"));  // 反則（打ち歩詰め）
        insert(maru::Foul_OverlookedCheck, QObject::tr("Foul - overlooked check"));  // 反則（王手放置）
        insert(maru::Foul_PerpetualCheck, QObject::tr("Foul - perpetual check"));  // 反則（連続王手）
        insert(maru::Draw_Repetition, QObject::tr("Repetition"));  // 千日手
        insert(maru::Draw_Impasse, QObject::tr("Impasse"));  // 持将棋
    }
};
Q_GLOBAL_STATIC(ResultString, resultString)


void MainController::stopGame(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail)
{
    _clock->stop();
    _board->stopGame();
    Engine::instance().gameover();

    // 対局記録
    recordResult(turn, result, detail);

    // 評価グラフに評価値セット
    setGraphScores();

    _mode = Watch;
    updateButtonStates();
}

// レーティング計算
//  初期式　新R=旧R+((相手R-旧R)±400)/(N+1)
//  通常式　新R=旧R+((相手R-旧R)±400)/25 (25試合目以降)
static int calcRating(int current, int opposite, bool win, int gameCount)
{
    int add;
    float den = qBound(2, gameCount + 1, 25);

    // 勝利時の下限は1, 敗北時の上限は-1
    if (win) {
        add = std::max((int)std::round((opposite - current + 400) / den), 1);
    } else {
        add = std::min((int)std::round((opposite - current - 400) / den), -1);
    }

    if (gameCount >= 25) {
        // 25局目以降は -31〜+31
        add = qBound(-31, add, 31);
    }

    return std::max(current + add, 1);  // 1が下限
}


// 結果記録
void MainController::recordResult(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail)
{
    showResult(turn, result, detail);

    if (_mode == Rating) {
        int gameCount = Kifu::load().count() + 1;

        // 棋譜
        Kifu kifu;
        kifu.sfen = Engine::instance().allMoves().join(" ");
        kifu.sente = _players[maru::Sente].name();
        kifu.gote =  _players[maru::Gote].name();
        kifu.user = (_players[maru::Sente].type() == maru::Human) ? "s" : "g";
        kifu.detail = resultCode()->value(detail);

        auto &user = User::load();
        maru::Turn opp = (turn == maru::Sente) ? maru::Gote : maru::Sente;

        if (result != maru::Draw) {
            // 勝敗
            kifu.winner = ((turn == maru::Sente && result == maru::Win) || (turn == maru::Gote && (result == maru::Loss || result == maru::Foul))) ? "s" : "g";
            bool win = (kifu.user == kifu.winner);
            // レーティング
            kifu.rating = calcRating(user.rating(), _players[opp].rating(), win, gameCount);
            user.setRating(kifu.rating);
            qDebug() << "new rating:" << kifu.rating;

            if (win) {
                user.setWins(user.wins() + 1);
            } else {
                user.setLosses(user.losses() + 1);
            }

            if (result == maru::Foul) {
                user.setFouls(user.fouls() + 1);
            }
            user.save();

        } else {
            user.setDraws(user.draws() + 1);
            user.save();
        }

        // 保存
        Kifu::saveAppend(kifu);
    }
}


void MainController::setGraphScores()
{
    _graph->clear();

    int prevEval = 0;
    for (int i = 0; i < _recorder->count(); i++) {
        auto scores = _recorder->scores(i);

        if (scores.count() > 1 && !scores[1].isEmpty()) {
            prevEval = scores[1].score;
        } else {
            if (!scores[0].isEmpty() && scores[0].pv.count() > 4) {
                prevEval = scores[0].score;  // スコア更新
            }
        }
        _graph->addScore(prevEval);
    }
}


void MainController::showResult(maru::Turn turn, maru::GameResult result, maru::ResultDetail detail)
{
    QString msg;

    if (result == maru::None) {
        return;
    }

    if (result == maru::Draw) {
        // 引き分け
        msg = resultString()->value(detail);
    } else {
        // 勝負あり
        QString winner = ((turn == maru::Sente && result == maru::Win)
            || (turn == maru::Gote && (result == maru::Loss || result == maru::Foul))) ? tr("Sente") : tr("Gote");

        msg = tr("%1 win. %2").arg(winner).arg(resultString()->value(detail));
    }

    auto *item = new QListWidgetItem(msg, _ui->recordWidget);
    _ui->recordWidget->addItem(item);
    _ui->recordWidget->scrollToItem(item);
}

// 棋譜記録
//  [指し手, 王手有無]
void MainController::record(const QPair<Piece*, QString> &move, bool check)
{
    QString kif = _recorder->record(move, check);
    int count = _ui->recordWidget->count();
    QString str = QString("%1  %2").arg(count, 3).arg(kif);
    auto *item = new QListWidgetItem(str, _ui->recordWidget);
    _ui->recordWidget->addItem(item);
    _ui->recordWidget->setCurrentItem(item);
    _ui->recordWidget->scrollToItem(item);
}

void MainController::showGameoverBox(const QString &msg) const
{
    QString str = msg;
    str += "\n\n";
    str += tr("Thank you");
    MessageBox::information(tr("Game over"), str);
}


// 駒移動
void MainController::move()
{
    Sound::playSnap();  // 駒音

    // 棋譜UIへ追加
    record(_board->lastMovedPiece(), _board->isCheck());  // 指し手記録

    // 禁じ手確認
    if (isFoulMove()) {
        return;
    }

    // 手番変更
    changeTurn();
}


// 禁じ手確認 true:禁じ手or千日手  false:それ以外（対局継続）
bool MainController::isFoulMove()
{
    auto currentTurn = _clock->currentTurn();
    auto lastp = _board->lastMovedPiece();  // 駒, 元のマス

    if (lastp.first) {
        int dstCrd = lastp.first->data(maru::Coord).toInt();
        int srdCrd = lastp.second.toInt();

        if (lastp.first->name() == Piece::Pawn && (srdCrd < 11 || srdCrd > 99)) {  // 歩打つ
            // 二歩チェック
            int count = 0;
            int col = (dstCrd / 10) * 10 + 1;
            for (int c = col; c < col + 9; c++) {
                const auto *p = _board->piece(c);
                if (p->name() == Piece::Pawn && p->owner() == lastp.first->owner()) {
                    if (++count > 1) {
                        stopGame(currentTurn, maru::Foul, maru::Foul_TwoPawns);
                        showGameoverBox(tr("Foul - Two Pawns."));  // 二歩は禁じ手です。
                        return true;
                    }
                }
            }

            // 打ち歩詰めチェック
            bool check = _board->isCheck();
            if (check) {  // 王手か
                QByteArray sfen = _recorder->sfen(_recorder->count() - 1);
                bool mated = Engine::instance().mated(sfen); // その局面が詰んでいるか
                if (mated) {
                    stopGame(currentTurn, maru::Foul, maru::Foul_DropPawnMate);
                    showGameoverBox(tr("Foul - Drop Pawn Mate.")); // 打ち歩詰めは禁じ手です。
                    return true;
                }
            }
        }

        // 王手放置チェック
        auto opponent = (currentTurn == maru::Sente) ? maru::Gote : maru::Sente;
        if (!_board->searchMovablePeace(opponent, _board->kingCoord(currentTurn)).isEmpty()) {
            stopGame(currentTurn, maru::Foul, maru::Foul_OverlookedCheck);
            showGameoverBox(tr("Foul - Overlooked Check.")); // 王手放置は禁じ手です。
            return true;
        }

        // 千日手チェック
        if (_recorder->isRepetition()) {
            if (_recorder->isPerpetualCheck()) {  // 連続王手の千日手か
                // 連続王手
                stopGame(currentTurn, maru::Foul, maru::Foul_PerpetualCheck);
                showGameoverBox(tr("Foul - Perpetual Check.")); // 連続王手の千日手は禁じ手です。
                return true;
            } else {
                // 千日手
                stopGame(currentTurn, maru::Draw, maru::Draw_Repetition);
                showGameoverBox(tr("Repetition.")); // 千日手です。
                return true;
            }
        }
    }
    return false;
}


void MainController::displayTurn(maru::Turn turn)
{
    // ラベル境界線の色変更
    if (turn == maru::Sente) {
        _ui->senteFrame->setStyleSheet("border: 1px solid tomato");
        _ui->goteFrame->setStyleSheet("border: 1px solid silver");
    } else {
        _ui->senteFrame->setStyleSheet("border: 1px solid silver");
        _ui->goteFrame->setStyleSheet("border: 1px solid tomato");
    }
}


void MainController::changeTurn()
{
    // 対局時計を起点に手番変更
    auto currentTurn = _clock->changeTurn();
    setTurn(currentTurn);
}


void MainController::setTurn(maru::Turn turn)
{
    displayTurn(turn);
    _board->setTurn(turn, (_players[turn].type() == maru::Human));

    if (_mode == Rating) {
        if (_players[turn].type() == maru::Human) {
            if (_board->isCheck()) {  // 最後の手が王手か
                // 詰みチェック
                //bool mated = Engine::instance().mated(_board->toStaticSfen(_recorder->count()));
                bool mated = Engine::instance().mated(_recorder->sfen(_recorder->count() - 1));
                if (mated) {
                    // 詰み
                    stopGame(turn, maru::Loss, maru::Loss_Resign);
                    showGameoverBox(tr("Checkmate."));  // 詰みました。
                    return;
                }
            }
        }

        if (_players[turn].type() == maru::Computer) {
            Engine::instance().go(_recorder->allMoves(), _clock->remainingTime(maru::Sente), _clock->remainingTime(maru::Gote), _clock->byoyomi());
        } else {
            if (_ponderFlag) {  // 先読み
                Engine::instance().ponder(_clock->remainingTime(maru::Sente), _clock->remainingTime(maru::Gote), _clock->byoyomi());
            }
        }
    }
}

// 候補手表示（検討手があるなら検討手を、なければ読み筋を表示）
void MainController::slotPonderedItemSelected(int row, int column)
{
    if (column <= 1) {
        return;
    }

    int idx = qBound(0, _ui->recordWidget->currentRow(), _recorder->count() - 1);
    QByteArray sfen = _recorder->sfen(idx);

    ScoreItem scoreItem = _recorder->scores(idx).value(0);  // 読み筋
    if (_recorder->scores(idx).count() > row + 1) {
        // 検討手表示
        scoreItem = _recorder->scores(idx).at(row + 1);
    }

    if (scoreItem.pv.isEmpty() || column - 1 >= scoreItem.pv.count()) {
        qDebug() << "No item" << idx;
        return;
    }

    sfen += " moves ";
    sfen += scoreItem.pv.mid(0, column - 1).join(" ");
    qDebug() << sfen;
    _board->setSfen(sfen, false);
    updateButtonStates();
    updateBoard();
}


void MainController::pondered(const PonderInfo &info)
{
    // スコア更新（先手優勢ならプラス、後手優勢ならマイナス）
    auto updateScore = [](PonderInfo &pi, bool minus) {
        if (minus) {
            pi.scoreCp *= -1;
            pi.mate *= -1;
        }

        if (pi.mate) {  // 詰みあり
            pi.scoreCp = (pi.mate > 0) ? 9999: -9999;
        }
    };

    PonderInfo pi = info;

    if (_mode == Rating) {  // レーティング対局
        updateScore(pi, (_players[maru::Gote].type() == maru::Computer));  // 後手がコンピュータならマイナス

        // PonderInfoチェック
        if (!pi.pv.isEmpty()) {
            if (pi.pv[0] == "resign") {
                // 詰み
                return;
            }

            QByteArray sfen = _recorder->sfen(_recorder->count() - 1);
            sfen += " moves ";
            if (_players[_clock->currentTurn()].type() == maru::Human
                && !Engine::instance().lastPondered().isEmpty()) {
                // 先読みの場合
                pi.pv.prepend(Engine::instance().lastPondered());
            }
            sfen += pi.pv.join(' ');
            bool ok;
            Sfen::fromSfen(sfen, &ok);
            if (!ok) {
                qDebug() << "dropped:" << sfen;
                return;
            }
        }

        const ScoreItem item(pi.scoreCp, pi.mate, pi.mateCount, pi.depth, pi.nodes, pi.pv);
        _recorder->recordPonderingScore(_recorder->count() - 1, 0, item);

        if (currentPlayer().type() == maru::Human && !Engine::instance().lastPondered().isEmpty()) {
            pi.pv.prepend(Engine::instance().lastPondered());
        }
        //qDebug() << "turn:" << ((pi.turn == maru::Sente) ? "b" : "w") << "score:" << pi.scoreCp << "mateCount:" << pi.mateCount << "nodes:" << pi.nodes << "pv:" << pi.pv;

    } else if (_mode == Analyzing) {  // 棋譜検討
        // 解析終了関数
        auto stopAnalysis = [&](int moves) {
            // 表示
            auto scores = _recorder->scores(moves);
            auto sfen = _recorder->sfen(moves);
            showAnalyzingMoves(scores, sfen);
            setCurrentRecordRow(moves);
            // 終了
            Engine::instance().gameover();
            killTimer(_analysisTimerId);
            _ui->infoLine->clear();
            _mode = Watch;
            MessageBox::information(tr("Information"), tr("Analysis completed"));
            qDebug() << "Analysis completed";
            setGraphScores();
            updateButtonStates();
        };

        updateScore(pi, (_recorder->turn(_analysisMoves) == maru::Gote));  // 後手ならマイナス

        // PonderInfoチェック
        if (pi.pv.value(0) != "resign") {
            if (!pi.pv.isEmpty()) {
                auto sfen = _recorder->sfen(_analysisMoves);
                sfen += " moves ";
                sfen += pi.pv.join(' ');
                bool ok;
                Sfen::fromSfen(sfen, &ok);
                if (!ok) {
                    qDebug() << "dropped:" << sfen;
                    return;
                }
            }

            const ScoreItem item(pi.scoreCp, pi.mate, pi.mateCount, pi.depth, pi.nodes, pi.pv);
            _recorder->recordPonderingScore(_analysisMoves, pi.multipv, item);

            if (QDateTime::currentSecsSinceEpoch() > _lastPvShownTime) {  // per 1sec
                _lastPvShownTime = QDateTime::currentSecsSinceEpoch();
                auto scores = _recorder->scores(_analysisMoves);
                showAnalyzingMoves(scores, _recorder->sfen(_analysisMoves));
            }
        }

        // 最新の先読み情報
        _lastPonder = pi;

        User &user = User::load();
        if (pi.mate || pi.pv.value(0) == "resign"
            || (_elapsedTimer.elapsed() >= user.analysisTimeSeconds() * 1000 && user.analysisTimeSeconds() > 0)
            || (pi.nodes >= user.analysisNodes() && user.analysisNodes() > 0)
            || (pi.depth >= user.analysisDepth() && user.analysisDepth() > 0)) {

            // 詰みあり or 制限超過
            Engine::instance().stop();

            if (_analysisMoves < _recorder->count() - 1) {
                // 次の手解析
                while (true) {
                    auto staticsfen = _recorder->sfen(++_analysisMoves); // 次の局面へ

                    // 詰みでないかどうか
                    bool mated = Engine::instance().mated(staticsfen);
                    if (!mated) {
                        break;
                    }

                    auto turn = _recorder->turn(_analysisMoves);
                    int score = (turn == maru::Sente) ? -9999: 9999;
                    _recorder->recordPonderingScore(_analysisMoves, 1, ScoreItem(score));

                    if (_analysisMoves >= _recorder->count() - 1) {
                        // 解析終了
                        stopAnalysis(_analysisMoves);
                        return;
                    }
                }

                auto sfen = _recorder->sfenMoves(_analysisMoves);
                Engine::instance().analysis(sfen);
                setCurrentRecordRow(_analysisMoves);
                _elapsedTimer.start();

            } else {
                // 解析終了
                stopAnalysis(_analysisMoves);
                return;
            }

            // 評価グラフに評価値セット
            setGraphScores();
            updateButtonStates();
        }
    } else {
        qCritical() << "Invalid state. Dropped ponder.";
    }
}

/*!
  待った
*/
void MainController::retract()
{
    if (currentPlayer().type() != maru::Human) {
        return;
    }

    int count = _recorder->count();
    if (count < 2) {
        return;
    }

    Engine::instance().stop();

    // ２つ削除
    _ui->recordWidget->takeItem(count - 1);
    _ui->recordWidget->takeItem(count - 2);
    auto *lastItem = _ui->recordWidget->item(count - 2);
    _ui->recordWidget->scrollToItem(lastItem);

    _recorder->removeLast();
    _recorder->removeLast();
}


Player MainController::currentPlayer() const
{
    return _players[_clock->currentTurn()];
}


void MainController::updateResize(int resizeMainWindow)
{
    const QSize BaseBoardSize(600, 780);
    const float s = _boardScale / 100.0;
    QSize scaledBoardSize = BaseBoardSize * s;
    _ui->boardView->setMinimumSize(scaledBoardSize);
    _ui->boardView->setMaximumSize(scaledBoardSize);
    _ui->boardView->setTransform(QTransform(s, 0, 0, s, 0, 0));
    QSize scaledSize = BaseMainWindowSize - BaseBoardSize + scaledBoardSize;

    // メッセージリストボックスのサイズ
    int w = scaledSize.width() - 14;
    int h = _ui->messageTableWidget->size().height();
    _ui->messageTableWidget->setMinimumWidth(w);
    _ui->messageTableWidget->setMaximumWidth(w);
    _ui->messageTableWidget->resize(w, h);

    // 情報行
    h = _ui->infoLine->size().height();
    _ui->infoLine->setMinimumWidth(w);
    _ui->infoLine->setMaximumWidth(w);
    _ui->infoLine->resize(w, h);

    if (resizeMainWindow) {
        _ui->centralWidget->resize(scaledSize);  // この行で画面がちらつく
        resize(scaledSize);

        _ui->mainVLayout->update();
    }
}


void MainController::updateBoard()
{
    _ui->boardView->resetTransform();
    updateResize();

    if (_rotated) {
        _ui->boardView->rotate(180);
    }

    Piece::setRotation(_rotated);
    _board->advance();
}


void MainController::resign()
{
    if (_players[_clock->currentTurn()].type() == maru::Human) {
        MessageBox::question(tr("Resign"), tr("Resign?"), this, SLOT(slotResign(QAbstractButton*)));  // 投了しますか
    } else {
        MessageBox::question(tr("Abort"), tr("Abort?"), this, SLOT(slotResign(QAbstractButton*)));  // 中断しますか
    }
}


void MainController::slotResign(QAbstractButton *button)
{
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
    // 双方入玉している場合は持将棋とすべき（未実装）

    auto *box = dynamic_cast<QMessageBox*>(sender());
    if (box && box->buttonRole(button) == QMessageBox::AcceptRole) {
        auto result = (_players[_clock->currentTurn()].type() == maru::Human) ? maru::Loss : maru::None;
        auto detail = (_players[_clock->currentTurn()].type() == maru::Human) ? maru::Loss_Resign : maru::Abort;
        stopGame(_clock->currentTurn(), result, detail);
        showGameoverBox(tr("You resigned."));  // あなたは投了しました。
    }
}


// 回転（トグル）
void MainController::toggleRotate()
{
    _rotated = !_rotated;
    updateBoard();
}


void MainController::rotate(bool rotation)
{
    _rotated = rotation;
    updateBoard();
}

// エンジン勝ち宣言
void MainController::engineWin()
{
    stopGame(_clock->currentTurn(), maru::Win, maru::Win_Declare);
    showGameoverBox(tr("The computer declared victory by entering the king, according to CSA rules."));
}

// エンジン投了
void MainController::engineResign()
{
    stopGame(_clock->currentTurn(), maru::Loss, maru::Loss_Resign);
    showGameoverBox(tr("You win!"));
}


void MainController::slotRecordItemSelected()
{
    if (_mode == Rating) {
        _ui->messageTableWidget->clear();
        return;
    }

    int idx = qBound(0, _ui->recordWidget->currentRow(), _recorder->count() - 1);
    auto sfenstr = _recorder->sfen(idx);

    _board->setSfen(sfenstr, false, _recorder->move(idx));
    updateButtonStates();
    updateBoard();

    if (_mode == Watch) {
        auto item = _recorder->scores(idx)[0];

        // 先読み
        _ui->messageTableWidget->clear();  // クリア

        // 読み筋表示
        if (!_recorder->scores(idx).isEmpty()) {
            showAnalyzingMoves(_recorder->scores(idx), sfenstr);
            //qDebug() << "index" << idx << "sfen" << sfenstr;
        }
    }
}


void MainController::showAnalyzingMoves(const QVector<ScoreItem> &scores, const QByteArray &sfen)
{
    Sfen sf(sfen);

    // 読み筋・解析手順の表示
    auto addMessageItem = [sf](QTableWidget *messageTableWidget, int row, const QString &head, const ScoreItem &item) {
        if (item.isEmpty()) {
            return;
        }

        int col = 0;
        messageTableWidget->setItem(row, col++, new QTableWidgetItem(head));
        // 手番&スコア
        QString str;
        int score = std::abs(item.score);
        if (score > 20) {  // 一旦20以下は互角とする
            if (item.score > 0) {
                str  = QString::fromUtf8(u8"▲+");
                str += QString::number(score);
            } else  {
                str = QString::fromUtf8(u8"△+");
                str += QString::number(score);
            }
        } else {
            str = tr("Even");
        }
        messageTableWidget->setItem(row, col++, new QTableWidgetItem(str));

        // 候補手
        int colnum = std::min((int)item.pv.count(), 9);
        for (auto &move : sf.generateKif(item.pv)) {
            messageTableWidget->setItem(row, col++, new QTableWidgetItem(move));
            if (col > colnum) {
                break;
            }
        }
    };

    _ui->messageTableWidget->clear();
    if (scores.count() > 1 && !scores[1].isEmpty()) {
        // 候補手・評価値
        int rownum = std::min((int)scores.count() - 1, 5);
        for (int row = 0; row < rownum; row++) {
            const auto &it = scores[row + 1];
            QString str = (row > 0) ? QString::number(row + 1) : tr("Best");
            addMessageItem(_ui->messageTableWidget, row, str, it);
        }
    } else {
        // index:0 指し手
        if (scores.count() > 0 && !scores[0].isEmpty()) {
            // 先読み候補の表示
            addMessageItem(_ui->messageTableWidget, 0, tr("Pnd"), scores[0]);
        }
    }
}


void MainController::showRemainingTime(maru::Turn turn, int time, int byoyomi)
{
    int t = (time > 0) ? time : byoyomi;
    QString str = QTime(0, 0, 0, 999).addMSecs(t).toString("h:mm:ss");
    str += (time == 0 && byoyomi > 0) ? tr(" byoyomi") : "";  // 秒読み

    if (turn == maru::Sente) {
        _ui->senteTimeLabel->setText(str);
    } else {
        _ui->goteTimeLabel->setText(str);
    }
}


void MainController::updateRemainingTime()
{
    showRemainingTime(maru::Sente, _clock->remainingTime(maru::Sente), _clock->remainingSeconds(maru::Sente));
    showRemainingTime(maru::Gote, _clock->remainingTime(maru::Gote), _clock->remainingSeconds(maru::Gote));
}


void MainController::gameoverTimeout()
{
    if (_mode == Rating) {
        stopGame(_clock->currentTurn(), maru::Foul, maru::Foul_OutOfTime);
        showGameoverBox(tr("Out of time."));  // 時間切れです。
    }
}


void MainController::setCurrentRecordRow(int move)
{
    _ui->recordWidget->setCurrentRow(move);
}


void MainController::startAnalyzing()
{
    _analysisDialog->hide();
    User &user = User::load();

    // Check conditions
    if (user.analysisTimeSeconds() <= 0 && user.analysisNodes() <= 0 && user.analysisDepth() <= 0) {
        MessageBox::information(tr("Warning"), tr("Enter at least one condition."), _analysisDialog, SLOT(open()));
        return;
    }

    _mode = Analyzing;
    // 解析開始手数
    _analysisMoves = (_analysisDialog->scope() == AnalysisDialog::All) ? 0 : qBound(0, _ui->recordWidget->currentRow(), _recorder->count() - 1);

    // 解析開始
    if (!Engine::instance().startAnalysis()) {
        qCritical() << "Failed start engine";
        return;
    }

    auto sfen = _recorder->sfen(_analysisMoves);
    Engine::instance().analysis(sfen);
    setCurrentRecordRow(_analysisMoves);
    _analysisTimerId = startTimer(1000);
    _analysisTimer.start();
    _elapsedTimer.start();
    updateButtonStates();
    showRemainingTime(maru::Sente, 0, 0);  // 先手残り時間
    showRemainingTime(maru::Gote, 0, 0);  // 後手残り時間
}


void MainController::slotAnalysisAction()
{
    switch (_mode) {
    case Watch:
        _analysisDialog->open();
        break;

    case Analyzing:
        // 解析終了
        Engine::instance().gameover();
        killTimer(_analysisTimerId);
        _ui->infoLine->clear();
        _mode = Watch;
        MessageBox::information(tr("Information"), tr("Analysis aborted"));
        updateButtonStates();
        break;

    default:
        qWarning() << "Invalid mode" << _mode << __FILE__ << __LINE__;
        break;
    }
}

// 情報行：経過時間、解析時間、深度、ノード、NPS
void MainController::showAnalysisInfo()
{
    auto timeFormat = [](int msecs) {
        QString format =  (msecs >= 3600 * 1000) ? "H:mm:ss" : "mm:ss";
        return QTime(0, 0).addMSecs(msecs).toString(format);
    };

    // 3桁毎にカンマ区切り
    auto numberFormat = [](const QString &number) {
        QString num = number;
        for (int i = number.length() - 3; i > 0; i -= 3) {
            num.insert(i, ',');
        }
        return num;
    };

    const PonderInfo &pi = _lastPonder;
    QString str;

    str += tr("Elapsed");
    str += ' ';
    str += timeFormat(_analysisTimer.elapsed());
    str += "    ";
    str += tr("Time");
    str += ' ';
    str += timeFormat(_elapsedTimer.elapsed());
    str += "    ";
    str += tr("Depth");
    str += ' ';
    str += numberFormat(QString::number(pi.depth));
    str += "    ";
    str += tr("Nodes");
    str += ' ';
    str += numberFormat(QString::number(pi.nodes));
    str += "    ";
    str += tr("NPS");
    str += ' ';
    str += numberFormat(QString::number(pi.nps));
    _ui->infoLine->setText(str);
}


void MainController::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == _analysisTimerId) {
        // 更新
        showAnalysisInfo();
    } else {
        qDebug() << "timerEvent" << event->timerId();
    }
}
