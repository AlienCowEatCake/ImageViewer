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
#include <QSet>
#include <QDir>
#include <QFileInfo>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#else
#include <QRegExp>
#endif

#include "IsOneOf.h"
#include "SettingsWrapper.h"
#include "SignalBlocker.h"
#include "Workarounds.h"

namespace Locale {

static const QString EN = QString::fromLatin1("en");
static const QString PT_PT = QString::fromLatin1("pt_PT");
static const QString ZH_CN = QString::fromLatin1("zh_CN");
static const QString ZH_TW = QString::fromLatin1("zh_TW");

} // namespace Locale

namespace {

const QString SETTINGS_KEY = QString::fromLatin1("Language");

typedef QList<QAction*> ActionList;
typedef QMap<QString, ActionList> ActionListMap;
typedef QList<QTranslator*> TranslatorList;

QStringList getLocalesFromTemplatesList(const QStringList &templatesList)
{
    QStringList result;
    for(QStringList::ConstIterator templatesIt = templatesList.constBegin(), templatesItEnd = templatesList.constEnd(); templatesIt != templatesItEnd; ++templatesIt)
    {
        const QFileInfo templateInfo(*templatesIt);
        const QString fileName = templateInfo.fileName() + (templateInfo.suffix() == QString::fromLatin1("qm") ? QString() : QString::fromLatin1(".qm"));
        QDir dir = templateInfo.dir();
        dir.setNameFilters(QStringList() << fileName.arg(QString::fromLatin1("*")));
        const QStringList entries = dir.entryList(QDir::Files | QDir::Readable);
        for(QStringList::ConstIterator entriesIt = entries.constBegin(), entriesItEnd = entries.constEnd(); entriesIt != entriesItEnd; ++entriesIt)
        {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            const QString regExpString = QRegularExpression::escape(fileName)
                    .replace(QString::fromLatin1("\\%1"), QString::fromLatin1("%1"))
                    .arg(QString::fromLatin1("(.*)"));
            const QRegularExpression regExp(regExpString);
            const QRegularExpressionMatch match = regExp.match(*entriesIt);
            if(match.hasMatch())
            {
                const QString captured = match.captured(1);
#else
            const QString regExpString = QRegExp::escape(fileName)
                    .replace(QString::fromLatin1("\\%1"), QString::fromLatin1("%1"))
                    .arg(QString::fromLatin1("(.*)"));
            QRegExp regExp(regExpString);
            if(regExp.indexIn(*entriesIt) != -1)
            {
                const QString captured = regExp.cap(1);
#endif
                if(!captured.isEmpty())
                    result.append(captured);
            }
        }
    }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    result.sort();
    result.removeDuplicates();
#else
    result = QStringList::fromSet(result.toSet());
#endif
    return result;
}

int getMatchedLocaleComponents(const QStringList &systemLocaleSplitted, const QString &locale)
{
    const QStringList localeSplitted = locale.split(QChar::fromLatin1('_'));
    int matchedComponents = 0;
    for(QStringList::ConstIterator cIt = localeSplitted.constBegin(), cItEnd = localeSplitted.constEnd(),
        sIt = systemLocaleSplitted.constBegin(), sItEnd = systemLocaleSplitted.constEnd();
        cIt != cItEnd && sIt != sItEnd; ++cIt, ++sIt, ++matchedComponents)
    {
        if(cIt->compare(*sIt, Qt::CaseInsensitive) != 0)
            return 0;
    }
    if(matchedComponents < static_cast<int>(localeSplitted.size()))
        matchedComponents = 0;
    return matchedComponents;
}

QString findBestLocaleForPreferred(const QStringList &localesList, const QList<QPair<QString, QString> > &customMappings, const QString &preferredLocale)
{
    const QStringList preferredLocaleSplitted = preferredLocale.split(QChar::fromLatin1('_'));
    int bestMatchedComponents = 0;
    QString bestLocale;
    for(QStringList::ConstIterator it = localesList.constBegin(), itEnd = localesList.constEnd(); it != itEnd; ++it)
    {
        int matchedComponents = getMatchedLocaleComponents(preferredLocaleSplitted, *it);
        if(matchedComponents > bestMatchedComponents)
        {
            bestLocale = *it;
            bestMatchedComponents = matchedComponents;
        }
    }
    if(!bestLocale.isEmpty())
        return bestLocale;


    for(QList<QPair<QString, QString> >::ConstIterator it = customMappings.constBegin(), itEnd = customMappings.constEnd(); it != itEnd; ++it)
    {
        if(getMatchedLocaleComponents(preferredLocaleSplitted, it->first))
            return it->second;
    }

    return QString();
}

QString findBestLocaleForSystem(const QStringList &localesList)
{
    QList<QPair<QString, QString> > customMappings;
    if(localesList.contains(Locale::PT_PT))
        customMappings.append(qMakePair<QString, QString>(QString::fromLatin1("pt"), QString(Locale::PT_PT)));
    if(localesList.contains(Locale::ZH_CN))
        customMappings.append(qMakePair<QString, QString>(QString::fromLatin1("zh_Hans"), QString(Locale::ZH_CN)));
    if(localesList.contains(Locale::ZH_TW))
        customMappings.append(qMakePair<QString, QString>(QString::fromLatin1("zh_Hant"), QString(Locale::ZH_TW)));

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
#if (QT_VERSION >= QT_VERSION_CHECK(6, 7, 0))
    const QStringList uiLanguages = QLocale::system().uiLanguages(QLocale::TagSeparator::Underscore);
#else
    const QStringList uiLanguages = QLocale::system().uiLanguages();
#endif
    for(QStringList::ConstIterator uiLangIt = uiLanguages.constBegin(), uiLangItEnd = uiLanguages.constEnd(); uiLangIt != uiLangItEnd; ++uiLangIt)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 7, 0))
        const QString uiLanguage = *uiLangIt;
#else
        const QString uiLanguage = QString(*uiLangIt).replace(QChar::fromLatin1('-'), QChar::fromLatin1('_'));
#endif
        const QString bestLocale = findBestLocaleForPreferred(localesList, customMappings, uiLanguage);
        if(!bestLocale.isEmpty())
            return bestLocale;
    }
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 7, 0))
    const QString systemLocale = QLocale::system().name(QLocale::TagSeparator::Underscore);
#else
    const QString systemLocale = QLocale::system().name().replace(QChar::fromLatin1('-'), QChar::fromLatin1('_'));
#endif
    const QString bestLocale = findBestLocaleForPreferred(localesList, customMappings, systemLocale);
    if(!bestLocale.isEmpty())
        return bestLocale;

    if(localesList.contains(Locale::EN))
        return Locale::EN;
    if(localesList.size() == 1)
        return localesList.first();
    return QString();
}

inline bool isGenericLanguage(const QLocale &locale)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    return IsOneOf(locale.language(), QLocale::C, QLocale::AnyLanguage);
#else
    return IsOneOf(static_cast<int>(locale.language()), static_cast<int>(QLocale::C), static_cast<int>(0));
#endif
}

QString getLocaleFallbackDescription(const QString &localeName)
{
    if(localeName.isEmpty())
        return QString();

    const QLocale locale(localeName);
    if(isGenericLanguage(locale))
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
    if(localeName.isEmpty())
        return QString();

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    const QLocale locale(localeName);
    if(isGenericLanguage(locale.language()))
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
        return QString::fromLatin1("%1 - %2").arg(localeName, nativeDescription);
    if(nativeDescription.isEmpty())
        return QString::fromLatin1("%1 - %2").arg(localeName, fallbackDescription);
    if(fallbackDescription == nativeDescription)
        return QString::fromLatin1("%1 - %2").arg(localeName, nativeDescription);
    return QString::fromLatin1("%1 - %2 - %3").arg(localeName, nativeDescription, fallbackDescription);
}

} // namespace

struct LocalizationManager::Impl
{
    QStringList supportedLocales;
    QString systemLocale;
    SettingsWrapper settings;

    ActionListMap actionsMap;
    QList<QComboBox*> comboBoxList;

    QTranslator qtTranslator;
    TranslatorList customTranslators;
    QStringList resourceTemplates;

    QString currentLocale() const
    {
        const QVariant rawValue = settings.value(SETTINGS_KEY, systemLocale);
        const QString value = rawValue.isValid() ? rawValue.toString() : systemLocale;
        return (!value.isEmpty() && supportedLocales.contains(value)) ? value : systemLocale;
    }

    void setCurrentLocale(const QString &locale)
    {
        settings.setValue(SETTINGS_KEY, locale);
    }

    void updateActions(const QString &locale)
    {
        if(locale.isEmpty())
            return;

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
    assert(m_impl->comboBoxList.isEmpty());
    assert(m_impl->actionsMap.isEmpty());
    m_impl->supportedLocales = getLocalesFromTemplatesList(templatesList);
    m_impl->systemLocale = findBestLocaleForSystem(m_impl->supportedLocales);
    m_impl->resourceTemplates = templatesList;
#if !defined (QTUTILS_DISABLE_EMBED_TRANSLATIONS)
    const QString translationsPath = QString::fromLatin1(":/translations");
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QString translationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    const QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
    const QString qtUtilsTemplate = QDir(translationsPath).filePath(QString::fromLatin1("qtutils_%1"));
    if(!m_impl->resourceTemplates.contains(qtUtilsTemplate))
        m_impl->resourceTemplates.append(qtUtilsTemplate);
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
    if(!newLocale.isEmpty())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        const QString translationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
        const QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
        if(m_impl->qtTranslator.load(QString::fromLatin1("qt_%1").arg(newLocale), translationsPath))
            qApp->installTranslator(&m_impl->qtTranslator);
    }

    for(TranslatorList::Iterator it = m_impl->customTranslators.begin(), itEnd = m_impl->customTranslators.end(); it != itEnd; ++it)
    {
        qApp->removeTranslator(*it);
        delete *it;
    }
    m_impl->customTranslators.clear();
    if(!newLocale.isEmpty())
    {
        for(QStringList::ConstIterator it = m_impl->resourceTemplates.constBegin(), itEnd = m_impl->resourceTemplates.constEnd(); it != itEnd; ++it)
        {
            QTranslator *translator = new QTranslator(qApp);
            if(translator->load((*it).arg(newLocale)))
            {
                m_impl->customTranslators.append(translator);
                qApp->installTranslator(translator);
            }
            else
            {
                delete translator;
            }
        }
    }

#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    if(!newLocale.isEmpty())
        qApp->setLayoutDirection(QLocale(newLocale).textDirection());
    else
        qApp->setLayoutDirection(Qt::LayoutDirectionAuto);
#else
    switch(QLocale(newLocale).language())
    {
    case QLocale::Arabic:
    case QLocale::Divehi:
    case QLocale::Hebrew:
    case QLocale::Kashmiri:
    case QLocale::Pashto:
    case QLocale::Persian:
    case QLocale::Sindhi:
    case QLocale::Syriac:
    case QLocale::Urdu:
    case QLocale::Uigur:
    case QLocale::Yiddish:
        qApp->setLayoutDirection(Qt::RightToLeft);
        break;
    default:
        qApp->setLayoutDirection(Qt::LeftToRight);
        break;
    }
#endif

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

