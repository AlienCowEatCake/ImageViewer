/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGlobal>
#if defined (QT_SVG_LIB)
#include <QGraphicsSvgItem>
#else
#include <QGraphicsItem>
#endif
#include <QFileInfo>

#include "IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"

#define DECODER_QT_SVG_PRIORITY 1800

namespace {

class DecoderQtSVG : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderQtSVG");
    }

    QList<DecoderFormatInfo> supportedFormats() const
    {
#if defined (QT_SVG_LIB)
        const QList<QByteArray> svgFormats = QList<QByteArray>()
                << "svg"
                << "svgz";
        QList<DecoderFormatInfo> result;
        for(QList<QByteArray>::ConstIterator it = svgFormats.constBegin(); it != svgFormats.constEnd(); ++it)
        {
            DecoderFormatInfo info;
            info.decoderPriority = DECODER_QT_SVG_PRIORITY;
            info.format = QString::fromLatin1(*it).toLower();
            result.append(info);
        }
        return result;
#else
        return QList<DecoderFormatInfo>();
#endif
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
#if defined (QT_SVG_LIB)
        return new QGraphicsSvgItem(filePath);
#else
        return NULL;
#endif
    }
};

DecoderAutoRegistrator registrator(new DecoderQtSVG);

} // namespace
