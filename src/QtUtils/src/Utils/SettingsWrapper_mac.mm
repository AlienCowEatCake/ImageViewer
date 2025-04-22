/*
   Copyright (C) 2011-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "Workarounds/BeginExcludeOpenTransport.h"
#import <Foundation/Foundation.h>
#include "Workarounds/EndExcludeOpenTransport.h"

#include <QString>
#include <QVariant>

#include "ObjectiveCUtils.h"

namespace SettingsEncoder {

/// @brief Encoder from QVariant to QString, uses human-readable representation if possible
/// @param[in] data - data to encode
/// @return Encoded data
QString Encode(const QVariant &data);

/// @brief Decoder from QString to QVariant
/// @param[in] data - encoded data from Encode()
/// @return Decoded data
/// @attention Suitable only for encoded data from Encode()
QVariant Decode(const QString &data);

} // namespace SettingsEncoder

namespace NativeSettingsStorage {

namespace {

/// @brief Get NSUserDefaults compatible key for specified key and group
/// @param[in] group - group (section or prefix) of settings
/// @param[in] key - original key in group
/// @return NSUserDefaults compatible key for specified key and group
QString getNativeKeyString(const QString &group, const QString &key)
{
    return (group.isEmpty() ? QString() : (group + QString::fromLatin1("/"))) + key;
}

} // namespace

/// @brief Set value to NSUserDefaults for specified key and group
/// @param[in] group - group (section or prefix) of settings
/// @param[in] key - key for set
/// @param[in] value - value for for set
void setValue(const QString &group, const QString &key, const QVariant &value)
{
    AUTORELEASE_POOL;
    NSString *nativeKey = ObjCUtils::QStringToNSString(getNativeKeyString(group, key));
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *nativeValue = ObjCUtils::QStringToNSString(SettingsEncoder::Encode(value));
    [defaults setObject: nativeValue forKey: nativeKey];
    [defaults synchronize];
}

/// @brief Get value from NSUserDefaults for specified key and group
/// @param[in] group - group (section or prefix) of settings
/// @param[in] key - key for get
/// @param[in] defaultValue - default value if value is absent
/// @return - value for specified key or defaultValue if value is absent
QVariant value(const QString &group, const QString &key, const QVariant &defaultValue)
{
    AUTORELEASE_POOL;
    NSString *nativeKey = ObjCUtils::QStringToNSString(getNativeKeyString(group, key));
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    const QString value = ObjCUtils::QStringFromNSString([defaults stringForKey: nativeKey]);
    QVariant result = defaultValue;
    if(!value.isEmpty())
    {
        const QVariant variantValue = SettingsEncoder::Decode(value);
        if(variantValue.isValid())
            result = variantValue;
    }
    return result;
}

} // namespace NativeSettingsStorage

