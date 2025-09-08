#include "stringconverter.h"
#include <iconv.h>

namespace maru {

QString fromShiftJis(const QByteArray &sjis)
{
    QString str;
    QByteArray buf;
    size_t sjislen = sjis.length();
    const size_t buflen = sjislen * 2;
    buf.reserve(buflen);
    size_t outbytesleft = buflen;

    auto *psjis = sjis.data();
    char *pbuf = buf.data();

    iconv_t ic = iconv_open("UTF-8", "SHIFT_JIS");
    if (ic != (iconv_t)-1) {
        auto res = iconv(ic, (char **)&psjis, &sjislen, &pbuf, &outbytesleft);
        if (res != (size_t)-1) {
            buf.resize(buflen - outbytesleft);
            str = QString::fromUtf8(buf);
        }
        iconv_close(ic);
    }

    return str;
}

}
