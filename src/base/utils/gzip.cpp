/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2015  Vladimir Golovnev <glassez@yandex.ru>
 * Copyright (C) 2006  Christophe Dumez <chris@qbittorrent.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#include "gzip.h"

#include <vector>

#include <QtGlobal>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#ifndef ZLIB_CONST
#define ZLIB_CONST  // make z_stream.next_in const
#endif
#include <zlib.h>

bool Utils::Gzip::compress(QDataStream &source, QDataStream &dest, int level)
{
    const int chunkSize = 128 * 1024;

    z_stream strm {};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    QByteArray in(chunkSize, Qt::Uninitialized);
    QByteArray out(chunkSize, Qt::Uninitialized);

    // windowBits = 15 + 16 to enable gzip
    // From the zlib manual: windowBits can also be greater than 15 for optional gzip encoding. Add 16 to windowBits
    // to write a simple gzip header and trailer around the compressed data instead of a zlib wrapper.
    int ret = deflateInit2(&strm, level, Z_DEFLATED, (15 + 16), 9, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK)
        return false;

    int flush;
    do
    {
        const qsizetype readBytes = source.readRawData(in.data(), chunkSize);
        if (readBytes == -1)
        {
            deflateEnd(&strm);
            return false;
        }
        flush = (source.status() == QDataStream::ReadPastEnd) ? Z_FINISH : Z_NO_FLUSH;
        strm.avail_in = readBytes;
        strm.next_in = reinterpret_cast<const Bytef *>(in.constData());

        do
        {
            strm.avail_out = chunkSize;
            strm.next_out = reinterpret_cast<Bytef *>(out.data());

            ret = deflate(&strm, flush);
            Q_ASSERT(ret != Z_STREAM_ERROR);

            const qsizetype have = chunkSize - strm.avail_out;
            if (dest.writeRawData(out.constData(), have) == -1 || dest.status() == QDataStream::WriteFailed)
            {
                deflateEnd(&strm);
                return false;
            }
        } while (strm.avail_out == 0);
        Q_ASSERT(strm.avail_in == 0);
    } while (flush != Z_FINISH);
    Q_ASSERT(ret == Z_STREAM_END);

    deflateEnd(&strm);
    return true;
}

QByteArray Utils::Gzip::compress(const QByteArray &data, const int level, bool *ok)
{
    if (ok) *ok = false;

    if (data.isEmpty())
        return {};

    QByteArray output;
    output.reserve(data.size());
    QDataStream source(data);
    QDataStream dest(&output, QIODevice::WriteOnly);

    if (ok)
        *ok = Utils::Gzip::compress(source, dest, level);
    return output;
}

QByteArray Utils::Gzip::decompress(const QByteArray &data, bool *ok)
{
    if (ok) *ok = false;

    if (data.isEmpty())
        return {};

    const int BUFSIZE = 1024 * 1024;
    std::vector<char> tmpBuf(BUFSIZE);

    z_stream strm {};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = reinterpret_cast<const Bytef *>(data.constData());
    strm.avail_in = uInt(data.size());
    strm.next_out = reinterpret_cast<Bytef *>(tmpBuf.data());
    strm.avail_out = BUFSIZE;

    // windowBits must be greater than or equal to the windowBits value provided to deflateInit2() while compressing
    // Add 32 to windowBits to enable zlib and gzip decoding with automatic header detection
    int result = inflateInit2(&strm, (15 + 32));
    if (result != Z_OK)
        return {};

    QByteArray output;
    // from lzbench, level 9 average compression ratio is: 31.92%, which decompression ratio is: 1 / 0.3192 = 3.13
    output.reserve(data.size() * 3);

    // run inflate
    while (true)
    {
        result = inflate(&strm, Z_NO_FLUSH);

        if (result == Z_STREAM_END)
        {
            output.append(tmpBuf.data(), (BUFSIZE - strm.avail_out));
            break;
        }

        if (result != Z_OK)
        {
            inflateEnd(&strm);
            return {};
        }

        output.append(tmpBuf.data(), (BUFSIZE - strm.avail_out));
        strm.next_out = reinterpret_cast<Bytef *>(tmpBuf.data());
        strm.avail_out = BUFSIZE;
    }

    inflateEnd(&strm);

    if (ok) *ok = true;
    return output;
}
