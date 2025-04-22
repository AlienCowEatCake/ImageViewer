/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_LOCALIZATION_MANAGER_H_INCLUDED)
#define QTUTILS_LOCALIZATION_MANAGER_H_INCLUDED

#include <QObject>
#include <QStringList>
#include "ScopedPointer.h"

class QMenu;
class QAction;
class QComboBox;

namespace Locale {

extern const QString EN;
extern const QString RU;
extern const QString ZH_CN;
extern const QString ZH_TW;

} // namespace Locale

/// @brief Manager for localization.
class LocalizationManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(LocalizationManager)

Q_SIGNALS:
    /// @brief Signal about locale changing.
    /// @param locale - New locale identifier.
    void localeChanged(const QString &locale);

public:
    /// @brief Get pointer to current localization manager.
    /// @return Pointer to current localization manager.
    static LocalizationManager *instance();

    /// @brief Initialize translators for specified ".qm" translation files.
    /// @param templatesList - paths for ".qm" translation files, e.g.
    /// ":/translations/qtutils_%1" where %1 is place-marker for locale.
    void initializeResources(const QStringList &templatesList);

    /// @brief Get current locale identifier.
    /// @return Current locale identifier.
    QString locale() const;

    /// @brief Set current locale with specified identifier.
    /// @param locale - Locale identifier.
    void setLocale(const QString &locale = QString());

    /// @brief Fill menu with items for locale choose. All required
    /// connections will be setup automatically.
    /// @param menu - Menu which should be filled.
    void fillMenu(QMenu *menu);

    /// @brief Fill combo box with items for locale choose. All required
    /// connections will be setup automatically.
    /// @param comboBox - Combo box which should be filled.
    /// @param autoApply - Apply changes automatically after combo box changes.
    void fillComboBox(QComboBox *comboBox, const bool autoApply = true);

private:
    LocalizationManager();
    ~LocalizationManager();

private Q_SLOTS:
    void onActionTriggered(QAction *action);
    void onComboBoxActivated(int index);
    void onActionDestroyed(QObject *object);
    void onComboBoxDestroyed(QObject *object);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QTUTILS_LOCALIZATION_MANAGER_H_INCLUDED

