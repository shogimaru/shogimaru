#include "operationbuttongroup.h"
#include "ui_operationbuttongroup.h"


OperationButtonGroup::OperationButtonGroup(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::OperationButtonGroup)
{
    _ui->setupUi(this);
    _ui->nextButton->setText(QString::fromUtf8(u8"▶"));
    _ui->next10Button->setText(QString::fromUtf8(u8"▶▶"));
    _ui->nextCandidateButton->setText(QString::fromUtf8(u8">"));
    _ui->prevButton->setText(QString::fromUtf8(u8"◀"));
    _ui->prev10Button->setText(QString::fromUtf8(u8"◀◀"));
    _ui->prevCandidateButton->setText(QString::fromUtf8(u8"<"));
    _ui->firstButton->setText(QString::fromUtf8(u8"｜◀"));
    _ui->lastButton->setText(QString::fromUtf8(u8"▶｜"));

#ifdef Q_OS_WIN
    QFont font("Yu Gothic UI", 8);
    _ui->nextButton->setFont(font);
    _ui->next10Button->setFont(font);
    _ui->nextCandidateButton->setFont(font);
    _ui->prevButton->setFont(font);
    _ui->prev10Button->setFont(font);
    _ui->prevCandidateButton->setFont(font);
    _ui->firstButton->setFont(font);
    _ui->lastButton->setFont(font);
#endif

    connect(_ui->nextButton, &QPushButton::clicked, this, &OperationButtonGroup::next);
    connect(_ui->next10Button, &QPushButton::clicked, this, &OperationButtonGroup::next10Moves);
    connect(_ui->nextCandidateButton, &QPushButton::clicked, this, &OperationButtonGroup::nextCandidate);
    connect(_ui->prevButton, &QPushButton::clicked, this, &OperationButtonGroup::previous);
    connect(_ui->prev10Button, &QPushButton::clicked, this, &OperationButtonGroup::previous10Moves);
    connect(_ui->prevCandidateButton, &QPushButton::clicked, this, &OperationButtonGroup::previousCandidate);
    connect(_ui->firstButton, &QPushButton::clicked, this, &OperationButtonGroup::firstPosition);
    connect(_ui->lastButton, &QPushButton::clicked, this, &OperationButtonGroup::lastPosition);
}


void OperationButtonGroup::setEnabled(bool enable)
{
    _ui->nextButton->setEnabled(enable);
    _ui->next10Button->setEnabled(enable);
    _ui->nextCandidateButton->setEnabled(enable);
    _ui->prevButton->setEnabled(enable);
    _ui->prev10Button->setEnabled(enable);
    _ui->prevCandidateButton->setEnabled(enable);
    _ui->firstButton->setEnabled(enable);
    _ui->lastButton->setEnabled(enable);
}
