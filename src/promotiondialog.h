#pragma once
#include <QMessageBox>
#include <QObject>

class Piece;


class PromotionDialog : public QMessageBox {
    Q_OBJECT
public:
    virtual ~PromotionDialog();

    Piece *src() { return _src; }
    Piece *dst() { return _dst; }

    static void question(Piece *src, Piece *dst, QObject *receiver, const char *member);
    static void abort();

private:
    PromotionDialog(Piece *src, Piece *dst, QWidget *parent = nullptr);

    Piece *_src {nullptr};
    Piece *_dst {nullptr};
};
