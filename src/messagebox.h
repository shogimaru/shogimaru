#pragma once
#include <QMessageBox>


class MessageBox {
public:
    static void information(const QString &title, const QString &text, QObject *receiver = nullptr, const char *member = nullptr);
    static void question(const QString &title, const QString &text, QObject *receiver, const char *member);
};
