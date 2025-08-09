/*
    Packbits compression used on many legacy formats (IFF, PSD, TIFF).

    SPDX-FileCopyrightText: 2025 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef PACKBITS_P_H
#define PACKBITS_P_H

#include <cstring>

#include <QIODevice>

/*!
 * \brief packbitsDecompress
 * Fast PackBits decompression.
 * \param input The compressed input buffer.
 * \param ilen The input buffer size.
 * \param output The uncompressed target buffer.
 * \param olen The target buffer size.
 * \param allowN128 If true, -128 is a valid run length size (false for PSD / TIFF, true for IFF) .
 * \return The number of valid bytes in the target buffer.
 */
inline qint64 packbitsDecompress(const char *input, qint64 ilen, char *output, qint64 olen, bool allowN128 = false)
{
    qint64  j = 0;
    for (qint64 ip = 0, rr = 0, available = olen; j < olen && ip < ilen; available = olen - j) {
        signed char n = static_cast<signed char>(input[ip++]);
        if (n == -128 && !allowN128)
            continue;

        if (n >= 0) {
            rr = qint64(n) + 1;
            if (available < rr) {
                --ip;
                break;
            }

            if (ip + rr > ilen)
                return -1;
            memcpy(output + j, input + ip, size_t(rr));
            ip += rr;
        } else if (ip < ilen) {
            rr = qint64(1-n);
            if (available < rr) {
                --ip;
                break;
            }
            memset(output + j, input[ip++], size_t(rr));
        }

        j += rr;
    }
    return j;
}

/*!
 * \brief packbitsDecompress
 * PackBits decompression.
 * \param input The input device.
 * \param output The uncompressed target buffer.
 * \param olen The target buffer size.
 * \param allowN128 If true, -128 is a valid run length size (false for PSD / TIFF, true for IFF) .
 * \return The number of valid bytes in the target buffer.
 */
inline qint64 packbitsDecompress(QIODevice *input, char *output, qint64 olen, bool allowN128 = false)
{
    qint64  j = 0;
    for (qint64 rr = 0, available = olen; j < olen; available = olen - j) {
        char n;

        // check the output buffer space for the next run
        if (available < 129) {
            if (input->peek(&n, 1) != 1) { // end of data (or error)
                break;
            }
            if (static_cast<signed char>(n) != -128 || allowN128)
                if ((static_cast<signed char>(n) >= 0 ? qint64(n) + 1 : qint64(1 - n)) > available)
                    break;
        }

        // decompress
        if (input->read(&n, 1) != 1) { // end of data (or error)
            break;
        }

        if (static_cast<signed char>(n) == -128 && !allowN128) {
            continue;
        }

        if (static_cast<signed char>(n) >= 0) {
            rr = input->read(output + j, qint64(n) + 1);
            if (rr == -1) {
                return -1;
            }
        }
        else {
            char b;
            if (input->read(&b, 1) != 1) {
                break;
            }
            rr = qint64(1 - static_cast<signed char>(n));
            std::memset(output + j, b, size_t(rr));
        }

        j += rr;
    }
    return j;
}


#endif // PACKBITS_P_H
