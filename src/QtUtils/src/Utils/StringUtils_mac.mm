/*
   Copyright (C) 2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "StringUtils.h"

#include <AvailabilityMacros.h>
#include <CoreFoundation/CoreFoundation.h>

#import <Foundation/Foundation.h>

#include "Global.h"
#include "InfoUtils.h"
#include "ObjectiveCUtils.h"

namespace StringUtils {

bool PlatformNumericLessThan(const QString &s1, const QString &s2)
{
    // https://developer.apple.com/library/archive/qa/qa1159/_index.html
#if defined (AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER) && defined (MAC_OS_X_VERSION_MAX_ALLOWED) && (MAC_OS_X_VERSION_MAX_ALLOWED >= 1060)
    if(InfoUtils::MacVersionGreatOrEqual(10, 6))
    {
        AUTORELEASE_POOL;
        NSString *nss1 = ObjCUtils::QStringToNSString(s1);
        NSString *nss2 = ObjCUtils::QStringToNSString(s2);
        if(!nss1 && !nss2)
            return false;
        if(!nss1)
            return true;
        if(!nss2)
            return false;
        return [nss1 localizedStandardCompare:nss2] == NSOrderedAscending;
    }
#endif
    const CFIndex s1Len = static_cast<CFIndex>(s1.size());
    const CFIndex s2Len = static_cast<CFIndex>(s2.size());
    const UniChar *s1Buf = reinterpret_cast<const UniChar*>(s1.constData());
    const UniChar *s2Buf = reinterpret_cast<const UniChar*>(s2.constData());
    if((!s1Buf && !s2Buf) || (!s1Len && !s2Len))
        return false;
    if(!s1Buf || !s1Len)
        return true;
    if(!s2Buf || !s2Len)
        return false;
    SInt32 compareResult = 0;
    const OSStatus compareStatus = UCCompareTextDefault(
                kUCCollateComposeInsensitiveMask | kUCCollateWidthInsensitiveMask | /*kUCCollateCaseInsensitiveMask |*/
                kUCCollateDigitsOverrideMask | kUCCollateDigitsAsNumberMask | kUCCollatePunctuationSignificantMask,
                s1Buf, s1Len, s2Buf, s2Len, Q_NULLPTR, &compareResult);
    if(compareStatus == noErr)
        return compareResult < 0;

    return NumericLessThan(s1, s2);
}

} // namespace StringUtils

