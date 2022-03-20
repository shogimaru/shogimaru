#pragma once
#include <QDebug>
#include <QMessageBox>
#include <QObject>
#include <functional>


class MessageBox {
public:
    static void information(const QString &title, const QString &text, QObject *receiver = nullptr, const char *member = nullptr);
    static void question(const QString &title, const QString &text, QObject *receiver, const char *member);
    static void question(const QString &title, const QString &text, std::function<void(void)> functor);
};


// For internal use
class Receiver : public QObject {
    Q_OBJECT
    std::function<void(void)> func;
public:
    Receiver(std::function<void(void)> functor, QObject *parent = nullptr) :
        QObject(parent), func(functor) { }
    ~Receiver() { }

public slots:
    void receive(QAbstractButton *button)
    {
        auto *box = dynamic_cast<QMessageBox *>(sender());
        if (box && box->buttonRole(button) == QMessageBox::AcceptRole) {
            func();
        }
    }
};
