#pragma once
#include <QString>

namespace maru {

QString fromShiftJis(const QByteArray &sjis, bool &error);

}
