#include "stringconverter.h"
#include <locale.h>
#include <windows.h>

namespace maru {

QString fromShiftJis(const QByteArray &sjis)
{
    std::size_t converted;
    std::vector<wchar_t> buf(sjis.size(), L'\0');

    if (::_mbstowcs_s_l(&converted, buf.data(), buf.size(), sjis.data(), buf.size(), _create_locale(LC_ALL, "jpn")) != 0) {
        return QString();
    }
    buf.resize(std::char_traits<wchar_t>::length(buf.data()));
    //buf.shrink_to_fit();
    return QString::fromStdWString(std::wstring(buf.begin(), buf.end()));
}

}
