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

/// @brief Кодировщик данных QVariant -> QString, по возможности использует человеко-читаемое представление
/// @param[in] data - Исходные данные
/// @return Кодированные данные
QString Encode(const QVariant &data);

/// @brief Декодировщик данных QString -> QVariant
/// @param[in] data - Кодированные в Encode() данные
/// @return Исходные данные
/// @attention Предназначен для работы совместно с Encode()
QVariant Decode(const QString &data);

} // namespace SettingsEncoder

namespace NativeSettingsStorage {

namespace {

/// @brief Получить из пары (group, key) ключ, пригодный для использования с NSUserDefaults
/// @param[in] group - группа (секция) настроек
/// @param[in] key - исходный ключ в группе
/// @return ключ, пригодный для использования с NSUserDefaults
QString getNativeKeyString(const QString &group, const QString &key)
{
    return (group.isEmpty() ? QString() : (group + QString::fromLatin1("/"))) + key;
}

} // namespace

/// @brief Установить значение для заданного ключа в NSUserDefaults
/// @param[in] group - группа (секция) настроек
/// @param[in] key - ключ, для которого устанавливается значение
/// @param[in] value - значение, которое устанавливается для ключа
void setValue(const QString &group, const QString &key, const QVariant &value)
{
    AUTORELEASE_POOL;
    NSString *nativeKey = ObjCUtils::QStringToNSString(getNativeKeyString(group, key));
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *nativeValue = ObjCUtils::QStringToNSString(SettingsEncoder::Encode(value));
    [defaults setObject: nativeValue forKey: nativeKey];
    [defaults synchronize];
}

/// @brief Получить значение для заданного ключа из NSUserDefaults
/// @param[in] group - группа (секция) настроек
/// @param[in] key - ключ, для которого получается значение
/// @param[in] defaultValue - умолчательное значение, возвращается при отсутствии значения
/// @return - значение для ключа или defaultValue при отсутствии значения
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

