/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"

namespace {

class DecoderQtSVG : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderQtSVG");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
#if defined (QT_SVG_LIB)
                << QString::fromLatin1("svg")
                << QString::fromLatin1("svgz")
#endif
                ;
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    bool isAvailable() const
    {
#if defined (QT_SVG_LIB)
        return true;
#else
        return false;
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
