/*
   Copyright (C) 2018-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_THEMEMANAGER_H_INCLUDED)
#define QTUTILS_THEMEMANAGER_H_INCLUDED

#include <QObject>
#include <QString>
#include <QStringList>

#include "Global.h"
#include "ScopedPointer.h"

class QMenu;
class QAction;
class QComboBox;

/// @brief Manager for themes.
class ThemeManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ThemeManager)

Q_SIGNALS:
    /// @brief Signal about theme changing.
    /// @param themeId - New theme identifier.
    void themeChanged(const QString &themeId);

public:
    /// @brief Get pointer to current theme manager.
    /// @return Pointer to current theme manager.
    static ThemeManager *instance();

    /// @brief Register theme.
    /// @param themeId - Unique identifier for theme.
    /// @param styleSheets - List of paths to QSS files with style sheets.
    /// @param translateContext - Translation context. String with themeId
    /// key should be present in this context for localized theme name.
    /// @param isDefault - Theme is default. Only one theme may be default.
    void registerTheme(const QString &themeId, const QStringList &styleSheets,
                       const QString &translateContext, const bool isDefault = false);

    /// @brief Apply current theme. Only one time usage on application start
    /// is allowed, otherwise effect can not be guaranteed.
    void applyCurrentTheme();

    /// @brief Get current theme identifier.
    /// @return Current theme identifier.
    QString currentTheme() const;

    /// @brief Set current theme with specified identifier. Theme will not be
    /// applied until you call applyCurrentTheme().
    /// @param themeId - Theme identifier.
    /// @param showMessage - Show message that theme will be applied only after
    /// application restart.
    /// @param parent - Parent for message shown if showMessage=true.
    void setTheme(const QString &themeId, const bool showMessage = false, QWidget *parent = Q_NULLPTR);

    /// @brief Fill menu with items for theme choose. All required connections
    /// will be setup automatically.
    /// @param menu - Menu which should be filled.
    void fillMenu(QMenu *menu);

    /// @brief Fill combo box with items for theme choose. All required
    /// connections will be setup automatically.
    /// @param comboBox - Combo box which should be filled.
    /// @param autoApply - Apply changes automatically after combo box changes.
    void fillComboBox(QComboBox *comboBox, const bool autoApply = true);

private:
    ThemeManager();
    ~ThemeManager();

private Q_SLOTS:
    void onActionTriggered(QAction *action);
    void onComboBoxActivated(int index);
    void onActionDestroyed(QObject *object);
    void onComboBoxDestroyed(QObject *object);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QTUTILS_THEMEMANAGER_H_INCLUDED

