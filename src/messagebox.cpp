#include "messagebox.h"
#include "global.h"

static QMessageBox *ibox = nullptr;
static QMessageBox *qbox = nullptr;


void MessageBox::information(const QString &title, const QString &text, QObject *receiver, const char *member)
{
    if (qbox) {
        qbox->deleteLater();
    }
    qbox = nullptr;

    if (ibox) {
        ibox->deleteLater();
    }

    QWidget *parent = nullptr;
#ifndef Q_OS_WASM
    parent = maru::mainWindow();
#endif

    ibox = new QMessageBox(QMessageBox::Information, title, text, QMessageBox::Ok, parent);
    ibox->setMaximumSize(500, 200);
    ibox->setStyleSheet("QLabel{text-align: left; margin: 30px 20px 30px 0px;}");

    if (parent) {
        // 中央に表示
        QRect geo = parent->geometry();
        int x = geo.x() + (geo.width() - ibox->sizeHint().width()) / 2;
        int y = geo.y() + (geo.height() - ibox->sizeHint().height()) / 2;
        ibox->move(x, y);
    }

    //box->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::FramelessWindowHint);
    if (receiver && member) {
        ibox->open(receiver, member);
    } else {
        ibox->open();
    }
}


void MessageBox::question(const QString &title, const QString &text, QObject *receiver, const char *member)
{
    if (ibox) {
        ibox->deleteLater();
    }
    ibox = nullptr;

    if (qbox) {
        qbox->deleteLater();
    }

    QWidget *parent = nullptr;
#ifndef Q_OS_WASM
    parent = maru::mainWindow();
#endif

    qbox = new QMessageBox(QMessageBox::Question, title, text, QMessageBox::Ok | QMessageBox::Cancel, parent);

    if (parent) {
        // 中央に表示
        QRect geo = parent->geometry();
        int x = geo.x() + (geo.width() - qbox->sizeHint().width()) / 2;
        int y = geo.y() + (geo.height() - qbox->sizeHint().height()) / 2;
        qbox->move(x, y);
    }

    qbox->setMaximumSize(500, 200);
    qbox->setStyleSheet("QLabel{text-align: left; margin: 30px 20px 30px 0px;}");
    qbox->open(receiver, member);
}


void MessageBox::question(const QString &title, const QString &text, std::function<void(void)> functor)
{
    static Receiver *receiver = nullptr;
    delete receiver;
    receiver = new Receiver(functor);
    MessageBox::question(title, text, receiver, SLOT(receive(QAbstractButton *)));
}
