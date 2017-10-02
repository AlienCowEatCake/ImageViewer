/*
   Copyright (C) 2011-2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "SettingsWrapper.h"
#include <QVariantMap>
#include <QSettings>
#include <QMutex>
#include <QApplication>
#include <QDir>
#include <QColor>

namespace SettingsEncoder {

namespace {

/// @brief Конвертер однобайтового числа в его строковое 16-ричное представление
/// @param[in] value - Однобайтовое число
/// @return Строковое 16-ричное представление исходного числа
QString ByteToHex(int value)
{
    QString result = QString::number(value, 16).toUpper();
    if(result.size() < 2)
        result.prepend(QString::fromLatin1("0"));
    return result;
}

} // namespace

/// @brief Кодировщик данных QVariant -> QString, по возможности использует человеко-читаемое представление
/// @param[in] data - Исходные данные
/// @return Кодированные данные
QString Encode(const QVariant &data)
{
    switch(data.type())
    {

    case QVariant::Bool:
    {
        const bool value = data.toBool();
        QString result = QString::fromLatin1("BOOL:");
        result += QString::fromLatin1(value ? "true" : "false");
        return result;
    }

    case QVariant::Int:
    case QVariant::LongLong:
    {
        const qlonglong value = data.toLongLong();
        QString result = QString::fromLatin1("INTEGER:");
        result += QString::number(value);
        return result;
    }

    case QVariant::UInt:
    case QVariant::ULongLong:
    {
        const qulonglong value = data.toULongLong();
        QString result = QString::fromLatin1("UNSIGNED:");
        result += QString::number(value);
        return result;
    }

    case QVariant::Char:
    case QVariant::String:
    {
        const QString value = data.toString();
        QString result = QString::fromLatin1("STRING:");
        result += value;
        return result;
    }

    case QVariant::Double:
    {
        const double value = data.toDouble();
        QString result = QString::fromLatin1("REAL:");
        result += QString::fromLatin1("%1").arg(value, 0, 'g', 16);
        return result;
    }

    case QVariant::Color:
    {
        const QColor color = data.value<QColor>();
        QString result = QString::fromLatin1("COLOR:");
        result += QString::fromLatin1("#");
        result += ByteToHex(color.red());
        result += ByteToHex(color.green());
        result += ByteToHex(color.blue());
        if(color.alpha() < 255)
            result += ByteToHex(color.alpha());
        return result;
    }

    default:
    {
        const QString value = QString::fromLatin1(data.toByteArray().toHex().data());
        QString result = QString::fromLatin1("HEXARRAY:");
        result += value;
        return result;
    }

    }
    return QString();
}

/// @brief Декодировщик данных QString -> QVariant
/// @param[in] data - Кодированные в Encode() данные
/// @return Исходные данные
/// @attention Предназначен для работы совместно с Encode()
QVariant Decode(const QString &data)
{
    const QString dataSimplified = data.simplified().toUpper();
    const int delimeter = data.indexOf(QChar::fromLatin1(':'));
    if(delimeter <= 0)
        return QVariant(QVariant::Invalid);
    QString dataValue = QString(data).remove(0, delimeter + 1);

    if(dataSimplified.startsWith(QString::fromLatin1("BOOL:")))
    {
        dataValue = dataValue.simplified().toLower();
        if(dataValue == QString::fromLatin1("no") ||
           dataValue == QString::fromLatin1("0") ||
           dataValue == QString::fromLatin1("false"))
            return QVariant(false);
        if(dataValue == QString::fromLatin1("yes") ||
           dataValue == QString::fromLatin1("1") ||
           dataValue == QString::fromLatin1("true"))
            return QVariant(true);
        return QVariant(QVariant::Invalid);
    }

    if(dataSimplified.startsWith(QString::fromLatin1("INTEGER:")))
    {
        dataValue = dataValue.simplified();
        bool ok = false;
        const qlonglong value = dataValue.toLongLong(&ok);
        if(ok)
            return QVariant(value);
        return QVariant(QVariant::Invalid);
    }

    if(dataSimplified.startsWith(QString::fromLatin1("UNSIGNED:")))
    {
        dataValue = dataValue.simplified();
        bool ok = false;
        const qulonglong value = dataValue.toULongLong(&ok);
        if(ok)
            return QVariant(value);
        return QVariant(QVariant::Invalid);
    }

    if(dataSimplified.startsWith(QString::fromLatin1("STRING:")))
    {
        return QVariant(dataValue);
    }

    if(dataSimplified.startsWith(QString::fromLatin1("REAL:")))
    {
        dataValue = dataValue.simplified();
        bool ok = false;
        const double value = dataValue.toDouble(&ok);
        if(ok)
            return QVariant(value);
        return QVariant(QVariant::Invalid);
    }

    if(dataSimplified.startsWith(QString::fromLatin1("COLOR:")))
    {
        dataValue = dataValue.simplified();
        if(dataValue.size() < 1 || dataValue[0] != QChar::fromLatin1('#'))
            return QVariant(QVariant::Invalid);
        bool ok = false;
        if(dataValue.size() == 4) // "#RGB"
        {
            const int r = dataValue.mid(1, 1).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int g = dataValue.mid(2, 1).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int b = dataValue.mid(3, 1).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            return QVariant(QColor(r, g, b));
        }
        else if(dataValue.size() == 5) // "#RGBA"
        {
            const int r = dataValue.mid(1, 1).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int g = dataValue.mid(2, 1).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int b = dataValue.mid(3, 1).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int a = dataValue.mid(4, 1).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            return QVariant(QColor(r, g, b, a));
        }
        else if(dataValue.size() == 7) // "#RRGGBB"
        {
            const int r = dataValue.mid(1, 2).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int g = dataValue.mid(3, 2).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int b = dataValue.mid(5, 2).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            return QVariant(QColor(r, g, b));
        }
        else if(dataValue.size() == 9) // "#RRGGBBAA"
        {
            const int r = dataValue.mid(1, 2).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int g = dataValue.mid(3, 2).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int b = dataValue.mid(5, 2).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            const int a = dataValue.mid(7, 2).toInt(&ok, 16);
            if(!ok) return QVariant(QVariant::Invalid);
            return QVariant(QColor(r, g, b, a));
        }
    }

    if(dataSimplified.startsWith(QString::fromLatin1("HEXARRAY:")))
    {
        dataValue = dataValue.simplified();
        return QVariant(QByteArray::fromHex(dataValue.toLatin1()));
    }

    return QVariant(QVariant::Invalid);
}

} // namespace SettingsEncoder

#if defined (Q_OS_MAC)

namespace NativeSettingsStorage {

/// @brief Установить значение для заданного ключа в NSUserDefaults
/// @param[in] group - группа (секция) настроек
/// @param[in] key - ключ, для которого устанавливается значение
/// @param[in] value - значение, которое устанавливается для ключа
/// @note Реализация в SettingsWrapper_mac.mm
void setValue(const QString &group, const QString &key, const QVariant &value);

/// @brief Получить значение для заданного ключа из NSUserDefaults
/// @param[in] group - группа (секция) настроек
/// @param[in] key - ключ, для которого получается значение
/// @param[in] defaultValue - умолчательное значение, возвращается при отсутствии значения
/// @return - значение для ключа или defaultValue при отсутствии значения
/// @note Реализация в SettingsWrapper_mac.mm
QVariant value(const QString &group, const QString &key, const QVariant &defaultValue);

} // namespace NativeSettingsStorage

#endif

namespace {

#if defined (Q_OS_MAC)

/// @brief Хранилище настроек для Mac, использует NSUserDefaults. Введен по причине глючности QSettings.
class SettingsStorage
{
    Q_DISABLE_COPY(SettingsStorage)

public:
    /// @brief Установить значение для заданного ключа в NSUserDefaults
    /// @param[in] group - группа (секция) настроек
    /// @param[in] key - ключ, для которого устанавливается значение
    /// @param[in] value - значение, которое устанавливается для ключа
    void setValue(const QString &group, const QString &key, const QVariant &value)
    {
        NativeSettingsStorage::setValue(group, key, value);
    }

    /// @brief Получить значение для заданного ключа из NSUserDefaults
    /// @param[in] group - группа (секция) настроек
    /// @param[in] key - ключ, для которого получается значение
    /// @param[in] defaultValue - умолчательное значение, возвращается при отсутствии значения
    /// @return - значение для ключа или defaultValue при отсутствии значения
    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue)
    {
        return NativeSettingsStorage::value(group, key, defaultValue);
    }
};

#else

/// @brief Универсальное хранилище настроек, использует QSettings.
class SettingsStorage
{
    Q_DISABLE_COPY(SettingsStorage)

public:
    SettingsStorage()
        : m_settings(NULL)
    {}

    ~SettingsStorage()
    {
        if(m_settings)
        {
            m_settings->sync();
            m_settings->deleteLater();
        }
    }

    /// @brief Установить значение для заданного ключа в QSettings
    /// @param[in] group - группа (секция) настроек
    /// @param[in] key - ключ, для которого устанавливается значение
    /// @param[in] value - значение, которое устанавливается для ключа
    void setValue(const QString &group, const QString &key, const QVariant &value)
    {
        if(!m_settings)
            initStorage();
        const bool useGroup = !group.isEmpty();
        if(useGroup)
            m_settings->beginGroup(group);
        m_settings->setValue(key, SettingsEncoder::Encode(value));
        if(useGroup)
            m_settings->endGroup();
    }

    /// @brief Получить значение для заданного ключа из QSettings
    /// @param[in] group - группа (секция) настроек
    /// @param[in] key - ключ, для которого получается значение
    /// @param[in] defaultValue - умолчательное значение, возвращается при отсутствии значения
    /// @return - значение для ключа или defaultValue при отсутствии значения
    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue)
    {
        if(!m_settings)
            initStorage();
        m_settings->beginGroup(group);
        const QVariant value = SettingsEncoder::Decode(m_settings->value(key, SettingsEncoder::Encode(defaultValue)).toString());
        m_settings->endGroup();
        return value;
    }

private:
    /// @brief Инициализация QSettings
    void initStorage()
    {
        if(!m_settings)
        {
#if defined (Q_OS_MAC)
            const QSettings::Format format = QSettings::IniFormat;
            const QString organizationName = QApplication::organizationName();
            const QString applicationName = QApplication::applicationName();
            QSettings::setPath(format, QSettings::UserScope,
                               QString::fromLatin1("%1/Library/Application Support").arg(QDir::homePath()));
#else
            const QSettings::Format format = QSettings::NativeFormat;
            const QString organizationName = QApplication::organizationName();
            const QString applicationName = QApplication::applicationName();
#endif
            m_settings = new QSettings(format, QSettings::UserScope, organizationName, applicationName);
            m_settings->setFallbacksEnabled(false);
        }
    }

    QSettings* m_settings;
};

#endif

} // namespace

/// @brief Кэш настроек
class SettingsWrapper::SettingsCache : public QObject
{
    Q_DISABLE_COPY(SettingsCache)

public:
    SettingsCache()
        : m_isConnectedToQApp(false)
    {}

    ~SettingsCache()
    {
        if(m_isConnectedToQApp)
            saveSettings();
    }

    /// @brief Установить значение для заданного ключа
    /// @param[in] group - группа (секция) настроек
    /// @param[in] key - ключ, для которого устанавливается значение
    /// @param[in] value - значение, которое устанавливается для ключа
    void setValue(const QString &group, const QString &key, const QVariant &value)
    {
        m_settingsMutex.lock();
        CheckOrConnectToQApp();
        m_settingsCache[group][key] = value;
        m_settingsMutex.unlock();
    }

    /// @brief Получить значение для заданного ключа
    /// @param[in] group - группа (секция) настроек
    /// @param[in] key - ключ, для которого получается значение
    /// @param[in] defaultValue - умолчательное значение, возвращается при отсутствии значения
    /// @return - значение для ключа или defaultValue при отсутствии значения
    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue)
    {
        m_settingsMutex.lock();
        CheckOrConnectToQApp();
        QVariant retValue = defaultValue;
        bool foundInCache = false;
        QMap<QString, QVariantMap>::Iterator groupIter = m_settingsCache.find(group);
        if(groupIter != m_settingsCache.end())
        {
            QVariantMap::Iterator valueIter = groupIter->find(key);
            if(valueIter != groupIter->end())
            {
                retValue = valueIter.value();
                foundInCache = true;
            }
        }
        if(!foundInCache)
        {
            QVariant newValue = m_settingsStorage.value(group, key, defaultValue);
            if(newValue.isValid())
            {
                m_settingsCache[group][key] = newValue;
                retValue = newValue;
            }
        }
        m_settingsMutex.unlock();
        return retValue;
    }

private:
    /// @brief Сброс настроек в SettingsStorage
    void saveSettings()
    {
        m_settingsMutex.lock();
        for(QMap<QString, QVariantMap>::Iterator group = m_settingsCache.begin(); group != m_settingsCache.end(); ++group)
        {
            for(QVariantMap::Iterator value = group->begin(); value != group->end(); ++value)
                m_settingsStorage.setValue(group.key(), value.key(), value.value());
        }
        m_settingsMutex.unlock();
    }

    /// @brief Проверяет, соединен ли кэш настроек с QApplication, если нет - выполняет соединение
    void CheckOrConnectToQApp()
    {
        if(!m_isConnectedToQApp && qApp)
        {
            connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
            m_isConnectedToQApp = true;
        }
    }

    /// @brief Флаг соединен ли кэш настроек с QApplication
    bool m_isConnectedToQApp;
    /// @brief Мьютекс, блокирующий доступ к кэшу из разных потоков
    QMutex m_settingsMutex;
    /// @brief Платформозависимое хранилище настроек
    SettingsStorage m_settingsStorage;
    /// @brief Кэш настроек, QMap<Group, QMap<Key, Value>>
    QMap<QString, QVariantMap> m_settingsCache;
};

/// @brief Глобальный кэш настроек
QPointer<SettingsWrapper::SettingsCache> SettingsWrapper::g_settingsCache = new SettingsCache;


/// @brief SettingsWrapper
/// @param[in] settingsGroup - группа (секция) настроек
SettingsWrapper::SettingsWrapper(const QString &settingsGroup)
    : m_settingsGroup(settingsGroup)
{}

SettingsWrapper::~SettingsWrapper()
{}

/// @brief Установить значение для заданного ключа
/// @param[in] key - ключ, для которого устанавливается значение
/// @param[in] value - значение, которое устанавливается для ключа
void SettingsWrapper::setValue(const QString &key, const QVariant &value)
{
    if(!g_settingsCache.isNull())
        g_settingsCache->setValue(m_settingsGroup, key, value);
}

/// @brief Получить значение для заданного ключа
/// @param[in] key - ключ, для которого получается значение
/// @param[in] defaultValue - умолчательное значение, возвращается при отсутствии значения
/// @return - значение для ключа или defaultValue при отсутствии значения
QVariant SettingsWrapper::value(const QString &key, const QVariant &defaultValue) const
{
    return g_settingsCache.isNull() ? defaultValue : g_settingsCache->value(m_settingsGroup, key, defaultValue);
}

