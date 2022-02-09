#include "analysisdialog.h"
#include "ui_analysisdialog.h"
#include "user.h"


AnalysisDialog::AnalysisDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::AnalysisDialog)
{
    _ui->setupUi(this);
    setWindowTitle("AnalysisDialog");
    // 範囲
    _ui->nodesSpinBox->setRange(1, INT_MAX);
    _ui->depthSpinBox->setRange(2, INT_MAX);

    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this, &AnalysisDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &AnalysisDialog::rejected);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &AnalysisDialog::close);
}


AnalysisDialog::~AnalysisDialog()
{
    delete _ui;
}


AnalysisDialog::Scope AnalysisDialog::scope() const
{
    return (_ui->allRadioButton->isChecked()) ? All : FromCurrentPosition;
}


void AnalysisDialog::open()
{
    auto &user = User::load();

    // 時間
    _ui->timeCheckBox->setChecked(user.analysisTimeSeconds() > 0);  // マイナスは無効化
    int seconds = std::min(std::abs(user.analysisTimeSeconds()), 23 * 60 * 60 + 59 * 60 + 59);  // 23:59:59
    _ui->timeEdit->setTime(QTime(0, 0).addSecs(seconds));

    // ノード
    _ui->nodesCheckBox->setChecked(user.analysisNodes() > 0);  // マイナスは無効化
    auto nodes = std::abs(user.analysisNodes());
    _ui->nodesSpinBox->setValue(nodes / 1000000LL);  // 1m

    // 深度
    _ui->depthCheckBox->setChecked(user.analysisDepth() > 0);
    auto depth = std::abs(user.analysisDepth());
    _ui->depthSpinBox->setValue(depth);

    QDialog::open();
}


void AnalysisDialog::accept()
{
    auto &user = User::load();

    // 時間
    int seconds = QTime(0, 0).secsTo(_ui->timeEdit->time());
    seconds *= (_ui->timeCheckBox->isChecked()) ? 1 : -1;
    user.setAnalysisTimeSeconds(seconds);

    // ノード
    qint64 nodes = _ui->nodesSpinBox->value() * 1000000LL;  // 1m
    nodes *= (_ui->nodesCheckBox->isChecked()) ? 1 : -1;
    user.setAnalysisNodes(nodes);

    // 深度
    int depth = _ui->depthSpinBox->value();
    depth *= (_ui->depthCheckBox->isChecked()) ? 1 : -1;
    user.setAnalysisDepth(depth);

    user.save();
    QDialog::accept();
}
