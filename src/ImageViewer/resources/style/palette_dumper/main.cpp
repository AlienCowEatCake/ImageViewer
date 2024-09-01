#include <QDebug>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QString>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QGuiApplication>
#else
#include <QApplication>
#define QGuiApplication QApplication
#endif

QColor getColor(const QPalette &palette, QPalette::ColorRole role, QPalette::ColorGroup group, bool isBrush)
{
    if(group == QPalette::All)
        group = QPalette::Normal;
    return isBrush ? palette.brush(group, role).color() : palette.color(group, role);
}

QString getHex(int value)
{
    const QString result = QString::number(value, 16);
    return result.size() == 2 ? result : QString::fromLatin1("0") + result;
}

QString getColorString(const QColor &color)
{
    if(color.alpha() < 255)
        return QString::fromLatin1("rgba(%1, %2, %3, %4%)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha() * 100 / 255);
    return QString::fromLatin1("#%1%2%3").arg(getHex(color.red()), getHex(color.green()), getHex(color.blue()));
}

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
#define PROCESS(NAME, ROLE, GROUP, IS_BRUSH) \
    qWarning().nospace() << "qproperty-" << (#NAME) << ": " << getColorString(getColor(a.palette(), (ROLE), (GROUP), (IS_BRUSH))).toLatin1().data() << ";";
    PROCESS(paletteWindow,                    QPalette::Window,           QPalette::All,      false)
    PROCESS(paletteWindowDisabled,            QPalette::Window,           QPalette::Disabled, false)
    PROCESS(paletteWindowText,                QPalette::WindowText,       QPalette::All,      true)
    PROCESS(paletteWindowTextDisabled,        QPalette::WindowText,       QPalette::Disabled, true)
    PROCESS(paletteBase,                      QPalette::Base,             QPalette::All,      false)
    PROCESS(paletteBaseDisabled,              QPalette::Base,             QPalette::Disabled, false)
    PROCESS(paletteAlternateBase,             QPalette::AlternateBase,    QPalette::All,      false)
    PROCESS(paletteAlternateBaseDisabled,     QPalette::AlternateBase,    QPalette::Disabled, false)
    PROCESS(paletteToolTipBase,               QPalette::ToolTipBase,      QPalette::All,      true)
    PROCESS(paletteToolTipBaseDisabled,       QPalette::ToolTipBase,      QPalette::Disabled, true)
    PROCESS(paletteToolTipText,               QPalette::ToolTipText,      QPalette::All,      false)
    PROCESS(paletteToolTipTextDisabled,       QPalette::ToolTipText,      QPalette::Disabled, false)
    PROCESS(paletteText,                      QPalette::Text,             QPalette::All,      true)
    PROCESS(paletteTextDisabled,              QPalette::Text,             QPalette::Disabled, true)
    PROCESS(paletteButton,                    QPalette::Button,           QPalette::All,      false)
    PROCESS(paletteButtonDisabled,            QPalette::Button,           QPalette::Disabled, false)
    PROCESS(paletteButtonText,                QPalette::ButtonText,       QPalette::All,      true)
    PROCESS(paletteButtonTextDisabled,        QPalette::ButtonText,       QPalette::Disabled, true)
    PROCESS(paletteBrightText,                QPalette::BrightText,       QPalette::All,      false)
    PROCESS(paletteBrightTextDisabled,        QPalette::BrightText,       QPalette::Disabled, false)
    PROCESS(paletteHighlight,                 QPalette::Highlight,        QPalette::All,      true)
    PROCESS(paletteHighlightDisabled,         QPalette::Highlight,        QPalette::Disabled, true)
    PROCESS(paletteHighlightedText,           QPalette::HighlightedText,  QPalette::All,      true)
    PROCESS(paletteHighlightedTextDisabled,   QPalette::HighlightedText,  QPalette::Disabled, true)
    PROCESS(paletteLink,                      QPalette::Link,             QPalette::All,      false)
    PROCESS(paletteLinkDisabled,              QPalette::Link,             QPalette::Disabled, false)
    PROCESS(paletteLinkVisited,               QPalette::LinkVisited,      QPalette::All,      false)
    PROCESS(paletteLinkVisitedDisabled,       QPalette::LinkVisited,      QPalette::Disabled, false)
    PROCESS(paletteLight,                     QPalette::Light,            QPalette::All,      false)
    PROCESS(paletteLightDisabled,             QPalette::Light,            QPalette::Disabled, false)
    PROCESS(paletteMidlight,                  QPalette::Midlight,         QPalette::All,      false)
    PROCESS(paletteMidlightDisabled,          QPalette::Midlight,         QPalette::Disabled, false)
    PROCESS(paletteDark,                      QPalette::Dark,             QPalette::All,      false)
    PROCESS(paletteDarkDisabled,              QPalette::Dark,             QPalette::Disabled, false)
    PROCESS(paletteMid,                       QPalette::Mid,              QPalette::All,      false)
    PROCESS(paletteMidDisabled,               QPalette::Mid,              QPalette::Disabled, false)
    PROCESS(paletteShadow,                    QPalette::Shadow,           QPalette::All,      false)
    PROCESS(paletteShadowDisabled,            QPalette::Shadow,           QPalette::Disabled, false)
    return 0;
}
