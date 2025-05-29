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

#include "SettingsWrapper.h"
#include <QVariantMap>
#include <QSettings>
#include <QMutex>
#include <QApplication>
#include <QDir>
#include <QColor>

#include "Global.h"

namespace SettingsEncoder {

namespace {

/// @brief Convert 1-byte number to hex string
/// @param[in] value - 1-byte number
/// @return Hex string representation of value
QString ByteToHex(int value)
{
    QString result = QString::number(value, 16).toUpper();
    if(result.size() < 2)
        result.prepend(QString::fromLatin1("0"));
    return result;
}

} // namespace

/// @brief Encoder from QVariant to QString, uses human-readable representation if possible
/// @param[in] data - data to encode
/// @return Encoded data
QString Encode(const QVariant &data)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    switch(data.typeId())
#else
    switch(data.type())
#endif
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    case QMetaType::Bool:
#else
    case QVariant::Bool:
#endif
    {
        const bool value = data.toBool();
        QString result = QString::fromLatin1("BOOL:");
        result += QString::fromLatin1(value ? "true" : "false");
        return result;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
#else
    case QVariant::Int:
    case QVariant::LongLong:
#endif
    {
        const qlonglong value = data.toLongLong();
        QString result = QString::fromLatin1("INTEGER:");
        result += QString::number(value);
        return result;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
#else
    case QVariant::UInt:
    case QVariant::ULongLong:
#endif
    {
        const qulonglong value = data.toULongLong();
        QString result = QString::fromLatin1("UNSIGNED:");
        result += QString::number(value);
        return result;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    case QMetaType::QChar:
    case QMetaType::QString:
#else
    case QVariant::Char:
    case QVariant::String:
#endif
    {
        const QString value = data.toString();
        QString result = QString::fromLatin1("STRING:");
        result += value;
        return result;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    case QMetaType::Float:
    case QMetaType::Double:
#else
    case QVariant::Double:
#endif
    {
        const double value = data.toDouble();
        QString result = QString::fromLatin1("REAL:");
        result += QString::fromLatin1("%1").arg(value, 0, 'g', 16);
        return result;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    case QMetaType::QColor:
#else
    case QVariant::Color:
#endif
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

/// @brief Decoder from QString to QVariant
/// @param[in] data - encoded data from Encode()
/// @return Decoded data
/// @attention Suitable only for encoded data from Encode()
QVariant Decode(const QString &data)
{
    const QString dataSimplified = data.simplified().toUpper();
    const int delimeter = data.indexOf(QChar::fromLatin1(':'));
    if(delimeter <= 0)
        return QVariant();
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
        return QVariant();
    }

    if(dataSimplified.startsWith(QString::fromLatin1("INTEGER:")))
    {
        dataValue = dataValue.simplified();
        bool ok = false;
        const qlonglong value = dataValue.toLongLong(&ok);
        if(ok)
            return QVariant(value);
        return QVariant();
    }

    if(dataSimplified.startsWith(QString::fromLatin1("UNSIGNED:")))
    {
        dataValue = dataValue.simplified();
        bool ok = false;
        const qulonglong value = dataValue.toULongLong(&ok);
        if(ok)
            return QVariant(value);
        return QVariant();
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
        return QVariant();
    }

    if(dataSimplified.startsWith(QString::fromLatin1("COLOR:")))
    {
        dataValue = dataValue.simplified();
        if(dataValue.size() < 1 || dataValue[0] != QChar::fromLatin1('#'))
            return QVariant();
        bool ok = false;
        if(dataValue.size() == 4) // "#RGB"
        {
            const int r = dataValue.mid(1, 1).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int g = dataValue.mid(2, 1).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int b = dataValue.mid(3, 1).toInt(&ok, 16);
            if(!ok) return QVariant();
            return QVariant(QColor(r, g, b));
        }
        else if(dataValue.size() == 5) // "#RGBA"
        {
            const int r = dataValue.mid(1, 1).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int g = dataValue.mid(2, 1).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int b = dataValue.mid(3, 1).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int a = dataValue.mid(4, 1).toInt(&ok, 16);
            if(!ok) return QVariant();
            return QVariant(QColor(r, g, b, a));
        }
        else if(dataValue.size() == 7) // "#RRGGBB"
        {
            const int r = dataValue.mid(1, 2).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int g = dataValue.mid(3, 2).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int b = dataValue.mid(5, 2).toInt(&ok, 16);
            if(!ok) return QVariant();
            return QVariant(QColor(r, g, b));
        }
        else if(dataValue.size() == 9) // "#RRGGBBAA"
        {
            const int r = dataValue.mid(1, 2).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int g = dataValue.mid(3, 2).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int b = dataValue.mid(5, 2).toInt(&ok, 16);
            if(!ok) return QVariant();
            const int a = dataValue.mid(7, 2).toInt(&ok, 16);
            if(!ok) return QVariant();
            return QVariant(QColor(r, g, b, a));
        }
    }

    if(dataSimplified.startsWith(QString::fromLatin1("HEXARRAY:")))
    {
        dataValue = dataValue.simplified();
        return QVariant(QByteArray::fromHex(dataValue.toLatin1()));
    }

    return QVariant();
}

} // namespace SettingsEncoder

#if defined (Q_OS_MAC)

namespace NativeSettingsStorage {

/// @brief Set value to NSUserDefaults for specified key and group
/// @param[in] group - group (section or prefix) of settings
/// @param[in] key - key for set
/// @param[in] value - value for for set
/// @note Implementation can be found in SettingsWrapper_mac.mm
void setValue(const QString &group, const QString &key, const QVariant &value);

/// @brief Get value from NSUserDefaults for specified key and group
/// @param[in] group - group (section or prefix) of settings
/// @param[in] key - key for get
/// @param[in] defaultValue - default value if value is absent
/// @return - value for specified key or defaultValue if value is absent
/// @note Implementation can be found in SettingsWrapper_mac.mm
QVariant value(const QString &group, const QString &key, const QVariant &defaultValue);

} // namespace NativeSettingsStorage

#endif

namespace {

#if defined (Q_OS_MAC)

/// @brief Settings storage for macOS implemented via NSUserDefaults.
/// @note It was introduced because QSettings is too buggy on older Qt versions.
/// It is still used for modern versions of Qt for compatibility reasons.
class SettingsStorage
{
    Q_DISABLE_COPY(SettingsStorage)

public:
    SettingsStorage()
    {}

    /// @brief Set value to NSUserDefaults for specified key and group
    /// @param[in] group - group (section or prefix) of settings
    /// @param[in] key - key for set
    /// @param[in] value - value for for set
    void setValue(const QString &group, const QString &key, const QVariant &value)
    {
        NativeSettingsStorage::setValue(group, key, value);
    }

    /// @brief Get value from NSUserDefaults for specified key and group
    /// @param[in] group - group (section or prefix) of settings
    /// @param[in] key - key for get
    /// @param[in] defaultValue - default value if value is absent
    /// @return - value for specified key or defaultValue if value is absent
    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue)
    {
        return NativeSettingsStorage::value(group, key, defaultValue);
    }
};

#else

/// @brief Generic settings storage implemented via QSettings.
class SettingsStorage
{
    Q_DISABLE_COPY(SettingsStorage)

public:
    SettingsStorage()
        : m_settings(Q_NULLPTR)
    {}

    ~SettingsStorage()
    {
        if(m_settings)
        {
            m_settings->sync();
            m_settings->deleteLater();
        }
    }

    /// @brief Set value to QSettings for specified key and group
    /// @param[in] group - group (section or prefix) of settings
    /// @param[in] key - key for set
    /// @param[in] value - value for for set
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

    /// @brief Get value from QSettings for specified key and group
    /// @param[in] group - group (section or prefix) of settings
    /// @param[in] key - key for get
    /// @param[in] defaultValue - default value if value is absent
    /// @return - value for specified key or defaultValue if value is absent
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
    /// @brief Initialize QSettings
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

/// @brief Settings cache
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

    /// @brief Set value for specified key and group
    /// @param[in] group - group (section or prefix) of settings
    /// @param[in] key - key for set
    /// @param[in] value - value for for set
    void setValue(const QString &group, const QString &key, const QVariant &value)
    {
        m_settingsMutex.lock();
        CheckOrConnectToQApp();
        m_settingsCache[group][key] = value;
        m_settingsMutex.unlock();
    }

    /// @brief Get value for specified key and group
    /// @param[in] group - group (section or prefix) of settings
    /// @param[in] key - key for get
    /// @param[in] defaultValue - default value if value is absent
    /// @return - value for specified key or defaultValue if value is absent
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

    /// @brief Immediately flush all settings to SettingsStorage
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

private:
    /// @brief Check connection to QApplication and setup it if necessary
    void CheckOrConnectToQApp()
    {
        if(!m_isConnectedToQApp && qApp)
        {
            connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
            m_isConnectedToQApp = true;
        }
    }

    /// @brief QApplication connection flag
    bool m_isConnectedToQApp;
    /// @brief Mutex which prevents cache from being accessed by multiple threads
    QMutex m_settingsMutex;
    /// @brief Platform dependent settings storage
    SettingsStorage m_settingsStorage;
    /// @brief Settings cache, QMap<Group, QMap<Key, Value> >
    QMap<QString, QVariantMap> m_settingsCache;
};

/// @brief Global settings cache
QPointer<SettingsWrapper::SettingsCache> SettingsWrapper::g_settingsCache = new SettingsCache;


/// @brief SettingsWrapper
/// @param[in] settingsGroup - group (section or prefix) of settings
SettingsWrapper::SettingsWrapper(const QString &settingsGroup)
    : m_settingsGroup(settingsGroup)
{}

SettingsWrapper::~SettingsWrapper()
{}

/// @brief Set value for specified key
/// @param[in] key - key for set
/// @param[in] value - value for for set
void SettingsWrapper::setValue(const QString &key, const QVariant &value)
{
    if(!g_settingsCache.isNull())
        g_settingsCache->setValue(m_settingsGroup, key, value);
}

/// @brief Get value for specified key
/// @param[in] key - key for get
/// @param[in] defaultValue - default value if value is absent
/// @return - value for specified key or defaultValue if value is absent
QVariant SettingsWrapper::value(const QString &key, const QVariant &defaultValue) const
{
    return g_settingsCache.isNull() ? defaultValue : g_settingsCache->value(m_settingsGroup, key, defaultValue);
}

/// @brief Immediately flush all settings from cache to disk or registry
void SettingsWrapper::flush()
{
    if(!g_settingsCache.isNull())
        g_settingsCache->saveSettings();
}

