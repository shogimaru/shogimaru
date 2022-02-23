#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "westerntabstyle.h"
#include <QTabBar>

static WesternTabStyle westernTabStyle;


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog)
{
    _ui->setupUi(this);
    _ui->tabWidget->tabBar()->setStyle(&westernTabStyle);
    _ui->tabWidget->setCurrentIndex(0);
}


void SettingsDialog::open()
{
    QDialog::open();
}
