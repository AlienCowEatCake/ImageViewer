/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>

#include "InfoUtils.h"

#include <QString>

namespace InfoUtils {

/// @brief Получить человеко-читаемую информацию о системе
QString GetSystemDescription()
{
    QString result;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    SInt32 majorVersion, minorVersion, bugFixVersion;
    Gestalt(gestaltSystemVersionMajor, &majorVersion);
    Gestalt(gestaltSystemVersionMinor, &minorVersion);
    Gestalt(gestaltSystemVersionBugFix, &bugFixVersion);

    if(majorVersion < 10)
        result.append(QString::fromLatin1("Mac OS"));
    else if(majorVersion == 10 && minorVersion < 8)
        result.append(QString::fromLatin1("Mac OS X"));
    else if(majorVersion == 10 && minorVersion < 12)
        result.append(QString::fromLatin1("OS X"));
    else
        result.append(QString::fromLatin1("macOS"));

    result.append(QString::fromLatin1(" %1.%2").arg(majorVersion).arg(minorVersion));
    if(bugFixVersion > 0)
        result.append(QString::fromLatin1(".%1").arg(bugFixVersion));

    switch (majorVersion * 10 + minorVersion)
    {
    case 100:
        result.append(QString::fromLatin1(" \"Cheetah\""));
        break;
    case 101:
        result.append(QString::fromLatin1(" \"Puma\""));
        break;
    case 102:
        result.append(QString::fromLatin1(" \"Jaguar\""));
        break;
    case 103:
        result.append(QString::fromLatin1(" \"Panther\""));
        break;
    case 104:
        result.append(QString::fromLatin1(" \"Tiger\""));
        break;
    case 105:
        result.append(QString::fromLatin1(" \"Leopard\""));
        break;
    case 106:
        result.append(QString::fromLatin1(" \"Snow Leopard\""));
        break;
    case 107:
        result.append(QString::fromLatin1(" \"Lion\""));
        break;
    case 108:
        result.append(QString::fromLatin1(" \"Mountain Lion\""));
        break;
    case 109:
        result.append(QString::fromLatin1(" \"Mavericks\""));
        break;
    case 110:
        result.append(QString::fromLatin1(" \"Yosemite\""));
        break;
    case 111:
        result.append(QString::fromLatin1(" \"El Capitan\""));
        break;
    case 112:
        result.append(QString::fromLatin1(" \"Sierra\""));
        break;
    default:
        break;
    }

    [pool release];
    return result;
}

} // namespace InfoUtils

