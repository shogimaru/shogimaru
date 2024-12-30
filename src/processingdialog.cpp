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

    QFont font = label->font();
    font.setPointSize(font.pointSize() + 2);  // 2ポイント大きく
    label->setFont(font);

    // レイアウトを設定
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    setFixedSize(300, 120);

    if (parent) {
        // 中央に表示
        QRect geo = parent->geometry();
        int x = geo.x() + (geo.width() - sizeHint().width()) / 2;
        int y = geo.y() + (geo.height() - sizeHint().height()) / 2;
        move(x, y);
    }
}
