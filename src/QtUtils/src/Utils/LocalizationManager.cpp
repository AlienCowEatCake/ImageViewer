/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QList>
#include <QStringList>

#include "SettingsWrapper.h"
#include "Workarounds.h"

namespace Locale {

const QString EN = QString::fromLatin1("en");
const QString RU = QString::fromLatin1("ru");

} // namespace Locale

namespace {

const QString SETTINGS_KEY = QString::fromLatin1("Language");

typedef QList<QAction*> ActionList;
typedef QMap<QString, ActionList> ActionListMap;
typedef QList<QTranslator*> TranslatorList;

QString findBestLocaleForSystem(const QStringList& localesList)
{
    const QString systemLocale = QLocale::system().name().toLower();
    for(QStringList::ConstIterator it = localesList.constBegin(), itEnd = localesList.constEnd(); it != itEnd; ++it)
        if(systemLocale.startsWith(*it))
            return *it;
    return Locale::EN;
}

} // namespace

struct LocalizationManager::Impl
{
    const QStringList supportedLocales;
    const QString systemLocale;
    SettingsWrapper settings;

    ActionListMap actionsMap;

    QTranslator qtTranslator;
    TranslatorList customTranslators;
    QStringList resourceTemplates;

    Impl()
        : supportedLocales(QStringList() << Locale::EN << Locale::RU)
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

        ActionList &actionsEnglish = actionsMap[Locale::EN];
        for(ActionList::Iterator it = actionsEnglish.begin(), itEnd = actionsEnglish.end(); it != itEnd; ++it)
            (*it)->setText(QApplication::translate("LocalizationManager", "&English"));

        ActionList &actionsRussian = actionsMap[Locale::RU];
        for(ActionList::Iterator it = actionsRussian.begin(), itEnd = actionsRussian.end(); it != itEnd; ++it)
            (*it)->setText(QApplication::translate("LocalizationManager", "&Russian"));
    }
};

LocalizationManager::~LocalizationManager()
{}

/// @brief Получить указатель на экземпляр текущего менеджера локализаций.
/// @return Указатель на экземпляр текущего менеджера локализаций.
LocalizationManager *LocalizationManager::instance()
{
    static LocalizationManager manager;
    return &manager;
}

/// @brief Инициализировать трансляторы для указанных ресурсов.
/// @param templatesList - пути к локализационным ресурсным файлам,
///  например ":/translations/qtutils_%1" (%1 - шаблон для локали).
void LocalizationManager::initializeResources(const QStringList &templatesList)
{
    m_impl->resourceTemplates = templatesList;
    setLocale();
}

/// @brief Получить текущую локаль.
/// @return Текущая локаль.
QString LocalizationManager::locale() const
{
    return m_impl->currentLocale();
}

/// @brief Установить новую локаль.
/// @param locale - Новая локаль.
void LocalizationManager::setLocale(const QString &locale)
{
    QString newLocale = locale;
    if(newLocale.isEmpty())
        newLocale = m_impl->currentLocale();
    else
        m_impl->setCurrentLocale(newLocale);

    qApp->removeTranslator(&m_impl->qtTranslator);
    m_impl->qtTranslator.load(QString::fromLatin1("qt_%1").arg(newLocale), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
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
        translator->load((*it).arg(newLocale));
        m_impl->customTranslators.append(translator);
        qApp->installTranslator(translator);
    }

    Workarounds::FontsFix(newLocale);
    m_impl->updateActions(newLocale);
    emit localeChanged(newLocale);
}

/// @brief Заполнить меню элементами для выбора локали. Все необходимые
///  соединения будут установлены автоматически.
/// @param menu - меню, которое должно быть заполнено.
void LocalizationManager::fillMenu(QMenu *menu)
{
    if(!menu)
        return;

    QAction *actionEnglish = new QAction(menu);
    QAction *actionRussian = new QAction(menu);

    connect(actionEnglish, SIGNAL(triggered()), this, SLOT(onActionEnglishTriggered()));
    connect(actionRussian, SIGNAL(triggered()), this, SLOT(onActionRussianTriggered()));

    QActionGroup *actionGroup = new QActionGroup(menu);
    actionGroup->setExclusive(true);

    QList<QAction*> actions(QList<QAction*>() << actionEnglish << actionRussian);
    for(QList<QAction*>::Iterator it = actions.begin(), itEnd = actions.end(); it != itEnd; ++it)
    {
        QAction *action = *it;
        menu->addAction(action);
        action->setMenuRole(QAction::NoRole);
        action->setCheckable(true);
        actionGroup->addAction(action);
        connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(onActionDestroyed(QObject*)));
    }

    m_impl->actionsMap[Locale::EN].append(actionEnglish);
    m_impl->actionsMap[Locale::RU].append(actionRussian);

    m_impl->updateActions(m_impl->currentLocale());
}

LocalizationManager::LocalizationManager()
    : m_impl(new Impl)
{
    assert(qApp);
    setLocale();
}

void LocalizationManager::onActionEnglishTriggered()
{
    setLocale(Locale::EN);
}

void LocalizationManager::onActionRussianTriggered()
{
    setLocale(Locale::RU);
}

void LocalizationManager::onActionDestroyed(QObject *object)
{
    QAction *action = static_cast<QAction*>(object);
    assert(action);
    for(ActionListMap::Iterator it = m_impl->actionsMap.begin(), itEnd = m_impl->actionsMap.end(); it != itEnd; ++it)
        it.value().removeAll(action);
}

