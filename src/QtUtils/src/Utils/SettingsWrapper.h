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

#if !defined (QTUTILS_SETTINGSWRAPPER_H_INCLUDED)
#define QTUTILS_SETTINGSWRAPPER_H_INCLUDED

#include <QString>
#include <QVariant>
#include <QPointer>

/// @brief Класс-обертка над настройками, содержит в себе кэш, сохраняемый при выходе из программы
/// @note Thread-safe
class SettingsWrapper
{
    Q_DISABLE_COPY(SettingsWrapper)

public:
    /// @brief SettingsWrapper
    /// @param[in] settingsGroup - группа (секция) настроек
    SettingsWrapper(const QString &settingsGroup = QString());

    ~SettingsWrapper();

    /// @brief Установить значение для заданного ключа
    /// @param[in] key - ключ, для которого устанавливается значение
    /// @param[in] value - значение, которое устанавливается для ключа
    void setValue(const QString &key, const QVariant &value);

    /// @brief Получить значение для заданного ключа
    /// @param[in] key - ключ, для которого получается значение
    /// @param[in] defaultValue - умолчательное значение, возвращается при отсутствии значения
    /// @return - значение для ключа или defaultValue при отсутствии значения
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /// @brief Принудительно сохранить все настройки, не дожидаясь выхода из программы
    void flush();

private:
    /// @brief Группа (секция) настроек
    const QString m_settingsGroup;

    /// @brief Глобальный кэш настроек
    class SettingsCache;
    static QPointer<SettingsCache> g_settingsCache;
};

#endif // QTUTILS_SETTINGSWRAPPER_H_INCLUDED

