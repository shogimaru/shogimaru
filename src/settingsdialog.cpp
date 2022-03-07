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
    //connect(_ui->deleteEngineOptButton, &QPushButton::clicked, this, &SettingsDialog::deleteEngineOption);
    connect(_ui->resetEngineOptButton, &QPushButton::clicked, this, &SettingsDialog::resetEngineOptions);
    //connect(_ui->deleteGeneralOptButton, &QPushButton::clicked, this, &SettingsDialog::deleteGeneralOption);
    connect(_ui->tableEngineOptions, &QTableWidget::itemClicked, this, &SettingsDialog::slotItemClicked);
    connect(_ui->tableGeneralOptions, &QTableWidget::itemClicked, this, &SettingsDialog::slotItemClicked);
#if QT_VERSION < 0x060000
    connect(_ui->engineComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(showEngineOptions(int)));
#else
    connect(_ui->engineComboBox, &QComboBox::currentIndexChanged, this, &SettingsDialog::showEngineOptions);
#endif

#ifdef Q_OS_WASM
    _ui->tabWidget->removeTab(0);  // WASMではエンジン設定を非表示
#else
    _ui->tabWidget->removeTab(1);  // デスクトップ版ではオプション設定を非表示
#endif

    // 個別オプション
    _ui->tableEngineOptions->setColumnCount(2);
    _ui->tableEngineOptions->setHorizontalHeaderItem(0, new QTableWidgetItem(QObject::tr("Option")));
    _ui->tableEngineOptions->setHorizontalHeaderItem(1, new QTableWidgetItem(QObject::tr("Value")));
    _ui->tableEngineOptions->setWordWrap(false);
    _ui->tableEngineOptions->setColumnWidth(0, 200);  // 1列目の幅
    _ui->tableEngineOptions->horizontalHeader()->setStretchLastSection(true);

    // 共通オプション
    _ui->tableGeneralOptions->setColumnCount(2);
    _ui->tableGeneralOptions->setHorizontalHeaderItem(0, new QTableWidgetItem(QObject::tr("Option")));
    _ui->tableGeneralOptions->setHorizontalHeaderItem(1, new QTableWidgetItem(QObject::tr("Value")));
    _ui->tableGeneralOptions->setWordWrap(false);
    _ui->tableGeneralOptions->setColumnWidth(0, 200);  // 1列目の幅
    _ui->tableGeneralOptions->horizontalHeader()->setStretchLastSection(true);
}


void SettingsDialog::open()
{
    loadSettings();
    QDialog::open();
}


void SettingsDialog::loadSettings()
{
    // エンジン設定
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

    if (_ui->engineComboBox->count() > 0) {
        _ui->engineComboBox->setCurrentIndex(EngineSettings::instance().currentIndex());
    }

    // 共通オプション
    const auto &generalOptions = EngineSettings::instance().generalOptions();
    _ui->tableGeneralOptions->clearContents();
    _ui->tableGeneralOptions->setRowCount(generalOptions.count());
    int row = 0;
    for (auto it = generalOptions.begin(); it != generalOptions.end(); ++it) {
        int col = 0;
        auto item = new QTableWidgetItem(it.key());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        _ui->tableGeneralOptions->setItem(row, col++, item);
        item = new QTableWidgetItem(it.value().toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        _ui->tableGeneralOptions->setItem(row, col++, item);
        row++;
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


void SettingsDialog::showEngineOptions(int index)
{
    const auto &availableEngines = EngineSettings::instance().availableEngines();

    if (index < 0 || index >= availableEngines.count()) {
        _ui->tableEngineOptions->clearContents();
        return;
    }
    // 切り替える前のエンジンオプションを反映させる
    updateEngineOptions(EngineSettings::instance().currentIndex());

    // エンジンオプション
    const auto &options = availableEngines[index].options;
    _ui->tableEngineOptions->clearContents();
    _ui->tableEngineOptions->setRowCount(options.count());

    int row = 0;
    for (auto it = options.begin(); it != options.end(); ++it) {
        int col = 0;
        auto item = new QTableWidgetItem(it.key());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        _ui->tableEngineOptions->setItem(row, col++, item);
        item = new QTableWidgetItem(it.value().toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        _ui->tableEngineOptions->setItem(row, col++, item);
        row++;
    }
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

    auto info = Engine::getEngineInfo(path);

    // エンジン追加
    EngineSettings::EngineData data;
    data.name = info.name;
    data.author = info.author;
    data.path = path;
    data.defaultOptions = [](const QMap<QString, Engine::Option> &options) {
        QVariantMap map;
        for (auto it = options.begin(); it != options.end(); ++it) {
            map.insert(it.key(), it.value().value);
        }
        return map;
    }(info.options);
    data.options = data.defaultOptions;  // デフォルトオプションを設定する

    EngineSettings::instance().addEngine(data);
    int newidx = EngineSettings::instance().availableEngineCount() - 1;
    EngineSettings::instance().setCurrentIndex(newidx);
    loadSettings();
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
    auto *optionTableWidget = item->tableWidget();
    if (item->column() == 1) {
        auto *optItem = optionTableWidget->item(item->row(), 0);  // Optionセル
        if (optItem) {
            QRegularExpression re("[a-z_]+Dir$");
            auto match = re.match(optItem->text());
            if (match.hasMatch()) {
                QString dir = QFileDialog::getExistingDirectory(this, QObject::tr("Open Directory"), QString());
                if (!dir.isEmpty()) {
                    item->setText(dir);
                }
            }
        }
    }
    optionTableWidget->editItem(item);  // シングルクリックで編集モードにする
}


// void SettingsDialog::deleteEngineOption()
// {
//     const QStringList excludes = {QLatin1String("EvalDir"), QLatin1String("BookDir")};
//     deleteOption(_ui->tableEngineOptions, excludes);
// }


// void SettingsDialog::deleteGeneralOption()
// {
//     deleteOption(_ui->tableGeneralOptions);
// }


// void SettingsDialog::deleteOption(QTableWidget *tableWidget, const QStringList &excludes)
// {
//     // エンジンオプション or 共通オプションのエントリ削除
//     if (!tableWidget) {
//         return;
//     }

//     int row = tableWidget->currentRow();
//     if (row < 0) {
//         return;
//     }

//     auto *item = tableWidget->item(row, 0);
//     if (!item || item->text().trimmed().isEmpty()) {
//         item = tableWidget->item(row, 1);
//         if (!item || item->text().trimmed().isEmpty()) {
//             // Option&Valueが空の場合
//             return;
//         }
//     } else {
//         QString option = item->text().trimmed();
//         if (excludes.contains(option)) {
//             QString msg = tr("%1 option cannot be deleted.").arg(option);
//             MessageBox::information("Warning", msg);
//             return;
//         }
//     }

//     int col = tableWidget->currentColumn();
//     tableWidget->removeRow(row);
//     tableWidget->setCurrentCell(row, col);
// }


void SettingsDialog::resetEngineOptions()
{
    MessageBox::question(tr("Reset option"), tr("Reset the engine options.\nAre you sure?"), [this]() {

    });
}


void SettingsDialog::updateEngineOptions(int index)
{
    if (index >= 0 && index < EngineSettings::instance().availableEngineCount()) {
        QVariantMap options;  // 画面の設定取得
        for (int i = 0; i < _ui->tableEngineOptions->rowCount(); i++) {
            auto *opt = _ui->tableEngineOptions->item(i, 0);
            auto *val = _ui->tableEngineOptions->item(i, 1);
            if (opt && !opt->text().isEmpty() && val && !val->text().isEmpty()) {
                options.insert(opt->text(), val->text());
            }
        }

        if (options.count() > 0) {
            auto engine = EngineSettings::instance().getEngine(index);
            engine.options = options;
            EngineSettings::instance().updateEngine(index, engine);
            //qDebug() << options;
        }
    }
}


void SettingsDialog::save()
{
    // 共通オプション反映
    auto &generalOptions = EngineSettings::instance().generalOptions();
    generalOptions.clear();

    for (int i = 0; i < _ui->tableGeneralOptions->rowCount(); i++) {
        auto *opt = _ui->tableGeneralOptions->item(i, 0);
        auto *val = _ui->tableGeneralOptions->item(i, 1);
        if (opt && !opt->text().isEmpty() && val && !val->text().isEmpty()) {
            generalOptions.insert(opt->text(), val->text());
        }
    }

    int index = _ui->engineComboBox->currentIndex();
    updateEngineOptions(index);
    EngineSettings::instance().setCurrentIndex(index);
    EngineSettings::instance().save();
    //qDebug() << "EngineSettings::instance().save()";
}
