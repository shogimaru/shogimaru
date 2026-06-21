#include "cp932decoder.h"
#include <algorithm>
#include <cstdint>

namespace {

constexpr std::uint32_t ReplacementChar = 0xFFFDU;

bool isLeadByte(unsigned char c)
{
    return (0x81 <= c && c <= 0x9F) || (0xE0 <= c && c <= 0xFC);
}

bool isTrailByte(unsigned char c)
{
    return (0x40 <= c && c <= 0x7E) || (0x80 <= c && c <= 0xFC);
}

std::uint32_t lookupSingle(unsigned char c)
{
    return maru::cp932::single[static_cast<std::size_t>(c)];
}

std::uint32_t lookupPair(unsigned char lead, unsigned char trail)
{
    const std::uint16_t key = static_cast<std::uint16_t>((static_cast<std::uint16_t>(lead) << 8) |
                                static_cast<std::uint16_t>(trail));

    const auto &pairs = maru::cp932::pairs;

    auto it = std::lower_bound(
        pairs.begin(),
        pairs.end(),
        key,
        [](const maru::cp932::Pair &p, std::uint16_t k) {
            return p.key < k;
        }
    );

    if (it != pairs.end() && it->key == key) {
        return it->ucs4;
    }

    return maru::cp932::Invalid;
}

void appendUcs4(QString &out, std::uint32_t cp)
{
    if (cp <= 0xFFFFU) {
        out.append(QChar(static_cast<char16_t>(cp)));
        return;
    }

    cp -= 0x10000U;
    const char16_t high = static_cast<char16_t>(0xD800U + ((cp >> 10) & 0x3FFU));
    const char16_t low = static_cast<char16_t>(0xDC00U + (cp & 0x3FFU));

    out.append(QChar(high));
    out.append(QChar(low));
}

} // namespace

namespace maru {

QString fromCp932(const QByteArray &bytes, bool &error)
{
    QString out;
    out.reserve(bytes.size());

    const auto *p = reinterpret_cast<const unsigned char *>(bytes.constData());
    const qsizetype n = bytes.size();

    qsizetype i = 0;
    error = false;

    while (i < n) {
        const unsigned char c = p[i];

        if (isLeadByte(c)) {
            if (i + 1 >= n) {
                error = true;
                appendUcs4(out, ReplacementChar);
                ++i;
                continue;
            }

            const unsigned char trail = p[i + 1];

            if (!isTrailByte(trail)) {
                error = true;
                appendUcs4(out, ReplacementChar);
                ++i;
                continue;
            }

            const std::uint32_t cp = lookupPair(c, trail);

            if (cp == maru::cp932::Invalid) {
                error = true;
                appendUcs4(out, ReplacementChar);
            } else {
                appendUcs4(out, cp);
            }

            i += 2;
            continue;
        }

        const std::uint32_t cp = lookupSingle(c);

        if (cp == maru::cp932::Invalid) {
            error = true;
            appendUcs4(out, ReplacementChar);
        } else {
            appendUcs4(out, cp);
        }

        ++i;
    }

    return out;
}

} // namespace maru
