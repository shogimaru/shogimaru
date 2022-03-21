#include "settingsdialog.h"
#include "engine.h"
#include "enginesettings.h"
#include "messagebox.h"
#include "ui_settingsdialog.h"
#include "user.h"
#include "westerntabstyle.h"
#include <QFileDialog>
#include <QTabBar>
#include <QtCore>

static WesternTabStyle westernTabStyle;


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog)
{
    _ui->setupUi(this);
    _ui->tabWidget->tabBar()->setStyle(&westernTabStyle);
    _ui->tabWidget->setCurrentIndex(0);

    connect(this, &SettingsDialog::finished, this, &SettingsDialog::save);
    connect(_ui->closeButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(_ui->newEngineButton, &QPushButton::clicked, this, &SettingsDialog::getEnginePath);
    connect(_ui->deleteEngineButton, &QPushButton::clicked, this, &SettingsDialog::confirmDelete);
    connect(_ui->resetEngineOptButton, &QPushButton::clicked, this, &SettingsDialog::resetEngineOptions);
    connect(_ui->tableEngineOptions, &QTableWidget::itemClicked, this, &SettingsDialog::slotItemClicked);
    connect(_ui->tableEngineOptions, &QTableWidget::currentItemChanged, this, &SettingsDialog::slotItemClicked);
#if QT_VERSION < 0x060000
    connect(_ui->engineComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(switchEngineOptions(int)));
#else
    connect(_ui->engineComboBox, &QComboBox::currentIndexChanged, this, &SettingsDialog::switchEngineOptions);
#endif
    connect(_ui->soundOnOffButton, &QToolButton::toggled, [this](bool checked) {
        auto text = (checked) ? tr("ON") : tr("OFF");
        _ui->soundOnOffButton->setText(text);
    });

#ifdef Q_OS_WASM
    _ui->newEngineButton->hide();
    _ui->deleteEngineButton->hide();
#endif

    // 個別オプション
    _ui->tableEngineOptions->verticalHeader()->hide();
    _ui->tableEngineOptions->setStyleSheet("QHeaderView::section {border-style: solid; border-width: 0 1px 1px 0; border-color: #ddd;}");
    _ui->tableEngineOptions->setColumnCount(2);
    _ui->tableEngineOptions->setHorizontalHeaderItem(0, new QTableWidgetItem(QObject::tr("Option")));
    _ui->tableEngineOptions->setHorizontalHeaderItem(1, new QTableWidgetItem(QObject::tr("Value")));
    _ui->tableEngineOptions->setWordWrap(false);
    _ui->tableEngineOptions->setColumnWidth(0, 200);  // 1列目の幅
    _ui->tableEngineOptions->horizontalHeader()->setStretchLastSection(true);

    // 駒種類選択ラジオボタン
    _ui->buttonGroup->setId(_ui->radioPiece1, 1);
    _ui->buttonGroup->setId(_ui->radioPiece2, 2);
}


void SettingsDialog::open()
{
    loadSettings(EngineSettings::instance().currentIndex());
    QDialog::open();
}


void SettingsDialog::loadSettings(int engineIndex)
{
    // エンジン設定読込
    const auto &availableEngines = EngineSettings::instance().availableEngines();
    _ui->engineComboBox->blockSignals(true);  // シグナル無効化
    _ui->engineComboBox->clear();
    for (const auto &engine : availableEngines) {
        QString name = engine.name;
        if (!engine.author.isEmpty()) {
            name += " (";
            name += engine.author;
            name += ")";
        }
        _ui->engineComboBox->addItem(name);
    }
    _ui->engineComboBox->setCurrentIndex(-1);  // 次のsetCurrentIndexでシグナルが飛ぶように
    _ui->engineComboBox->blockSignals(false);  // シグナル有効化

    // 現在インデックスの設定
    if (engineIndex >= 0 && engineIndex < _ui->engineComboBox->count()) {
        _ui->engineComboBox->setCurrentIndex(engineIndex);
    }

    auto &user = User::load();
    _ui->soundOnOffButton->setChecked(user.soundEnable());  // サウンド
    auto *button = _ui->buttonGroup->button(user.pieceType());
    if (button) {
        button->setChecked(true);  // 駒種類
    }
}


void SettingsDialog::setCurrentEngine(int index)
{
    const auto &availableEngines = EngineSettings::instance().availableEngines();

    if (index < 0 || index >= availableEngines.count()) {
        return;
    }
    _ui->engineComboBox->setCurrentIndex(index);
}


void SettingsDialog::switchEngineOptions(int index)
{
    const auto &availableEngines = EngineSettings::instance().availableEngines();
    if (index < 0 || index >= availableEngines.count()) {
        _ui->tableEngineOptions->clearContents();
        return;
    }
    // 切り替える前のエンジンオプションを反映させる
    updateEngineOptions(prevIndex);
    // 設定表示
    showEngineOptions(index);
}


void SettingsDialog::showEngineOptions(int index)
{
    // 該当するエンジン設定を取得
    const auto &availableEngines = EngineSettings::instance().availableEngines();

    if (index < 0 || index >= availableEngines.count()) {
        _ui->tableEngineOptions->clearContents();
        return;
    }

    // エンジンオプション
    const auto &engineData = availableEngines[index];
    auto info = Engine::getEngineInfo(engineData.path);
    _defaultOptions = info.options;  // デフォルトオプション取得
    auto options = engineData.options;

    _ui->tableEngineOptions->clearContents();
    _ui->tableEngineOptions->setRowCount(options.count());

    QStringList keys = options.keys();
    keys.sort(Qt::CaseInsensitive);  // ソート
    int row = -1;
    for (auto &key : keys) {
        row++;
        auto item = new QTableWidgetItem(key);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        _ui->tableEngineOptions->setItem(row, 0, item);

        int type = engineData.types.value(key).toInt();
        auto value = options[key].toString();
        item = new QTableWidgetItem(value, type);

        if (type == QMetaType::Bool) {
            Qt::CheckState state = (value == QLatin1String("true")) ? Qt::Checked : Qt::Unchecked;
            item->setCheckState(state);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setText((state == Qt::Checked) ? tr("true") : tr("false"));
        } else if (type == QMetaType::QStringList) {
            // コンボボックス
            delete item;
            auto variables = _defaultOptions[key].value.toStringList();  // 選択肢
            QComboBox *combo = new QComboBox;
            for (auto &var : variables) {
                combo->addItem(var);
            }
            combo->setCurrentText(value);
            _ui->tableEngineOptions->setCellWidget(row, 1, combo);
            continue;
        } else {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
        _ui->tableEngineOptions->setItem(row, 1, item);
    }

    // オプション情報非表示
    _ui->labelEngineOptInfo->setText("");
    prevIndex = index;
}


void SettingsDialog::accept()
{
    QDialog::accept();
}


void SettingsDialog::getEnginePath()
{
#ifdef Q_OS_WIN
    QString filter = QObject::tr("Executable (*.exe)");
#else
    QString filter = QObject::tr("Executable (*)");
#endif

    QString path = QFileDialog::getOpenFileName(this, QObject::tr("Select Engine"), QString(), filter);

    if (path.trimmed().isEmpty()) {
        return;
    }

    QFileInfo fi(path);
    if (!fi.isExecutable()) {
        MessageBox::information(tr("Invalid file"), tr("File not executable"));
        return;
    }

    // エンジン追加
    auto info = Engine::getEngineInfo(path);
    EngineSettings::EngineData newEngine;
    newEngine.name = info.name;
    newEngine.author = info.author;
    newEngine.path = path;
    for (auto it = info.options.begin(); it != info.options.end(); ++it) {
        newEngine.options.insert(it.key(), it.value().value);
        newEngine.types.insert(it.key(), QVariant((int)it.value().type));
    }

    EngineSettings::setCustomOptions(newEngine.options);
    EngineSettings::instance().addEngine(newEngine);
    _defaultOptions.clear();
    int newidx = EngineSettings::instance().availableEngineCount() - 1;
    loadSettings(newidx);
}


void SettingsDialog::confirmDelete()
{
    if (EngineSettings::instance().availableEngineCount() > 0 && _ui->engineComboBox->currentIndex() >= 0) {
        QString text = tr("Delete %1.\nAre you sure?").arg(_ui->engineComboBox->currentText());
        MessageBox::question(tr("Delete engine"), text, [this]() {
            // OKの場合
            deleteEngine();
        });
    }
}


void SettingsDialog::deleteEngine()
{
    // 削除
    int idx = _ui->engineComboBox->currentIndex();
    if (idx >= 0 && idx < EngineSettings::instance().availableEngineCount()) {
        EngineSettings::instance().removeEngine(idx);
        _ui->engineComboBox->removeItem(idx);
        _ui->tableEngineOptions->clearContents();

        // 次のエンジン選択
        idx = std::min(idx, EngineSettings::instance().availableEngineCount() - 1);
        _ui->engineComboBox->setCurrentIndex(-1);  // シグナルを飛ばすため
        _ui->engineComboBox->setCurrentIndex(idx);
    }
}

// オプションの日本語説明
static QString jpText(const QString &name)
{
    static QMap<QString, QString> jpTextMap = {
        // やねうら王
        {QLatin1String("Threads"), QString::fromUtf8(u8"スレッド数")},
        {QLatin1String("USI_Hash"), QString::fromUtf8(u8"置換表のサイズ(MB)")},
        {QLatin1String("USI_Ponder"), QString::fromUtf8(u8"相手番の時にエンジンが思考するフラグ")},
        {QLatin1String("Stochastic_Ponder"), QString::fromUtf8(u8"先読み(Ponder)が有効な場合に相手局面を探索するフラグ")},
        {QLatin1String("MultiPV"), QString::fromUtf8(u8"読み筋(PV)の数（5件まで表示可）")},
        {QLatin1String("NetworkDelay"), QString::fromUtf8(u8"通信時の平均遅延時間（ミリ秒）")},
        {QLatin1String("NetworkDelay2"), QString::fromUtf8(u8"通信時の最大遅延時間（ミリ秒）")},
        {QLatin1String("MinimumThinkingTime"), QString::fromUtf8(u8"最小思考時間（ミリ秒）")},
        {QLatin1String("SlowMover"), QString::fromUtf8(u8"序盤重視率(%) （対局では自動的に値が設定される）")},
        {QLatin1String("MaxMovesToDraw"), QString::fromUtf8(u8"終局までの手数（0は無制限）")},
        {QLatin1String("DepthLimit"), QString::fromUtf8(u8"探索深さの制限（0は無制限）")},
        {QLatin1String("NodesLimit"), QString::fromUtf8(u8"探索ノード数の制限（0は無制限）")},
#ifdef Q_OS_WIN
        {QLatin1String("EvalDir"), QString::fromUtf8(u8"評価関数用ファイルの格納フォルダ")},
        {QLatin1String("BookDir"), QString::fromUtf8(u8"定跡ファイルの格納フォルダ")},
#else
        {QLatin1String("EvalDir"), QString::fromUtf8(u8"評価関数用ファイルの格納ディレクトリ")},
        {QLatin1String("BookDir"), QString::fromUtf8(u8"定跡ファイルの格納ディレクトリ")},
#endif
        {QLatin1String("WriteDebugLog"), QString::fromUtf8(u8"標準入出力の出力先ファイル")},
        {QLatin1String("GenerateAllLegalMoves"), QString::fromUtf8(u8"全ての合法手を生成するフラグ")},
        {QLatin1String("EnteringKingRule"), QString::fromUtf8(u8"入玉ルール")},
        {QLatin1String("USI_OwnBook"), QString::fromUtf8(u8"定跡を有効にするフラグ")},
        {QLatin1String("NarrowBook"), QString::fromUtf8(u8"出現率が低い定跡を除外するフラグ")},
        {QLatin1String("BookMoves"), QString::fromUtf8(u8"定跡を用いる手数（0は使用しない）")},
        {QLatin1String("BookIgnoreRate"), QString::fromUtf8(u8"定跡を無視する確率(%)")},
        {QLatin1String("BookFile"), QString::fromUtf8(u8"定跡ファイル")},
        {QLatin1String("BookEvalDiff"), QString::fromUtf8(u8"定跡の第一候補手との評価値の差")},
        {QLatin1String("BookEvalBlackLimit"), QString::fromUtf8(u8"定跡の先手の評価値下限")},
        {QLatin1String("BookEvalWhiteLimit"), QString::fromUtf8(u8"定跡の後手の評価値下限")},
        {QLatin1String("BookDepthLimit"), QString::fromUtf8(u8"定跡の深さ下限")},
        {QLatin1String("BookOnTheFly"), QString::fromUtf8(u8"定跡ファイルを全てメモリに置かないフラグ")},
        {QLatin1String("ConsiderBookMoveCount"), QString::fromUtf8(u8"定跡の指し手を定跡DBの採択率に比例させるフラグ")},
        {QLatin1String("BookPvMoves"), QString::fromUtf8(u8"定跡にヒットしたときに表示する読み筋の手数")},
        {QLatin1String("IgnoreBookPly"), QString::fromUtf8(u8"定跡DB上の手数を無視するフラグ")},
        {QLatin1String("SkillLevel"), QString::fromUtf8(u8"強さレベル（対局では自動的に値が設定される）")},
        {QLatin1String("DrawValueBlack"), QString::fromUtf8(u8"探索開始局面が先手番で 引き分けの時のスコア")},
        {QLatin1String("DrawValueWhite"), QString::fromUtf8(u8"探索開始局面が後手番で 引き分けの時のスコア")},
        {QLatin1String("PvInterval"), QString::fromUtf8(u8"読み筋の出力間隔（ミリ秒, 0なら出力しない）")},
        {QLatin1String("ResignValue"), QString::fromUtf8(u8"投了スコア")},
        {QLatin1String("ConsiderationMode"), QString::fromUtf8(u8"中途半端な読み筋を非表示にするフラグ")},
        {QLatin1String("OutputFailLHPV"), QString::fromUtf8(u8"fail low/highの時に読み筋を出力するフラグ")},
        {QLatin1String("FV_SCALE"), QString::fromUtf8(u8"評価値を算出するための除数（割り算の分母）")},
        // dlshogi
        {QLatin1String("Best_Book_Move"), QString::fromUtf8(u8"最善定跡使用フラグ")},
        {QLatin1String("Book_File"), QString::fromUtf8(u8"定跡ファイル")},
        {QLatin1String("Byoyomi_Margin"), QString::fromUtf8(u8"秒読み時の遅延時間（ミリ秒）")},
        {QLatin1String("C_base"), QString::fromUtf8(u8"探索パラメータ")},
        {QLatin1String("C_base_root"), QString::fromUtf8(u8"探索パラメータ")},
        {QLatin1String("C_fpu_reduction"), QString::fromUtf8(u8"探索パラメータ")},
        {QLatin1String("C_fpu_reduction_root"), QString::fromUtf8(u8"探索パラメータ")},
        {QLatin1String("C_init"), QString::fromUtf8(u8"探索パラメータ")},
        {QLatin1String("C_init_root"), QString::fromUtf8(u8"探索パラメータ")},
        {QLatin1String("Const_Playout"), QString::fromUtf8(u8"1手あたりのプレイアウト数")},
        {QLatin1String("DebugMessage"), QString::fromUtf8(u8"")},
        {QLatin1String("DfPn_Hash"), QString::fromUtf8(u8"df-pn置換表のサイズ")},
        {QLatin1String("DfPn_Min_Search_Millisecs"), QString::fromUtf8(u8"df-pn最小探索時間（ミリ秒）")},
        {QLatin1String("DNN_Batch_Size"), QString::fromUtf8(u8"バッチサイズ")},
        {QLatin1String("DNN_Batch_Size2"), QString::fromUtf8(u8"バッチサイズ")},
        {QLatin1String("DNN_Batch_Size3"), QString::fromUtf8(u8"バッチサイズ")},
        {QLatin1String("DNN_Batch_Size4"), QString::fromUtf8(u8"バッチサイズ")},
        {QLatin1String("DNN_Batch_Size5"), QString::fromUtf8(u8"バッチサイズ")},
        {QLatin1String("DNN_Batch_Size6"), QString::fromUtf8(u8"バッチサイズ")},
        {QLatin1String("DNN_Batch_Size7"), QString::fromUtf8(u8"バッチサイズ")},
        {QLatin1String("DNN_Batch_Size8"), QString::fromUtf8(u8"バッチサイズ")},
        {QLatin1String("DNN_Model"), QString::fromUtf8(u8"モデルファイル")},
        {QLatin1String("DNN_Model2"), QString::fromUtf8(u8"モデルファイル")},
        {QLatin1String("DNN_Model3"), QString::fromUtf8(u8"モデルファイル")},
        {QLatin1String("DNN_Model4"), QString::fromUtf8(u8"モデルファイル")},
        {QLatin1String("DNN_Model5"), QString::fromUtf8(u8"モデルファイル")},
        {QLatin1String("DNN_Model6"), QString::fromUtf8(u8"モデルファイル")},
        {QLatin1String("DNN_Model7"), QString::fromUtf8(u8"モデルファイル")},
        {QLatin1String("DNN_Model8"), QString::fromUtf8(u8"モデルファイル")},
        {QLatin1String("Draw_Ply"), QString::fromUtf8(u8"")},
        {QLatin1String("Draw_Value_Black"), QString::fromUtf8(u8"")},
        {QLatin1String("Draw_Value_White"), QString::fromUtf8(u8"")},
        {QLatin1String("Engine_Name"), QString::fromUtf8(u8"エンジン名")},
        {QLatin1String("Mate_Root_Search"), QString::fromUtf8(u8"詰み探索の深さ制限値")},
        {QLatin1String("Min_Book_Score"), QString::fromUtf8(u8"")},
        {QLatin1String("OwnBook"), QString::fromUtf8(u8"定跡使用フラグ")},
        {QLatin1String("PV_Interval"), QString::fromUtf8(u8"読み筋の出力間隔（ミリ秒, 0なら出力しない）")},
        {QLatin1String("Resign_Threshold"), QString::fromUtf8(u8"投了する勝率の閾値")},
        {QLatin1String("ReuseSubtree"), QString::fromUtf8(u8"")},
        {QLatin1String("Softmax_Temperature"), QString::fromUtf8(u8"探索のソフトマックス温度")},
        {QLatin1String("Time_Margin"), QString::fromUtf8(u8"遅延時間（ミリ秒）")},
        {QLatin1String("UCT_NodeLimit"), QString::fromUtf8(u8"最大探索ノード数")},
        {QLatin1String("UCT_Threads"), QString::fromUtf8(u8"GPU1枚あたりの探索のスレッド数")},
        {QLatin1String("UCT_Threads2"), QString::fromUtf8(u8"GPU1枚あたりの探索のスレッド数")},
        {QLatin1String("UCT_Threads3"), QString::fromUtf8(u8"GPU1枚あたりの探索のスレッド数")},
        {QLatin1String("UCT_Threads4"), QString::fromUtf8(u8"GPU1枚あたりの探索のスレッド数")},
        {QLatin1String("UCT_Threads5"), QString::fromUtf8(u8"GPU1枚あたりの探索のスレッド数")},
        {QLatin1String("UCT_Threads6"), QString::fromUtf8(u8"GPU1枚あたりの探索のスレッド数")},
        {QLatin1String("UCT_Threads7"), QString::fromUtf8(u8"GPU1枚あたりの探索のスレッド数")},
        {QLatin1String("UCT_Threads8"), QString::fromUtf8(u8"GPU1枚あたりの探索のスレッド数")},
        // Apery
        {QLatin1String("Book_Enable"), QString::fromUtf8(u8"定跡使用フラグ")},
#ifdef Q_OS_WIN
        {QLatin1String("Eval_Dir"), QString::fromUtf8(u8"評価関数用ファイルの格納フォルダ")},
#else
        {QLatin1String("Eval_Dir"), QString::fromUtf8(u8"評価関数用ファイルの格納ディレクトリ")},
#endif
        {QLatin1String("Eval_Hash"), QString::fromUtf8(u8"評価用置換表のサイズ(MB)")},
        {QLatin1String("Slow_Mover"), QString::fromUtf8(u8"序盤重視率(%)")},
    };
    return jpTextMap.value(name);
}


void SettingsDialog::slotItemClicked(QTableWidgetItem *item)
{
    if (!item) {
        return;
    }

    int index = _ui->engineComboBox->currentIndex();
    auto engineData = EngineSettings::instance().getEngine(index);
    auto *optionTableWidget = item->tableWidget();
    auto *optItem = optionTableWidget->item(item->row(), 0);  // Optionセル

    if (item->column() == 1) {
#ifndef Q_OS_WASM
        // クリックされた項目によってファイル選択ダイアログを表示
        if (optItem) {
            auto engineDir = QFileInfo(engineData.path).dir().absolutePath();
            QRegularExpression re("[a-z_]+Dir$");
            auto matchd = re.match(optItem->text());  // オプション名
            QFileInfo fi(item->text());

            if (matchd.hasMatch()) {
                if (!fi.isDir() || !fi.exists()) {
                    fi.setFile(engineDir, item->text());
                }
                QString path = (fi.isDir() && fi.exists()) ? fi.absoluteFilePath() : engineDir;
                QString dir = QFileDialog::getExistingDirectory(this, QObject::tr("Select Directory"), path);
                if (!dir.isEmpty()) {
                    item->setText(dir);
                }
            } else {
                int type = engineData.types.value(optItem->text()).toInt();
                QRegularExpression refile("[a-z_]+File$");
                auto matchf = refile.match(optItem->text());  // オプション名

                if (type == QMetaType::QUrl || matchf.hasMatch()) {
                    // ファイル読込
                    if (!fi.isFile() || !fi.exists()) {
                        fi.setFile(engineDir, item->text());
                    }

                    QString path = fi.dir().exists() ? fi.dir().absolutePath() : engineDir;
                    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), path, "*");
                    if (!fileName.isEmpty()) {
                        item->setText(fileName);
                    }
                }
            }
        }
#endif

        if (item->type() == QMetaType::Bool) {
            // チェックステートを変更
            auto state = item->checkState();
            item->setText((state == Qt::Checked) ? tr("true") : tr("false"));
        } else {
            optionTableWidget->editItem(item);  // シングルクリックで編集モードにする
        }
    }

    if (optItem) {
        auto option = _defaultOptions.value(optItem->text());
        QString str = optItem->text();
        if (maru::isLocaleLangJapanese()) {
            QString jp = jpText(optItem->text());
            if (!jp.isEmpty()) {
                str = jp;
            }
        }
        str += "\n";
        str += tr("Default");
        str += " ";
        if (option.type == QMetaType::Bool) {
            str += (option.value.toBool()) ? tr("true") : tr("false");
        } else if (option.type == QMetaType::QStringList) {
            str += option.value.toStringList().value(0);
        } else {
            str += option.value.toString();
        }

        if (option.type == QMetaType::LongLong) {
            str += "  ";
            str += tr("Max");
            str += " ";
            str += QString::number(option.max);
            str += "  ";
            str += tr("Min");
            str += " ";
            str += QString::number(option.min);
        }
        _ui->labelEngineOptInfo->setText(str);
    }
}


void SettingsDialog::resetEngineOptions()
{
    MessageBox::question(tr("Reset option"), tr("Reset the engine options.\nAre you sure?"), [this]() {
        int index = _ui->engineComboBox->currentIndex();
        auto engineData = EngineSettings::instance().getEngine(index);
        engineData.options.clear();
        for (auto it = _defaultOptions.begin(); it != _defaultOptions.end(); ++it) {
            if (it.value().type == QMetaType::QStringList) {
                // コンボボックス
                QString def = it.value().value.toStringList().value(0);
                engineData.options.insert(it.key(), def);
                //qDebug() << it.key() << def;
            } else {
                engineData.options.insert(it.key(), it.value().value);
                //qDebug() << it.key() << it.value().value;
            }
        }
        EngineSettings::setCustomOptions(engineData.options);
        EngineSettings::instance().updateEngine(index, engineData);
        showEngineOptions(index);
    });
}


void SettingsDialog::updateEngineOptions(int index)
{
    if (index >= 0 && index < EngineSettings::instance().availableEngineCount()) {
        QVariantMap options;  // 画面の設定取得
        for (int i = 0; i < _ui->tableEngineOptions->rowCount(); i++) {
            auto *optItem = _ui->tableEngineOptions->item(i, 0);
            auto *valItem = _ui->tableEngineOptions->item(i, 1);
            if (!optItem || optItem->text().isEmpty()) {
                continue;
            }

            if (valItem && !valItem->text().isEmpty()) {
                if (valItem->type() == QMetaType::Bool) {
                    bool check = (valItem->checkState() == Qt::Checked);
                    options.insert(optItem->text(), check);  // boolean
                } else {
                    options.insert(optItem->text(), valItem->text());
                }
            } else {
                // コンボボックス
                auto *widget = _ui->tableEngineOptions->cellWidget(i, 1);
                auto *combo = dynamic_cast<QComboBox *>(widget);
                if (combo) {
                    options.insert(optItem->text(), combo->currentText());
                }
            }
        }

        // 設定に反映
        if (options.count() > 0) {
            auto engine = EngineSettings::instance().getEngine(index);
            engine.options = options;
            EngineSettings::instance().updateEngine(index, engine);
        }
    }
}


void SettingsDialog::save()
{
    // オプション反映
    int index = _ui->engineComboBox->currentIndex();
    updateEngineOptions(index);
    EngineSettings::instance().setCurrentIndex(index);
    EngineSettings::instance().save();

    auto &user = User::load();
    user.setSoundEnable(_ui->soundOnOffButton->isChecked());  // サウンド
    user.setPieceType(_ui->buttonGroup->checkedId());  // 駒種類
    user.save();
}
