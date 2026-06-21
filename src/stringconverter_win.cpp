#include "stringconverter.h"
#include <vector>
#include <locale.h>
#include <vector>
#include <windows.h>

namespace maru {

QString fromShiftJis(const QByteArray &sjis, bool &error)
{
    static auto lc = _create_locale(LC_ALL, "jpn");
    std::size_t converted;
    std::vector<wchar_t> buf(sjis.size() + 1, L'\0');

    if (::_mbstowcs_s_l(&converted, buf.data(), sjis.size(), sjis.data(), _TRUNCATE, lc) != 0) {
        error = true;
        return QString();
    }

    buf.resize(std::char_traits<wchar_t>::length(buf.data()));
    error = false;
    return QString::fromStdWString(std::wstring(buf.begin(), buf.end()));
}

}
