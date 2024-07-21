/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

/// @note qDebug macro conflicts
#import <CoreServices/CoreServices.h>

#include "InfoUtils.h"

#import <Foundation/Foundation.h>
#include <AvailabilityMacros.h>

#include <stdlib.h>

#include <QString>

#include "Global.h"
#include "ObjectiveCUtils.h"

namespace InfoUtils {

namespace {

struct Version
{
    int major;
    int minor;
    int patch;

    Version(const int major = -1, const int minor = -1, const int patch = -1)
    {
        this->major = major;
        this->minor = minor;
        this->patch = patch;
    }
};

template <typename T>
Version CreateVersion(const T major = -1, const T minor = -1, const T patch = -1)
{
    return Version(static_cast<int>(major), static_cast<int>(minor), static_cast<int>(patch));
}

int NSStringToInt(const NSString *str)
{
    if(!str)
        return 0;

    const char *utfStr = [str UTF8String];
    if(!utfStr)
        return 0;

    return atoi(utfStr);
}

Version GetCurrentMacVersionImpl()
{
    AUTORELEASE_POOL;

#if defined (AVAILABLE_MAC_OS_X_VERSION_10_10_AND_LATER)
    NSProcessInfo *processInfo = [NSProcessInfo processInfo];
    if([processInfo respondsToSelector:@selector(operatingSystemVersion)])
    {
        NSOperatingSystemVersion version = [processInfo operatingSystemVersion];
        return CreateVersion(version.majorVersion, version.minorVersion, version.patchVersion);
    }
#endif

    if(NSDictionary *systemVersion = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"])
    {
        NSString *productVersion = [systemVersion objectForKey:@"ProductVersion"];
        if(productVersion && [productVersion isKindOfClass:[NSString class]])
        {
            NSArray *components = [productVersion componentsSeparatedByString:@"."];
            if(components && [components count] >= 2)
            {
                const int major = NSStringToInt((NSString*)[components objectAtIndex:0]);
                const int minor = NSStringToInt((NSString*)[components objectAtIndex:1]);
                const int patch = ([components count] >= 3 ? NSStringToInt((NSString*)[components objectAtIndex:2]) : -1);
                return CreateVersion(major, minor, patch);
            }
        }
    }

    SInt32 majorVersion = 0, minorVersion = 0, bugFixVersion = 0;
#if !defined (AVAILABLE_MAC_OS_X_VERSION_10_8_AND_LATER)
    Gestalt(gestaltSystemVersionMajor, &majorVersion);
    Gestalt(gestaltSystemVersionMinor, &minorVersion);
    Gestalt(gestaltSystemVersionBugFix, &bugFixVersion);
#else
    typedef OSErr (*Gestalt_t)(OSType selector, SInt32 *response);
    Gestalt_t Gestalt_f = Q_NULLPTR;
    CFBundleRef coreServicesBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CoreServices"));
    if(coreServicesBundle)
        Gestalt_f = reinterpret_cast<Gestalt_t>(CFBundleGetFunctionPointerForName(coreServicesBundle, CFSTR("Gestalt")));
    if(Gestalt_f)
    {
        Gestalt_f('sys1', &majorVersion);
        Gestalt_f('sys2', &minorVersion);
        Gestalt_f('sys3', &bugFixVersion);
    }
#endif
    return CreateVersion(majorVersion, minorVersion, bugFixVersion);
}

Version GetCurrentMacVersion()
{
    static const Version version = GetCurrentMacVersionImpl();
    return version;
}

} // namespace

/// @brief Проверить текущую версию macOS
bool MacVersionGreatOrEqual(const int major, const int minor, const int patch)
{
    const Version version = GetCurrentMacVersion();
    if(version.major > major)
        return true;
    if(version.major < major)
        return false;
    if(version.minor > minor)
        return true;
    if(version.minor < minor)
        return false;
    if(version.patch > patch)
        return true;
    if(version.patch < patch)
        return false;
    return true;
}

/// @brief Получить человеко-читаемую информацию о системе
QString GetSystemDescription()
{
    const Version version = GetCurrentMacVersion();

    QString osName;
    switch(version.major * 100 + (version.major < 11 ? version.minor : 0))
    {
    case 1000:
        osName = QString::fromLatin1("Cheetah");
        break;
    case 1001:
        osName = QString::fromLatin1("Puma");
        break;
    case 1002:
        osName = QString::fromLatin1("Jaguar");
        break;
    case 1003:
        osName = QString::fromLatin1("Panther");
        break;
    case 1004:
        osName = QString::fromLatin1("Tiger");
        break;
    case 1005:
        osName = QString::fromLatin1("Leopard");
        break;
    case 1006:
        osName = QString::fromLatin1("Snow Leopard");
        break;
    case 1007:
        osName = QString::fromLatin1("Lion");
        break;
    case 1008:
        osName = QString::fromLatin1("Mountain Lion");
        break;
    case 1009:
        osName = QString::fromLatin1("Mavericks");
        break;
    case 1010:
        osName = QString::fromLatin1("Yosemite");
        break;
    case 1011:
        osName = QString::fromLatin1("El Capitan");
        break;
    case 1012:
        osName = QString::fromLatin1("Sierra");
        break;
    case 1013:
        osName = QString::fromLatin1("High Sierra");
        break;
    case 1014:
        osName = QString::fromLatin1("Mojave");
        break;
    case 1015:
        osName = QString::fromLatin1("Catalina");
        break;
    case 1016:
    case 1100:
        osName = QString::fromLatin1("Big Sur");
        break;
    case 1200:
        osName = QString::fromLatin1("Monterey");
        break;
    case 1300:
        osName = QString::fromLatin1("Ventura");
        break;
    case 1400:
        osName = QString::fromLatin1("Sonoma");
        break;
    case 1500:
        osName = QString::fromLatin1("Sequoia");
        break;
    default:
        break;
    }

    const QString osPrefix =
            version.major < 10                          ? QString::fromLatin1("Mac OS")     :
            version.major == 10 && version.minor < 8    ? QString::fromLatin1("Mac OS X")   :
            version.major == 10 && version.minor < 12   ? QString::fromLatin1("OS X")       :
                                                          QString::fromLatin1("macOS")      ;

    QString result = QString::fromLatin1("%1 %2.%3").arg(osPrefix).arg(version.major).arg(version.minor);
    if(version.patch > 0)
        result.append(QString::fromLatin1(".%1").arg(version.patch));
    if(!osName.isEmpty())
        result.append(QString::fromLatin1(" \"%1\"").arg(osName));
    return result;
}

} // namespace InfoUtils

