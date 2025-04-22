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

#if !defined (QTUTILS_SETTINGSWRAPPER_H_INCLUDED)
#define QTUTILS_SETTINGSWRAPPER_H_INCLUDED

#include <QString>
#include <QVariant>
#include <QPointer>

/// @brief Wrapper around settings which provides in-memory cache which will be flushed on app exit
/// @note Thread-safe
class SettingsWrapper
{
    Q_DISABLE_COPY(SettingsWrapper)

public:
    /// @brief SettingsWrapper
    /// @param[in] settingsGroup - group (section or prefix) of settings
    explicit SettingsWrapper(const QString &settingsGroup = QString());

    ~SettingsWrapper();

    /// @brief Set value for specified key
    /// @param[in] key - key for set
    /// @param[in] value - value for for set
    void setValue(const QString &key, const QVariant &value);

    /// @brief Get value for specified key
    /// @param[in] key - key for get
    /// @param[in] defaultValue - default value if value is absent
    /// @return - value for specified key or defaultValue if value is absent
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /// @brief Immediately flush all settings from cache to disk or registry
    void flush();

private:
    /// @brief Group (section or prefix) of settings
    const QString m_settingsGroup;

    /// @brief Global settings cache
    class SettingsCache;
    static QPointer<SettingsCache> g_settingsCache;
};

#endif // QTUTILS_SETTINGSWRAPPER_H_INCLUDED

