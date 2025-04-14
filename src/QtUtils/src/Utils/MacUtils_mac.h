/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

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

#if !defined (QTUTILS_MACUTILS_MAC_H_INCLUDED)
#define QTUTILS_MACUTILS_MAC_H_INCLUDED

#include "Workarounds/BeginExcludeOpenTransport.h"
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include "Workarounds/EndExcludeOpenTransport.h"

#include <QtGlobal>
#include <QSize>

#include "CFTypePtr.h"

class QImage;
class QPixmap;
class QString;

namespace MacUtils {

QPixmap QPixmapFromCGImage(const CFTypePtr<CGImageRef> &image, const QSize &sizeInPixels = QSize());
CFTypePtr<CGImageRef> QPixmapToCGImage(const QPixmap &pixmap);

QImage QImageFromCGImage(const CFTypePtr<CGImageRef> &image, const QSize &sizeInPixels = QSize());
CFTypePtr<CGImageRef> QImageToCGImage(const QImage &image);

QString QStringFromCFString(const CFTypePtr<CFStringRef> &string);
CFTypePtr<CFStringRef> QStringToCFString(const QString &string);

} // namespace MacUtils

#endif // QTUTILS_MACUTILS_MAC_H_INCLUDED

