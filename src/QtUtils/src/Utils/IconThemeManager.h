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

#if !defined (QTUTILS_ICONTHEMEMANAGER_H_INCLUDED)
#define QTUTILS_ICONTHEMEMANAGER_H_INCLUDED

#include <QString>
#include <QStringList>
#include <QObject>

#include "Global.h"
#include "ScopedPointer.h"
#include "ThemeUtils.h"

class QAction;
class QComboBox;
class QIcon;
class QMenu;

/// @brief Manager for icon themes.
class IconThemeManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IconThemeManager)

Q_SIGNALS:
    /// @brief Signal about icon theme changing.
    /// @param themeId - New icon theme identifier.
    void themeChanged(const QString &themeId);

public:
    /// @brief Icon theme identifier for default QtUtils icons
    static const QString QTUTILS_THEME_ID;
    /// @brief Icon theme identifier for system theme icons
    static const QString SYSTEM_THEME_ID;

    /// @brief Get pointer to current icon theme manager.
    /// @return Pointer to current icon theme manager.
    static IconThemeManager *instance();

    /// @brief Register icon theme.
    /// @param themeId - Unique identifier for icon theme.
    /// @param iconThemeSearchPaths - List of paths to icon theme files.
    /// See QIcon::themeSearchPaths() documentation for details.
    /// @param translateContext - Translation context. String with themeId
    /// key should be present in this context for localized icon theme name.
    /// @param isDefault - Icon theme is default. Only one icon theme may
    /// be default.
    void registerTheme(const QString &themeId, const QStringList &iconThemeSearchPaths,
                       const QString &translateContext, const bool isDefault = false);

    /// @brief Get current icon theme identifier.
    /// @return Current icon theme identifier.
    QString currentTheme() const;

    /// @brief Set current icon theme with specified identifier.
    /// @param themeId - Icon theme identifier.
    void setTheme(const QString &themeId);

    /// @brief Fill menu with items for icon theme choose. All required
    /// connections will be setup automatically.
    /// @param menu - Menu which should be filled.
    void fillMenu(QMenu *menu);

    /// @brief Fill combo box with items for icon theme choose. All required
    /// connections will be setup automatically.
    /// @param menu - Combo box which should be filled.
    /// @param autoApply - Apply changes automatically after combo box changes.
    void fillComboBox(QComboBox *comboBox, const bool autoApply = true);

    /// @brief Get icon for current icon theme.
    /// @param fallbackRequired - Icon is strictly required and can't be empty.
    /// @param darkBackground - true if icon should be placed on dark background.
    /// @return Icon for current icon theme.
    QIcon GetIcon(ThemeUtils::IconTypes type, bool fallbackRequired = false, bool darkBackground = false) const;

private:
    IconThemeManager();
    ~IconThemeManager();

private Q_SLOTS:
    void onActionTriggered(QAction *action);
    void onComboBoxActivated(int index);
    void onActionDestroyed(QObject *object);
    void onComboBoxDestroyed(QObject *object);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QTUTILS_ICONTHEMEMANAGER_H_INCLUDED

