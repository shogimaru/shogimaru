#include "settingsdialog.h"
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

    connect(_ui->closeButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(_ui->newEngineButton, &QPushButton::clicked, this, &SettingsDialog::getEnginePath);
    connect(_ui->deleteEngineButton, &QPushButton::clicked, this, &SettingsDialog::confirmDelete);
#if QT_VERSION < 0x060000
    connect(_ui->engineComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(showEngineOptions(int)));
#else
    connect(_ui->engineComboBox, &QComboBox::currentIndexChanged, this, &SettingsDialog::showEngineOptions);
#endif

#ifdef Q_OS_WASM
    _ui->tabWidget->removeTab(0);  // WASMではエンジン設定を非表示
    //_ui->tabEngine->deleteLater();
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
    // エンジン設定
    const auto &availableEngines = EngineSettings::instance().availableEngines();
    for (const auto &engine : availableEngines) {
        _ui->engineComboBox->addItem(engine.name);
    }

    showEngineOptions(EngineSettings::instance().selectedEngine());

    // 共通オプション
    const auto &generalOptions = EngineSettings::instance().generalOptions();
    _ui->tableGeneralOptions->setRowCount(generalOptions.count() + 1);
    int row = 0;
    for (auto it = generalOptions.begin(); it != generalOptions.end(); ++it) {
        int col = 0;
        auto item = new QTableWidgetItem(it.key());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        _ui->tableGeneralOptions->setItem(row, col++, item);
        item = new QTableWidgetItem(it.value().toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        _ui->tableGeneralOptions->setItem(row, col++, item);
        //qDebug() << col << it.key() << it.value();
        row++;
    }

    setCurrentEngine(EngineSettings::instance().selectedEngine());
    QDialog::open();
}


void SettingsDialog::setCurrentEngine(int index)
{
    const auto &availableEngines = EngineSettings::instance().availableEngines();

    if (index < 0 || index >= availableEngines.count()) {
        return;
    }

    _ui->engineComboBox->setCurrentIndex(index);
}


void SettingsDialog::showEngineOptions(int index) const
{
    const auto &availableEngines = EngineSettings::instance().availableEngines();
    _ui->tableEngineOptions->clearContents();

    if (index < 0 || index >= availableEngines.count()) {
        return;
    }

    if (index != EngineSettings::instance().selectedEngine()) {
        EngineSettings::instance().setSelectedEngine(index);
        EngineSettings::instance().save();
    }

    // エンジンオプション
    const auto &options = availableEngines[index].options;
    _ui->tableEngineOptions->setRowCount(options.count() + 1);
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

    if (!QFileInfo(path).isExecutable()) {
        MessageBox::information(tr("Invalid file"), tr("File not executable"));
        return;
    }

    EngineSettings::EngineData data;
    data.name = [](const QString &path) {
        QString name;
        QDir dir = QFileInfo(path).absoluteDir();
        QFile nameFile(dir.absoluteFilePath("engine_name.txt"));
        if (nameFile.open(QIODevice::ReadOnly)) {
            name = nameFile.readLine().trimmed();
        }
        if (name.isEmpty()) {
            name = QFileInfo(path).fileName();
        }
        return name;
    }(path);

    data.path = path;
    data.defaultOptions = [](const QString &path) {
        QVariantMap map;
        QDir dir = QFileInfo(path).absoluteDir();
        QFile optionsFile(dir.absoluteFilePath("engine_options.txt"));
        if (optionsFile.open(QIODevice::ReadOnly)) {
            auto lines = optionsFile.readAll().trimmed().split('\n');
            for (auto &line : lines) {
                auto strs = line.trimmed().split(' ');
                strs.removeAll("");
                if (!strs.count()) {
                    continue;
                }

                QByteArray name;
                int index = strs.indexOf("name");
                if (index >= 0) {
                    name = strs.value(index + 1);
                }

                QByteArray value;
                index = strs.indexOf("default");
                if (index >= 0) {
                    value = strs.value(index + 1);
                }

                if (!name.isEmpty() && !value.isEmpty()) {
                    map.insert(name, value);
                }
            }
        }
        return map;
    }(path);
    data.options = data.defaultOptions;  // デフォルトオプションを設定する

    EngineSettings::instance().addEngine(data);
    EngineSettings::instance().save();
    _ui->engineComboBox->addItem(data.name);
    _ui->engineComboBox->setCurrentIndex(_ui->engineComboBox->count() - 1);
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
        EngineSettings::instance().save();

        _ui->engineComboBox->removeItem(idx);
        _ui->tableEngineOptions->clearContents();
        // 次のエンジン選択
        idx = std::min(idx, EngineSettings::instance().availableEngineCount() - 1);
        showEngineOptions(idx);
        _ui->engineComboBox->setCurrentIndex(idx);
    }
}
