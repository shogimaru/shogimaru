#include "processingdialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QString>


ProcessingDialog::ProcessingDialog(const QString &message, QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);  // 閉じるボタンを無効化
    setModal(true);

    auto *label = new QLabel(message, this);
    label->setAlignment(Qt::AlignCenter);

    // レイアウトを設定
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);

    setFixedSize(300, 200);
}
