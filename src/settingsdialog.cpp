#include "settingsdialog.h"
#include "engine.h"
#include "enginesettings.h"
#include "messagebox.h"
#include "ui_settingsdialog.h"
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
#if QT_VERSION < 0x060000
    connect(_ui->engineComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(switchEngineOptions(int)));
#else
    connect(_ui->engineComboBox, &QComboBox::currentIndexChanged, this, &SettingsDialog::switchEngineOptions);
#endif

#ifdef Q_OS_WASM
    _ui->newEngineButton->hide();
    _ui->deleteEngineButton->hide();
#endif

    // 個別オプション
    _ui->tableEngineOptions->setColumnCount(2);
    _ui->tableEngineOptions->setHorizontalHeaderItem(0, new QTableWidgetItem(QObject::tr("Option")));
    _ui->tableEngineOptions->setHorizontalHeaderItem(1, new QTableWidgetItem(QObject::tr("Value")));
    _ui->tableEngineOptions->setWordWrap(false);
    _ui->tableEngineOptions->setColumnWidth(0, 200);  // 1列目の幅
    _ui->tableEngineOptions->horizontalHeader()->setStretchLastSection(true);
}


void SettingsDialog::open()
{
    loadSettings(EngineSettings::instance().currentIndex());
    QDialog::open();
}


void SettingsDialog::loadSettings(int currentIndex)
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
    if (currentIndex >= 0 && currentIndex < _ui->engineComboBox->count()) {
        _ui->engineComboBox->setCurrentIndex(currentIndex);
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
    int row = 0;
    for (auto &key : keys) {
        int col = 0;
        auto item = new QTableWidgetItem(key);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        _ui->tableEngineOptions->setItem(row, col++, item);

        int type = engineData.types.value(key).toInt();
        auto value = options[key].toString();
        item = new QTableWidgetItem(value, type);

        if (type == QMetaType::Bool) {
            Qt::CheckState state = (value == QLatin1String("true")) ? Qt::Checked : Qt::Unchecked;
            item->setCheckState(state);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setText((state == Qt::Checked) ? tr("true") : tr("false"));
        } else {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
        _ui->tableEngineOptions->setItem(row, col++, item);
        row++;
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


void SettingsDialog::slotItemClicked(QTableWidgetItem *item)
{
    int index = _ui->engineComboBox->currentIndex();
    auto engineData = EngineSettings::instance().getEngine(index);
    auto *optionTableWidget = item->tableWidget();
    auto *optItem = optionTableWidget->item(item->row(), 0);  // Optionセル

    if (item->column() == 1) {
#ifndef Q_OS_WASM
        // クリックされた項目によってファイル選択ダイアログを表示
        if (optItem) {
            auto engineDir = QFileInfo(engineData.path).dir().absolutePath();
            QFileInfo fi(engineDir + QDir::separator() + item->text());
            QRegularExpression re("[a-z_]+Dir$");
            auto matchd = re.match(optItem->text());  // オプション名

            if (matchd.hasMatch()) {
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
        QString str = optItem->text() + "\n";
        str += tr("Default");
        str += " ";
        str += option.value.toString();
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
            engineData.options.insert(it.key(), it.value().value);
            qDebug() << it.key() << it.value().value;
        }
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
            if (optItem && !optItem->text().isEmpty() && valItem && !valItem->text().isEmpty()) {
                if (valItem->type() == QMetaType::Bool) {
                    bool check = (valItem->checkState() == Qt::Checked);
                    options.insert(optItem->text(), check);  // boolean
                } else {
                    options.insert(optItem->text(), valItem->text());
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
}
