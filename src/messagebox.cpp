#include "messagebox.h"
#include "global.h"

static QMessageBox *ibox = nullptr;
static QMessageBox *qbox = nullptr;


void MessageBox::information(const QString &title, const QString &text, QObject *receiver, const char *member)
{
    delete qbox;
    qbox = nullptr;

    delete ibox;
    ibox = new QMessageBox(QMessageBox::Information, title, text, QMessageBox::Ok
#ifndef __EMSCRIPTEN__
    , maru::mainWindow()
#endif
    );
    ibox->setMaximumSize(500, 200);
    ibox->setStyleSheet("QLabel{text-align: left; margin: 30px 20px 30px 0px;}");

    //box->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::FramelessWindowHint);
    if (receiver && member) {
        ibox->open(receiver, member);
    } else {
        ibox->open();
    }
}


void MessageBox::question(const QString &title, const QString &text, QObject *receiver, const char *member)
{
    delete ibox;
    ibox = nullptr;

    delete qbox;
    qbox = new QMessageBox(QMessageBox::Question, title, text, QMessageBox::Ok | QMessageBox::Cancel
#ifndef __EMSCRIPTEN__
    , maru::mainWindow()
#endif
    );
    qbox->setMaximumSize(500, 200);
    qbox->setStyleSheet("QLabel{text-align: left; margin: 30px 20px 30px 0px;}");
    qbox->open(receiver, member);
}
