/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QImage>
#include <QRgb>
#include <QRgba64>

inline int iAbs(const int &v)
{
    return v < 0 ? -v : v;
}

template<class Trait>
static bool fuzzyeq(const QImage &im1, const QImage &im2, int fuzziness, bool perceptiveFuzzer)
{
    Q_ASSERT(im1.format() == im2.format());
    Q_ASSERT(im1.depth() == 24 || im1.depth() == 32 || im1.depth() == 64);

    const bool hasAlpha = im1.hasAlphaChannel();
    const int height = im1.height();
    const int width = im1.width();
    for (int i = 0; i < height; ++i) {
        const Trait *line1 = reinterpret_cast<const Trait *>(im1.scanLine(i));
        const Trait *line2 = reinterpret_cast<const Trait *>(im2.scanLine(i));
        for (int j = 0; j < width; ++j) {
            auto &&px1 = line1[j];
            auto &&px2 = line2[j];
            auto fuzz = int(fuzziness);

            // Calculate the deltas
            auto dr = iAbs(int(qRed(px2)) - int(qRed(px1)));
            auto dg = iAbs(int(qGreen(px2)) - int(qGreen(px1)));
            auto db = iAbs(int(qBlue(px2)) - int(qBlue(px1)));
            auto da = iAbs(int(qAlpha(px2)) - int(qAlpha(px1)));

            // Always compare alpha even on images without it: some formats (e.g. RGBX64),
            // want it set to a certain value (e.g. 65535).
            if (da > fuzz)
                return false;

            // Calculate the perceptive fuzziness.
            if (hasAlpha && perceptiveFuzzer) {
                auto alpha = std::max(4, int(qAlpha(px1)));
                if (sizeof(Trait) == 4)
                    fuzz = std::min(fuzz * (255 / alpha), 255);
                else
                    fuzz = std::min(fuzz * (65535 / alpha), 255 * 257);
            }

            // Compare the deltas of R, G, B components.
            if (dr > fuzz)
                return false;
            if (dg > fuzz)
                return false;
            if (db > fuzz)
                return false;
        }
    }
    return true;
}

// allow each byte to be different by up to 1, to allow for rounding errors
static bool fuzzyeq(const QImage &im1, const QImage &im2, uchar fuzziness, bool perceptiveFuzzer = false)
{
    return (im1.depth() == 64) ? fuzzyeq<QRgba64>(im1, im2, int(fuzziness) * 257, perceptiveFuzzer) : fuzzyeq<QRgb>(im1, im2, int(fuzziness), perceptiveFuzzer);
}
