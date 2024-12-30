#pragma once
#include <QDialog>


class ProcessingDialog : public QDialog {
public:
    ProcessingDialog(const QString &message, QWidget *parent = nullptr);
};
