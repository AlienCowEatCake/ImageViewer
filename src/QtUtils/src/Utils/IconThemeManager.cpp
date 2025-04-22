/*
   Copyright (C) 2024-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "IconThemeManager.h"

#include <cassert>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QIcon>
#include <QMap>
#include <QMenu>

#include "SettingsWrapper.h"
#include "SignalBlocker.h"
#if defined (Q_OS_MAC)
#include "ThemeUtils_mac.h"
#endif

namespace {

const QString SETTINGS_KEY = QString::fromLatin1("IconTheme");

struct ThemeData
{
    QString themeId;
    QStringList iconThemeSearchPaths;
    QString translateContext;
    bool isDefault;

    ThemeData()
        : isDefault(false)
    {}

    ThemeData(const QString &themeId, const QStringList &iconThemeSearchPaths,
              const QString &translateContext, const bool isDefault)
        : themeId(themeId)
        , iconThemeSearchPaths(iconThemeSearchPaths)
        , translateContext(translateContext)
        , isDefault(isDefault)
    {}
};

typedef QList<QAction*> ActionList;
typedef QMap<QString, ActionList> ActionListMap;

} // namespace

/// @brief Icon theme identifier for default QtUtils icons
const QString IconThemeManager::QTUTILS_THEME_ID = QString::fromLatin1(QT_TRANSLATE_NOOP("IconThemes", "QtUtils"));
/// @brief Icon theme identifier for system theme icons
const QString IconThemeManager::SYSTEM_THEME_ID = QString::fromLatin1(QT_TRANSLATE_NOOP("IconThemes", "System"));

struct IconThemeManager::Impl
{
    QList<ThemeData> themes;
    SettingsWrapper settings;

    ActionListMap actionsMap;
    QList<QComboBox*> comboBoxList;

    ThemeData *currentTheme()
    {
        const QString themeString = settings.value(SETTINGS_KEY).toString();
        if(!themeString.isEmpty())
        {
            for(QList<ThemeData>::Iterator it = themes.begin(), itEnd = themes.end(); it != itEnd; ++it)
                if(themeString.compare(it->themeId, Qt::CaseInsensitive) == 0)
                    return &(*it);
        }
        for(QList<ThemeData>::Iterator it = themes.begin(), itEnd = themes.end(); it != itEnd; ++it)
            if(it->isDefault)
                return &(*it);
        return Q_NULLPTR;
    }

    void updateActions()
    {
        const ThemeData *theme = currentTheme();
        if(theme)
        {
            ActionList &actions = actionsMap[theme->themeId];
            for(ActionList::Iterator it = actions.begin(), itEnd = actions.end(); it != itEnd; ++it)
                (*it)->setChecked(true);
        }

        for(QList<ThemeData>::ConstIterator it = themes.constBegin(), itEnd = themes.constEnd(); it != itEnd; ++it)
        {
            ActionList &actions = actionsMap[it->themeId];
            for(ActionList::Iterator jt = actions.begin(), jtEnd = actions.end(); jt != jtEnd; ++jt)
            {
                const QByteArray context = it->translateContext.toLatin1();
                const QByteArray key = it->themeId.toLatin1();
                (*jt)->setText(QApplication::translate(context.data(), key.data()));
            }
        }
    }

    void updateComboBoxes()
    {
        QMap<QString, QString> itemTexts;
        for(QList<ThemeData>::ConstIterator it = themes.constBegin(), itEnd = themes.constEnd(); it != itEnd; ++it)
        {
            const QByteArray context = it->translateContext.toLatin1();
            const QByteArray key = it->themeId.toLatin1();
            itemTexts[it->themeId] = QApplication::translate(context.data(), key.data());
        }
        const ThemeData *theme = currentTheme();
        for(QList<QComboBox*>::Iterator it = comboBoxList.begin(), itEnd = comboBoxList.end(); it != itEnd; ++it)
        {
            QComboBox *comboBox = *it;
            QSignalBlocker blocker(comboBox);
            comboBox->clear();
            comboBox->setEditable(false);
            for(QList<ThemeData>::ConstIterator jt = themes.constBegin(), jtEnd = themes.constEnd(); jt != jtEnd; ++jt)
                comboBox->addItem(itemTexts[jt->themeId], jt->themeId);
            if(theme)
                comboBox->setCurrentIndex(comboBox->findData(theme->themeId));
        }
    }
};

/// @brief Get pointer to current icon theme manager.
/// @return Pointer to current icon theme manager.
IconThemeManager *IconThemeManager::instance()
{
    static IconThemeManager manager;
    return &manager;
}

/// @brief Register icon theme.
/// @param themeId - Unique identifier for icon theme.
/// @param iconThemeSearchPaths - List of paths to icon theme files.
/// See QIcon::themeSearchPaths() documentation for details.
/// @param translateContext - Translation context. String with themeId
/// key should be present in this context for localized icon theme name.
/// @param isDefault - Icon theme is default. Only one icon theme may
/// be default.
void IconThemeManager::registerTheme(const QString &themeId, const QStringList &iconThemeSearchPaths,
                                 const QString &translateContext, const bool isDefault)
{
    QStringList updatedIconThemeSearchPaths = iconThemeSearchPaths;
    QString updatedTranslateContext = translateContext;
    if(themeId == QTUTILS_THEME_ID || themeId == SYSTEM_THEME_ID)
    {
        if(updatedTranslateContext.isEmpty())
            updatedTranslateContext = QString::fromLatin1("IconThemes");
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
        if(updatedIconThemeSearchPaths.isEmpty())
            updatedIconThemeSearchPaths = QIcon::themeSearchPaths();
#endif
    }
    m_impl->themes.append(ThemeData(themeId, updatedIconThemeSearchPaths, updatedTranslateContext, isDefault));
}

/// @brief Get current icon theme identifier.
/// @return Current icon theme identifier.
QString IconThemeManager::currentTheme() const
{
    const ThemeData *currentTheme = m_impl->currentTheme();
    if(currentTheme)
        return currentTheme->themeId;
    return QString();
}

/// @brief Set current icon theme with specified identifier.
/// @param themeId - Icon theme identifier.
void IconThemeManager::setTheme(const QString &themeId)
{
    QList<ThemeData>::Iterator theme = m_impl->themes.end();
    for(QList<ThemeData>::Iterator it = m_impl->themes.begin(), itEnd = m_impl->themes.end(); it != itEnd && theme == itEnd; ++it)
        if(it->themeId == themeId)
            theme = it;
    if(theme == m_impl->themes.end())
        return;

    m_impl->settings.setValue(SETTINGS_KEY, theme->themeId);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    QIcon::setThemeSearchPaths(theme->iconThemeSearchPaths);
#endif
    m_impl->updateActions();
    m_impl->updateComboBoxes();
    Q_EMIT themeChanged(theme->themeId);
}

/// @brief Fill menu with items for icon theme choose. All required
/// connections will be setup automatically.
/// @param menu - Menu which should be filled.
void IconThemeManager::fillMenu(QMenu *menu)
{
    if(!menu)
        return;

    QActionGroup *actionGroup = new QActionGroup(menu);
    actionGroup->setExclusive(true);
    connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(onActionTriggered(QAction*)));

    for(QList<ThemeData>::ConstIterator it = m_impl->themes.constBegin(), itEnd = m_impl->themes.constEnd(); it != itEnd; ++it)
    {
        QAction *action = new QAction(menu);
        menu->addAction(action);
        action->setMenuRole(QAction::NoRole);
        action->setCheckable(true);
        actionGroup->addAction(action);
        connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(onActionDestroyed(QObject*)));
        action->setData(it->themeId);
        m_impl->actionsMap[it->themeId].append(action);
    }
    m_impl->updateActions();
}

/// @brief Fill combo box with items for icon theme choose. All required
/// connections will be setup automatically.
/// @param comboBox - Combo box which should be filled.
/// @param autoApply - Apply changes automatically after combo box changes.
void IconThemeManager::fillComboBox(QComboBox *comboBox, const bool autoApply)
{
    if(!comboBox)
        return;

    m_impl->comboBoxList.append(comboBox);
    if(autoApply)
        connect(comboBox, SIGNAL(activated(int)), this, SLOT(onComboBoxActivated(int)));
    connect(comboBox, SIGNAL(destroyed(QObject*)), this, SLOT(onComboBoxDestroyed(QObject*)));
    m_impl->updateComboBoxes();
}

/// @brief Get icon for current icon theme.
/// @param fallbackRequired - Icon is strictly required and can't be empty.
/// @param darkBackground - true if icon should be placed on dark background.
/// @return Icon for current icon theme.
QIcon IconThemeManager::GetIcon(ThemeUtils::IconTypes type, bool fallbackRequired, bool darkBackground) const
{
    const ThemeData *currentTheme = m_impl->currentTheme();
#if defined (Q_OS_MAC) || defined (Q_OS_WIN)
    if(currentTheme && currentTheme->themeId == SYSTEM_THEME_ID)
    {
        QList<QImage> iconImages;
        static const QList<int> iconSizes = QList<int>() << 16 << 32;
        for(QList<int>::ConstIterator it = iconSizes.begin(); it != iconSizes.end(); ++it)
        {
#if defined (Q_OS_MAC)
            QImage image = ThemeUtils::GetMacSystemImage(type, QSize(*it, *it));
#elif defined (Q_OS_WIN)
            QImage image = ThemeUtils::GetWinSystemImage(type, QSize(*it, *it));
#endif
            if(!image.isNull())
                iconImages.append(image);
        }
        if(!iconImages.isEmpty())
        {
            const QIcon result = ThemeUtils::CreateScalableIcon(iconImages, darkBackground);
            if(!result.isNull())
                return result;
        }
        // Symbolic monochrome icons are used as system icon theme for Windows and macOS.
        // So we can safely use QtUtils monochrome theme as fallback
        return ThemeUtils::GetIcon(type, darkBackground);
    }
#endif
    if(currentTheme && currentTheme->themeId != QTUTILS_THEME_ID)
    {
        const QIcon result = ThemeUtils::GetThemeIcon(type);
        if(!result.isNull() || !fallbackRequired)
            return result;
    }
    return ThemeUtils::GetIcon(type, darkBackground);
}

IconThemeManager::IconThemeManager()
    : m_impl(new Impl)
{}

IconThemeManager::~IconThemeManager()
{}

void IconThemeManager::onActionTriggered(QAction *action)
{
    assert(action);
    setTheme(action->data().toString());
}

void IconThemeManager::onComboBoxActivated(int index)
{
    Q_UNUSED(index);
    QComboBox *comboBox = static_cast<QComboBox*>(sender());
    assert(comboBox);
    setTheme(comboBox->itemData(comboBox->currentIndex()).toString());
}

void IconThemeManager::onActionDestroyed(QObject *object)
{
    QAction *action = static_cast<QAction*>(object);
    assert(action);
    for(ActionListMap::Iterator it = m_impl->actionsMap.begin(), itEnd = m_impl->actionsMap.end(); it != itEnd; ++it)
        it.value().removeAll(action);
}

void IconThemeManager::onComboBoxDestroyed(QObject *object)
{
    QComboBox *comboBox = static_cast<QComboBox*>(object);
    assert(comboBox);
    m_impl->comboBoxList.removeAll(comboBox);
}

