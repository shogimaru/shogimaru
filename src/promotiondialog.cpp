#include "promotiondialog.h"
#include "global.h"

static PromotionDialog *dialog = nullptr;


PromotionDialog::PromotionDialog(Piece *src, Piece *dst, QWidget *parent) :
    QMessageBox(QMessageBox::Question, tr("Promotion"), tr("Promote?"), (QMessageBox::Yes | QMessageBox::No), parent),
    _src(src),
    _dst(dst)
{
    setWindowModality(Qt::ApplicationModal);
    setMaximumSize(300, 200);
    setStyleSheet("QLabel{text-align: left; margin: 30px 20px 30px 0px;}");

    if (parent) {
        // 中央に表示
        QRect geo = parent->geometry();
        int x = geo.x() + (geo.width() - sizeHint().width()) / 2;
        int y = geo.y() + (geo.height() - sizeHint().height()) / 2;
        move(x, y);
    }
}


PromotionDialog::~PromotionDialog()
{
}


void PromotionDialog::question(Piece *src, Piece *dst, QObject *receiver, const char *member)
{
    delete dialog;
    dialog = new PromotionDialog(src, dst
#ifndef Q_OS_WASM
    , maru::mainWindow()
#endif
    );
    dialog->open(receiver, member);
}


void PromotionDialog::abort()
{
    delete dialog;
    dialog = nullptr;
}
