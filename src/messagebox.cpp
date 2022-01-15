#include "messagebox.h"


void MessageBox::information(const QString &title, const QString &text, QObject *receiver, const char *member)
{
    static QMessageBox *box = nullptr;

    delete box;
    box = new QMessageBox(QMessageBox::Information, title, text, QMessageBox::Ok);
    box->setMaximumSize(500, 200);

    //box->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::FramelessWindowHint);
    if (receiver && member) {
        box->open(receiver, member);
    } else {
        box->open();
    }
}


void MessageBox::question(const QString &title, const QString &text, QObject *receiver, const char *member)
{
    static QMessageBox *box = nullptr;

    delete box;
    box = new QMessageBox(QMessageBox::Question, title, text, QMessageBox::Ok | QMessageBox::Cancel);
    box->setMaximumSize(500, 200);
    box->open(receiver, member);
}
