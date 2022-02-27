#include "settingsdialog.h"
#include "enginesettings.h"
#include "ui_settingsdialog.h"
#include "westerntabstyle.h"
#include <QFileDialog>
#include <QTabBar>
#include <QtCore>

constexpr auto EngineListFilePath = "engines.json";
static WesternTabStyle westernTabStyle;


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog)
{
    _ui->setupUi(this);
    _ui->tabWidget->tabBar()->setStyle(&westernTabStyle);
    _ui->tabWidget->setCurrentIndex(0);

    connect(_ui->closeButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(_ui->newEngineButton, &QPushButton::clicked, this, &SettingsDialog::getEngineFilePath);
}


void SettingsDialog::open()
{
    //EngineSettings::instance().save();

    QDialog::open();
}


void SettingsDialog::accept()
{


    QDialog::accept();
}


void SettingsDialog::getEngineFilePath()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select Engine"), QString(),
#if Q_OS_WIN
        tr("Executable (*.exe)")
#else
        tr("Executable (*)")
#endif
    );

    _ui->engineComboBox->addItem(path);
}


void SettingsDialog::loadAvailableEngineList()
{
    QFile file(EngineListFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    auto json = QJsonDocument::fromJson(file.readAll()).object();
    QStringList engineNames;
    for (const auto &engine : json.value("availableEngines").toArray()) {
        engineNames << engine.toObject().value("name").toString();
    }
}


void SettingsDialog::save()
{
}


/* 利用可能なエンジンのリスト
{
    "availableEngines": [
        {
            "name": "engine1",
            "path": "xxxx/hoge",
            "options": {
                "EvalDir": "fuga",
                "BookDir": "fuga",
                "option1": 1,
                "option2": 3
            },
            "defaultOptions": {
                "option1": 1,
                "option2": 1
            }
        },
        {
            "name": "engine2",
            "path": "xxxx/foo",
            "options": {
                "EvalDir": "fuga",
                "BookDir": "fuga",
                "option1": 16,
                "option2": 32
            },
            "defaultOptions": {
                "option1": 1,
                "option2": 1
            }
        }
    ],
    "selectedEngine": 0,
    "generalOptions": {
        "option1": 0,
        "option2": 0,
    }
}
*/
