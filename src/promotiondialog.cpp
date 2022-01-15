#include "promotiondialog.h"

static PromotionDialog *dialog = nullptr;


PromotionDialog::PromotionDialog(Piece *src, Piece *dst, QWidget *parent) :
    QMessageBox(QMessageBox::Question, u8"成り駒", u8"駒を成りますか", (QMessageBox::Yes | QMessageBox::No), parent),
    _src(src),
    _dst(dst)
{
    setModal(true);
    setWindowModality(Qt::ApplicationModal);
    setMaximumSize(300, 200);
}


PromotionDialog::~PromotionDialog()
{ }


void PromotionDialog::question(Piece *src, Piece *dst, QObject *receiver, const char *member)
{
    delete dialog;
    dialog = new PromotionDialog(src, dst);
    dialog->open(receiver, member);
}


void PromotionDialog::abort()
{
    delete dialog;
    dialog = nullptr;
}
