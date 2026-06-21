#include "stringconverter.h"
#include <iconv.h>
#ifdef Q_OS_WASM
#include "cp932decoder.h"
#endif

namespace maru {

QString fromShiftJis(const QByteArray &sjis, bool &error)
{
    const char *froms[] = {
        "Shift_JISX0213",
        "CP932",
        "WINDOWS-31J",
        "SHIFT_JIS",
        "MS932",
        "SJIS"
    };

    QString str;
    QByteArray buf;
    size_t sjislen = sjis.length();

    if (sjislen == 0) {
        return str;
    }

    const size_t buflen = sjislen * 3;
    buf.reserve(buflen);
    size_t outbytesleft = buflen;

    auto *psjis = sjis.data();
    char *pbuf = buf.data();
    error = true;

    for (const char *from : froms) {
        iconv_t ic = iconv_open("UTF-8", from);
        if (ic != (iconv_t)-1) {
            auto res = iconv(ic, (char **)&psjis, &sjislen, &pbuf, &outbytesleft);
            if (res != (size_t)-1) {
                buf.resize(buflen - outbytesleft);
                str = QString::fromUtf8(buf);
                error = false;
            }
            iconv_close(ic);
            break;
        }
    }

#ifdef Q_OS_WASM
    if (error) {
        str = fromCp932(sjis, error);
    }
#endif

    return str;
}

}
