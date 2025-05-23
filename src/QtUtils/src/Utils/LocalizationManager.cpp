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

#include "LocalizationManager.h"

#include <cassert>

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QList>
#include <QMap>
#include <QPair>

#include "SettingsWrapper.h"
#include "SignalBlocker.h"
#include "Workarounds.h"

namespace Locale {

const QString EN = QString::fromLatin1("en");
const QString RU = QString::fromLatin1("ru");
const QString ZH_CN = QString::fromLatin1("zh_CN");
const QString ZH_TW = QString::fromLatin1("zh_TW");

} // namespace Locale

namespace {

const QString SETTINGS_KEY = QString::fromLatin1("Language");

typedef QList<QAction*> ActionList;
typedef QMap<QString, ActionList> ActionListMap;
typedef QList<QTranslator*> TranslatorList;

QString findBestLocaleForSystem(const QStringList& localesList)
{
    const QString systemLocale = QLocale::system().name().replace(QChar::fromLatin1('-'), QChar::fromLatin1('_'));
    for(QStringList::ConstIterator it = localesList.constBegin(), itEnd = localesList.constEnd(); it != itEnd; ++it)
        if(systemLocale.startsWith(*it, Qt::CaseInsensitive))
            return *it;
    if(systemLocale.startsWith(QString::fromLatin1("zh_Hans"), Qt::CaseInsensitive))
        return Locale::ZH_CN;
    if(systemLocale.startsWith(QString::fromLatin1("zh_Hant"), Qt::CaseInsensitive))
        return Locale::ZH_TW;
    return Locale::EN;
}

QString getLocaleFallbackDescription(const QString &localeName)
{
    const QLocale locale(localeName);
    if(locale.language() == QLocale::C)
        return localeName;

    const QString language = QLocale::languageToString(locale.language());
    if(language.isEmpty())
        return localeName;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    const QString territory = QLocale::territoryToString(locale.territory());
#else
    const QString territory = QLocale::countryToString(locale.country());
#endif
    if(territory.isEmpty())
        return language;

    return QString::fromLatin1("%1 (%2)").arg(language, territory);
}

QString getLocaleNativeDescription(const QString &localeName)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    const QLocale locale(localeName);
    if(locale.language() == QLocale::C)
        return getLocaleFallbackDescription(localeName);

    const QString language = locale.nativeLanguageName();
    if(language.isEmpty())
        return getLocaleFallbackDescription(localeName);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    const QString territory = locale.nativeTerritoryName();
#else
    const QString territory = locale.nativeCountryName();
#endif
    if(territory.isEmpty())
        return language;

    return QString::fromLatin1("%1 (%2)").arg(language, territory);
#else
    return getLocaleFallbackDescription(localeName);
#endif
}

QString getLocaleFullDescription(const QString &localeName)
{
    const QString fallbackDescription = getLocaleFallbackDescription(localeName);
    const QString nativeDescription = getLocaleNativeDescription(localeName);
    if(fallbackDescription.isEmpty() && nativeDescription.isEmpty())
        return localeName;
    if(fallbackDescription.isEmpty())
        return nativeDescription;
    if(nativeDescription.isEmpty())
        return fallbackDescription;
    if(fallbackDescription == nativeDescription)
        return nativeDescription;
    return QString::fromLatin1("%1 - %2").arg(nativeDescription, fallbackDescription);
}

} // namespace

struct LocalizationManager::Impl
{
    const QStringList supportedLocales;
    const QString systemLocale;
    SettingsWrapper settings;

    ActionListMap actionsMap;
    QList<QComboBox*> comboBoxList;

    QTranslator qtTranslator;
    TranslatorList customTranslators;
    QStringList resourceTemplates;

    Impl()
        : supportedLocales(QStringList() << Locale::EN << Locale::RU << Locale::ZH_CN << Locale::ZH_TW)
        , systemLocale(findBestLocaleForSystem(supportedLocales))
    {
        assert(supportedLocales.contains(systemLocale));
    }

    QString currentLocale() const
    {
        const QVariant rawValue = settings.value(SETTINGS_KEY, systemLocale);
        const QString value = rawValue.isValid() ? rawValue.toString() : systemLocale;
        return supportedLocales.contains(value) ? value : systemLocale;
    }

    void setCurrentLocale(const QString &locale)
    {
        settings.setValue(SETTINGS_KEY, locale);
    }

    void updateActions(const QString &locale)
    {
        ActionList &actions = actionsMap[locale];
        for(ActionList::Iterator it = actions.begin(), itEnd = actions.end(); it != itEnd; ++it)
            (*it)->setChecked(true);

        for(QStringList::ConstIterator it = supportedLocales.constBegin(), itEnd = supportedLocales.constEnd(); it != itEnd; ++it)
        {
            const QString description = getLocaleFullDescription(*it);
            ActionList &actions = actionsMap[*it];
            for(ActionList::Iterator jt = actions.begin(), jtEnd = actions.end(); jt != jtEnd; ++jt)
                (*jt)->setText(description);
        }
    }

    void updateComboBoxes(const QString &locale)
    {
        QMap<QString, QPair<QString, QString> > itemTexts;
        for(QStringList::ConstIterator it = supportedLocales.constBegin(), itEnd = supportedLocales.constEnd(); it != itEnd; ++it)
            itemTexts[*it] = qMakePair<QString, QString>(getLocaleNativeDescription(*it), getLocaleFullDescription(*it));

        for(QList<QComboBox*>::Iterator it = comboBoxList.begin(), itEnd = comboBoxList.end(); it != itEnd; ++it)
        {
            QComboBox *comboBox = *it;
            QSignalBlocker blocker(comboBox);
            comboBox->clear();
            comboBox->setEditable(false);
            for(QMap<QString, QPair<QString, QString> >::ConstIterator jt = itemTexts.constBegin(), jtEnd = itemTexts.constEnd(); jt != jtEnd; ++jt)
            {
                comboBox->addItem(jt.value().first, jt.key());
                comboBox->setItemData(comboBox->count() - 1, jt.value().second, Qt::ToolTipRole);
            }
            comboBox->setCurrentIndex(comboBox->findData(locale));
        }
    }
};

/// @brief Get pointer to current localization manager.
/// @return Pointer to current localization manager.
LocalizationManager *LocalizationManager::instance()
{
    static LocalizationManager manager;
    return &manager;
}

/// @brief Initialize translators for specified ".qm" translation files.
/// @param templatesList - paths for ".qm" translation files, e.g.
/// ":/translations/qtutils_%1" where %1 is place-marker for locale.
void LocalizationManager::initializeResources(const QStringList &templatesList)
{
    m_impl->resourceTemplates = templatesList;
    setLocale();
}

/// @brief Get current locale identifier.
/// @return Current locale identifier.
QString LocalizationManager::locale() const
{
    return m_impl->currentLocale();
}

/// @brief Set current locale with specified identifier.
/// @param locale - Locale identifier.
void LocalizationManager::setLocale(const QString &locale)
{
    if(!locale.isEmpty())
        m_impl->setCurrentLocale(locale);
    const QString newLocale = m_impl->currentLocale();

    qApp->removeTranslator(&m_impl->qtTranslator);
    (void)m_impl->qtTranslator.load(QString::fromLatin1("qt_%1").arg(newLocale),
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                                    QLibraryInfo::path(QLibraryInfo::TranslationsPath));
#else
                                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#endif
    qApp->installTranslator(&m_impl->qtTranslator);

    for(TranslatorList::Iterator it = m_impl->customTranslators.begin(), itEnd = m_impl->customTranslators.end(); it != itEnd; ++it)
    {
        qApp->removeTranslator(*it);
        delete *it;
    }
    m_impl->customTranslators.clear();
    for(QStringList::ConstIterator it = m_impl->resourceTemplates.constBegin(), itEnd = m_impl->resourceTemplates.constEnd(); it != itEnd; ++it)
    {
        QTranslator *translator = new QTranslator(qApp);
        (void)translator->load((*it).arg(newLocale));
        m_impl->customTranslators.append(translator);
        qApp->installTranslator(translator);
    }

    Workarounds::FontsFix(newLocale);
    m_impl->updateActions(newLocale);
    m_impl->updateComboBoxes(newLocale);
    Q_EMIT localeChanged(newLocale);
}

/// @brief Fill menu with items for locale choose. All required
/// connections will be setup automatically.
/// @param menu - Menu which should be filled.
void LocalizationManager::fillMenu(QMenu *menu)
{
    if(!menu)
        return;

    QActionGroup *actionGroup = new QActionGroup(menu);
    actionGroup->setExclusive(true);
    connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(onActionTriggered(QAction*)));
    for(QStringList::ConstIterator it = m_impl->supportedLocales.constBegin(), itEnd = m_impl->supportedLocales.constEnd(); it != itEnd; ++it)
    {
        QAction *action = new QAction(menu);
        menu->addAction(action);
        action->setMenuRole(QAction::NoRole);
        action->setCheckable(true);
        actionGroup->addAction(action);
        connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(onActionDestroyed(QObject*)));
        m_impl->actionsMap[*it].append(action);
    }
    m_impl->updateActions(m_impl->currentLocale());
}

/// @brief Fill combo box with items for locale choose. All required
/// connections will be setup automatically.
/// @param comboBox - Combo box which should be filled.
/// @param autoApply - Apply changes automatically after combo box changes.
void LocalizationManager::fillComboBox(QComboBox *comboBox, const bool autoApply)
{
    if(!comboBox)
        return;

    m_impl->comboBoxList.append(comboBox);
    if(autoApply)
        connect(comboBox, SIGNAL(activated(int)), this, SLOT(onComboBoxActivated(int)));
    connect(comboBox, SIGNAL(destroyed(QObject*)), this, SLOT(onComboBoxDestroyed(QObject*)));
    m_impl->updateComboBoxes(m_impl->currentLocale());
}

LocalizationManager::LocalizationManager()
    : m_impl(new Impl)
{
    assert(qApp);
    setLocale();
}

LocalizationManager::~LocalizationManager()
{}

void LocalizationManager::onActionTriggered(QAction *action)
{
    for(QStringList::ConstIterator it = m_impl->supportedLocales.constBegin(), itEnd = m_impl->supportedLocales.constEnd(); it != itEnd; ++it)
    {
        if(!m_impl->actionsMap[*it].contains(action))
            continue;
        setLocale(*it);
        return;
    }
}

void LocalizationManager::onComboBoxActivated(int index)
{
    Q_UNUSED(index);
    QComboBox *comboBox = static_cast<QComboBox*>(sender());
    assert(comboBox);
    setLocale(comboBox->itemData(comboBox->currentIndex()).toString());
}

void LocalizationManager::onActionDestroyed(QObject *object)
{
    QAction *action = static_cast<QAction*>(object);
    assert(action);
    for(ActionListMap::Iterator it = m_impl->actionsMap.begin(), itEnd = m_impl->actionsMap.end(); it != itEnd; ++it)
        it.value().removeAll(action);
}

void LocalizationManager::onComboBoxDestroyed(QObject *object)
{
    QComboBox *comboBox = static_cast<QComboBox*>(object);
    assert(comboBox);
    m_impl->comboBoxList.removeAll(comboBox);
}

