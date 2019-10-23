/*
   Copyright (C) 2018-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "ThemeManager.h"
#include "ThemeManager_p.h"

#include <cassert>

#include <QPalette>
#include <QApplication>
#include <QMessageBox>
#include <QStyleFactory>
#include <QStyle>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QAbstractScrollArea>
#include <QMap>
#include <QColor>
#include <QTextDocument>

#include "SettingsWrapper.h"
#include "SignalBlocker.h"
#include "ThemeUtils.h"

namespace {

const QString SETTINGS_KEY = QString::fromLatin1("Theme");

struct PaletteMappingData
{
    QColor color;
    QPalette::ColorRole paletteColorRole;
    QPalette::ColorGroup paletteColorGroup;
    bool setColorRoleAsBrush;

    PaletteMappingData(QColor color,
                       QPalette::ColorRole paletteColorRole,
                       QPalette::ColorGroup paletteColorGroup,
                       bool setColorRoleAsBrush)
        : color(color)
        , paletteColorRole(paletteColorRole)
        , paletteColorGroup(paletteColorGroup)
        , setColorRoleAsBrush(setColorRoleAsBrush)
    {}
};

struct ThemeData
{
    QString themeId;
    QStringList styleSheets;
    QString translateContext;
    bool isDefault;

    ThemeData()
        : isDefault(false)
    {}

    ThemeData(const QString &themeId, const QStringList &styleSheets,
              const QString &translateContext, const bool isDefault)
        : themeId(themeId)
        , styleSheets(styleSheets)
        , translateContext(translateContext)
        , isDefault(isDefault)
    {}
};

#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))

#define USE_TEXTDOCUMENT_STYLER

/// @note https://bugreports.qt.io/browse/QTBUG-34114
class TextDocumentStyler : public QObject
{
    Q_DISABLE_COPY(TextDocumentStyler)

public:
    TextDocumentStyler(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {}

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE
    {
        if(event->type() == QEvent::ChildAdded)
        {
            if(QTextDocument *document = qobject_cast<QTextDocument*>(object))
            {
                const QString styleSheetTemplate = QString::fromLatin1("a { color: %1; }");
                document->setDefaultStyleSheet(styleSheetTemplate.arg(qApp->palette().color(QPalette::Link).name()));
            }
        }
        return QObject::eventFilter(object, event);
    }
};

#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 5, 0))

#define USE_SCROLLBAR_STYLER

class ScrollBarStyler : public QObject
{
    Q_DISABLE_COPY(ScrollBarStyler)

public:
    ScrollBarStyler(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {}

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE
    {
        if(event->type() == QEvent::ChildAdded)
        {
            if(QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(object))
                scrollArea->setProperty("ScrollBorderRequired", true);
        }
        return QObject::eventFilter(object, event);
    }
};

#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))

#define USE_MENU_STYLER

class MenuStyler : public QObject
{
    Q_DISABLE_COPY(MenuStyler)

public:
    MenuStyler(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {}

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE
    {
        if(event->type() == QEvent::ChildAdded)
        {
            if(QMenu *menu = qobject_cast<QMenu*>(object))
                menu->setProperty("LeftPaddingRequired", true);
        }
        return QObject::eventFilter(object, event);
    }
};

#endif

typedef QList<QAction*> ActionList;
typedef QMap<QString, ActionList> ActionListMap;

} // namespace

struct ThemeManager::Impl
{
    QPalette defaultPalette;
    QList<ThemeData> themes;
    SettingsWrapper settings;

    ActionListMap actionsMap;
    QList<QComboBox*> comboBoxList;

    Impl()
        : defaultPalette(qApp->palette())
    {
#if defined (USE_TEXTDOCUMENT_STYLER)
        qApp->installEventFilter(new TextDocumentStyler(qApp));
#endif
#if defined (USE_SCROLLBAR_STYLER)
        qApp->installEventFilter(new ScrollBarStyler(qApp));
#endif
#if defined (USE_MENU_STYLER)
        qApp->installEventFilter(new MenuStyler(qApp));
#endif
    }

    ThemeData *currentTheme()
    {
        QString themeString = settings.value(SETTINGS_KEY).toString();
        if(!themeString.isEmpty())
        {
            for(QList<ThemeData>::Iterator it = themes.begin(), itEnd = themes.end(); it != itEnd; ++it)
                if(themeString.compare(it->themeId, Qt::CaseInsensitive) == 0)
                    return &(*it);
        }
        themeString.clear();
        for(QList<ThemeData>::Iterator it = themes.begin(), itEnd = themes.end(); it != itEnd; ++it)
            if(it->isDefault)
                return &(*it);
        return Q_NULLPTR;
    }

    void applyThemeStyle()
    {
        StylableTheme theme;
        const QStringList availableStyles = QStyleFactory::keys();
        const QStringList preferredStyles = theme.preferredStyles().split(QChar::fromLatin1(','), QString::SkipEmptyParts);
        QString matchedStyle;
        for(QStringList::ConstIterator it = preferredStyles.constBegin(), itEnd = preferredStyles.constEnd(); it != itEnd; ++it)
        {
            if(availableStyles.contains(*it, Qt::CaseInsensitive))
            {
                matchedStyle = *it;
                break;
            }
        }
        QStyle *style = QStyleFactory::create(matchedStyle);

        const QList<PaletteMappingData> mapping = QList<PaletteMappingData>()
                << PaletteMappingData(theme.paletteWindow(),                    QPalette::Window,           QPalette::All,      false)
                << PaletteMappingData(theme.paletteWindowDisabled(),            QPalette::Window,           QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteWindowText(),                QPalette::WindowText,       QPalette::All,      true)
                << PaletteMappingData(theme.paletteWindowTextDisabled(),        QPalette::WindowText,       QPalette::Disabled, true)
                << PaletteMappingData(theme.paletteBase(),                      QPalette::Base,             QPalette::All,      false)
                << PaletteMappingData(theme.paletteBaseDisabled(),              QPalette::Base,             QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteAlternateBase(),             QPalette::AlternateBase,    QPalette::All,      false)
                << PaletteMappingData(theme.paletteAlternateBaseDisabled(),     QPalette::AlternateBase,    QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteToolTipBase(),               QPalette::ToolTipBase,      QPalette::All,      true)
                << PaletteMappingData(theme.paletteToolTipBaseDisabled(),       QPalette::ToolTipBase,      QPalette::Disabled, true)
                << PaletteMappingData(theme.paletteToolTipText(),               QPalette::ToolTipText,      QPalette::All,      false)
                << PaletteMappingData(theme.paletteToolTipTextDisabled(),       QPalette::ToolTipText,      QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteText(),                      QPalette::Text,             QPalette::All,      true)
                << PaletteMappingData(theme.paletteTextDisabled(),              QPalette::Text,             QPalette::Disabled, true)
                << PaletteMappingData(theme.paletteButton(),                    QPalette::Button,           QPalette::All,      false)
                << PaletteMappingData(theme.paletteButtonDisabled(),            QPalette::Button,           QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteButtonText(),                QPalette::ButtonText,       QPalette::All,      true)
                << PaletteMappingData(theme.paletteButtonTextDisabled(),        QPalette::ButtonText,       QPalette::Disabled, true)
                << PaletteMappingData(theme.paletteBrightText(),                QPalette::BrightText,       QPalette::All,      false)
                << PaletteMappingData(theme.paletteBrightTextDisabled(),        QPalette::BrightText,       QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteHighlight(),                 QPalette::Highlight,        QPalette::All,      true)
                << PaletteMappingData(theme.paletteHighlightDisabled(),         QPalette::Highlight,        QPalette::Disabled, true)
                << PaletteMappingData(theme.paletteHighlightedText(),           QPalette::HighlightedText,  QPalette::All,      true)
                << PaletteMappingData(theme.paletteHighlightedTextDisabled(),   QPalette::HighlightedText,  QPalette::Disabled, true)
                << PaletteMappingData(theme.paletteLink(),                      QPalette::Link,             QPalette::All,      false)
                << PaletteMappingData(theme.paletteLinkDisabled(),              QPalette::Link,             QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteLinkVisited(),               QPalette::LinkVisited,      QPalette::All,      false)
                << PaletteMappingData(theme.paletteLinkVisitedDisabled(),       QPalette::LinkVisited,      QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteLight(),                     QPalette::Light,            QPalette::All,      false)
                << PaletteMappingData(theme.paletteLightDisabled(),             QPalette::Light,            QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteMidlight(),                  QPalette::Midlight,         QPalette::All,      false)
                << PaletteMappingData(theme.paletteMidlightDisabled(),          QPalette::Midlight,         QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteDark(),                      QPalette::Dark,             QPalette::All,      false)
                << PaletteMappingData(theme.paletteDarkDisabled(),              QPalette::Dark,             QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteMid(),                       QPalette::Mid,              QPalette::All,      false)
                << PaletteMappingData(theme.paletteMidDisabled(),               QPalette::Mid,              QPalette::Disabled, false)
                << PaletteMappingData(theme.paletteShadow(),                    QPalette::Shadow,           QPalette::All,      false)
                << PaletteMappingData(theme.paletteShadowDisabled(),            QPalette::Shadow,           QPalette::Disabled, false)
                   ;
        QPalette palette = style ? style->standardPalette() : defaultPalette;
        for(QList<PaletteMappingData>::ConstIterator it = mapping.constBegin(), itEnd = mapping.constEnd(); it != itEnd; ++it)
        {
            if(!it->color.isValid())
                continue;
            if(it->setColorRoleAsBrush)
                palette.setBrush(it->paletteColorGroup, it->paletteColorRole, it->color);
            else
                palette.setColor(it->paletteColorGroup, it->paletteColorRole, it->color);
        }

        qApp->setPalette(palette);
        if(!style)
            return;

        qApp->setStyle(style);
        qApp->setPalette(palette);
    }

    void updateActions()
    {
        if(currentTheme())
        {
            ActionList &actions = actionsMap[currentTheme()->themeId];
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
        for(QList<QComboBox*>::Iterator it = comboBoxList.begin(), itEnd = comboBoxList.end(); it != itEnd; ++it)
        {
            QComboBox *comboBox = *it;
            QSignalBlocker blocker(comboBox);
            comboBox->clear();
            comboBox->setEditable(false);
            for(QList<ThemeData>::ConstIterator jt = themes.constBegin(), jtEnd = themes.constEnd(); jt != jtEnd; ++jt)
                comboBox->addItem(itemTexts[jt->themeId], jt->themeId);
            if(currentTheme())
                comboBox->setCurrentIndex(comboBox->findData(currentTheme()->themeId));
        }
    }
};

/// @brief Получить указатель на экземпляр текущего менеджера тем.
/// @return Указатель на экземпляр текущего менеджера тем.
ThemeManager *ThemeManager::instance()
{
    static ThemeManager manager;
    return &manager;
}

/// @brief Зарегистрировать тему.
/// @param themeId - Уникальный идентификатор темы.
/// @param styleSheets - Список путей до QSS файлов со стилями.
/// @param translateContext - Контекст локализации. В этом контексте должна
///  быть строка с ключем themeId, содержащая локализованное название темы.
/// @param isDefault - Тема по умолчанию. Тема по умолчанию должна быть одна.
void ThemeManager::registerTheme(const QString &themeId, const QStringList &styleSheets,
                                 const QString &translateContext, const bool isDefault)
{
    m_impl->themes.append(ThemeData(themeId, styleSheets, translateContext, isDefault));
}

/// @brief Применить текущую тему. Допускается использование только один раз
/// при старте приложения, в противном случае результат не гарантируется.
void ThemeManager::applyCurrentTheme()
{
    ThemeData *data = m_impl->currentTheme();
    if(!data)
        return;

    ThemeUtils::LoadStyleSheet(data->styleSheets);
    qApp->style()->unpolish(qApp);
    qApp->style()->polish(qApp);
    m_impl->applyThemeStyle();
    qApp->style()->unpolish(qApp);
    qApp->style()->polish(qApp);
}

/// @brief Получить идентификатор текущей темы.
/// @return Идентификатор текущей темы.
QString ThemeManager::currentTheme() const
{
    if(m_impl->currentTheme())
        return m_impl->currentTheme()->themeId;
    return QString();
}

/// @brief Установить текущей тему с указанным идентификатором. Сама тема не
/// применится, пока не будет вызван метод applyCurrentTheme().
/// @param themeId - Идентификатор темы.
/// @param showMessage - Показать ли сообщение что тема будет применена после
///  перезапуска программы.
/// @param parent - Родитель для сообщения, показываемого при showMessage=true.
void ThemeManager::setTheme(const QString &themeId, const bool showMessage, QWidget *parent)
{
    QList<ThemeData>::Iterator theme = m_impl->themes.end();
    for(QList<ThemeData>::Iterator it = m_impl->themes.begin(), itEnd = m_impl->themes.end(); it != itEnd && theme == itEnd; ++it)
        if(it->themeId == themeId)
            theme = it;
    if(theme == m_impl->themes.end())
        return;

    if(showMessage)
        QMessageBox::information(parent, qApp->translate("ThemeManager", "Restart Required"),
                                 qApp->translate("ThemeManager", "The theme change will take effect after a restart of application."),
                                 QMessageBox::Ok, QMessageBox::Ok);
    m_impl->settings.setValue(SETTINGS_KEY, theme->themeId);
    m_impl->updateActions();
    m_impl->updateComboBoxes();
    emit themeChanged(theme->themeId);
}

/// @brief Заполнить меню элементами для выбора темы. Все необходимые
///  соединения будут установлены автоматически.
/// @param menu - меню, которое должно быть заполнено.
void ThemeManager::fillMenu(QMenu *menu)
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

/// @brief Заполнить комбобокс элементами для выбора темы. Все необходимые
///  соединения будут установлены автоматически.
/// @param comboBox - комбобокс, который должен быть заполнен.
/// @param autoApply - автоматически применять изменения при выборе темы.
void ThemeManager::fillComboBox(QComboBox *comboBox, const bool autoApply)
{
    if(!comboBox)
        return;

    m_impl->comboBoxList.append(comboBox);
    if(autoApply)
        connect(comboBox, SIGNAL(activated(int)), this, SLOT(onComboBoxActivated(int)));
    connect(comboBox, SIGNAL(destroyed(QObject*)), this, SLOT(onComboBoxDestroyed(QObject*)));
    m_impl->updateComboBoxes();
}

ThemeManager::ThemeManager()
    : m_impl(new Impl)
{}

ThemeManager::~ThemeManager()
{}

void ThemeManager::onActionTriggered(QAction *action)
{
    assert(action);
    setTheme(action->data().toString());
}

void ThemeManager::onComboBoxActivated(int index)
{
    Q_UNUSED(index);
    QComboBox *comboBox = static_cast<QComboBox*>(sender());
    assert(comboBox);
    setTheme(comboBox->itemData(comboBox->currentIndex()).toString());
}

void ThemeManager::onActionDestroyed(QObject *object)
{
    QAction *action = static_cast<QAction*>(object);
    assert(action);
    for(ActionListMap::Iterator it = m_impl->actionsMap.begin(), itEnd = m_impl->actionsMap.end(); it != itEnd; ++it)
        it.value().removeAll(action);
}

void ThemeManager::onComboBoxDestroyed(QObject *object)
{
    QComboBox *comboBox = static_cast<QComboBox*>(object);
    assert(comboBox);
    m_impl->comboBoxList.removeAll(comboBox);
}

