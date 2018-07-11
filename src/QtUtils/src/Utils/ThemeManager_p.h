/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_THEMEMANAGER_P_H_INCLUDED)
#define QTUTILS_THEMEMANAGER_P_H_INCLUDED

#include <QWidget>
#include <QString>
#include <QColor>

struct StylableTheme : public QWidget
{
    Q_OBJECT

public:
    StylableTheme()
    {
        ensurePolished();
    }

    //

    /// preferredStyles
private:
    Q_PROPERTY(QString preferredStyles READ preferredStyles WRITE setPreferredStyles STORED false)
public:
    QString preferredStyles() const { return m_preferredStyles; }
    void setPreferredStyles(const QString &preferredStyles) { m_preferredStyles = preferredStyles; }
private:
    QString m_preferredStyles;

    //

    /// paletteWindow
private:
    Q_PROPERTY(QColor paletteWindow READ paletteWindow WRITE setPaletteWindow STORED false)
public:
    QColor paletteWindow() const { return m_paletteWindow; }
    void setPaletteWindow(const QColor &paletteWindow) { m_paletteWindow = paletteWindow; }
private:
    QColor m_paletteWindow;

    /// paletteWindowText
private:
    Q_PROPERTY(QColor paletteWindowText READ paletteWindowText WRITE setPaletteWindowText STORED false)
public:
    QColor paletteWindowText() const { return m_paletteWindowText; }
    void setPaletteWindowText(const QColor &paletteWindowText) { m_paletteWindowText = paletteWindowText; }
private:
    QColor m_paletteWindowText;

    /// paletteBase
private:
    Q_PROPERTY(QColor paletteBase READ paletteBase WRITE setPaletteBase STORED false)
public:
    QColor paletteBase() const { return m_paletteBase; }
    void setPaletteBase(const QColor &paletteBase) { m_paletteBase = paletteBase; }
private:
    QColor m_paletteBase;

    /// paletteAlternateBase
private:
    Q_PROPERTY(QColor paletteAlternateBase READ paletteAlternateBase WRITE setPaletteAlternateBase STORED false)
public:
    QColor paletteAlternateBase() const { return m_paletteAlternateBase; }
    void setPaletteAlternateBase(const QColor &paletteAlternateBase) { m_paletteAlternateBase = paletteAlternateBase; }
private:
    QColor m_paletteAlternateBase;

    /// paletteToolTipBase
private:
    Q_PROPERTY(QColor paletteToolTipBase READ paletteToolTipBase WRITE setPaletteToolTipBase STORED false)
public:
    QColor paletteToolTipBase() const { return m_paletteToolTipBase; }
    void setPaletteToolTipBase(const QColor &paletteToolTipBase) { m_paletteToolTipBase = paletteToolTipBase; }
private:
    QColor m_paletteToolTipBase;

    /// paletteToolTipText
private:
    Q_PROPERTY(QColor paletteToolTipText READ paletteToolTipText WRITE setPaletteToolTipText STORED false)
public:
    QColor paletteToolTipText() const { return m_paletteToolTipText; }
    void setPaletteToolTipText(const QColor &paletteToolTipText) { m_paletteToolTipText = paletteToolTipText; }
private:
    QColor m_paletteToolTipText;

    /// paletteText
private:
    Q_PROPERTY(QColor paletteText READ paletteText WRITE setPaletteText STORED false)
public:
    QColor paletteText() const { return m_paletteText; }
    void setPaletteText(const QColor &paletteText) { m_paletteText = paletteText; }
private:
    QColor m_paletteText;

    /// paletteButton
private:
    Q_PROPERTY(QColor paletteButton READ paletteButton WRITE setPaletteButton STORED false)
public:
    QColor paletteButton() const { return m_paletteButton; }
    void setPaletteButton(const QColor &paletteButton) { m_paletteButton = paletteButton; }
private:
    QColor m_paletteButton;

    /// paletteButtonText
private:
    Q_PROPERTY(QColor paletteButtonText READ paletteButtonText WRITE setPaletteButtonText STORED false)
public:
    QColor paletteButtonText() const { return m_paletteButtonText; }
    void setPaletteButtonText(const QColor &paletteButtonText) { m_paletteButtonText = paletteButtonText; }
private:
    QColor m_paletteButtonText;

    /// paletteBrightText
private:
    Q_PROPERTY(QColor paletteBrightText READ paletteBrightText WRITE setPaletteBrightText STORED false)
public:
    QColor paletteBrightText() const { return m_paletteBrightText; }
    void setPaletteBrightText(const QColor &paletteBrightText) { m_paletteBrightText = paletteBrightText; }
private:
    QColor m_paletteBrightText;

    /// paletteHighlight
private:
    Q_PROPERTY(QColor paletteHighlight READ paletteHighlight WRITE setPaletteHighlight STORED false)
public:
    QColor paletteHighlight() const { return m_paletteHighlight; }
    void setPaletteHighlight(const QColor &paletteHighlight) { m_paletteHighlight = paletteHighlight; }
private:
    QColor m_paletteHighlight;

    /// paletteHighlightedText
private:
    Q_PROPERTY(QColor paletteHighlightedText READ paletteHighlightedText WRITE setPaletteHighlightedText STORED false)
public:
    QColor paletteHighlightedText() const { return m_paletteHighlightedText; }
    void setPaletteHighlightedText(const QColor &paletteHighlightedText) { m_paletteHighlightedText = paletteHighlightedText; }
private:
    QColor m_paletteHighlightedText;

    /// paletteLink
private:
    Q_PROPERTY(QColor paletteLink READ paletteLink WRITE setPaletteLink STORED false)
public:
    QColor paletteLink() const { return m_paletteLink; }
    void setPaletteLink(const QColor &paletteLink) { m_paletteLink = paletteLink; }
private:
    QColor m_paletteLink;

    /// paletteLinkVisited
private:
    Q_PROPERTY(QColor paletteLinkVisited READ paletteLinkVisited WRITE setPaletteLinkVisited STORED false)
public:
    QColor paletteLinkVisited() const { return m_paletteLinkVisited; }
    void setPaletteLinkVisited(const QColor &paletteLinkVisited) { m_paletteLinkVisited = paletteLinkVisited; }
private:
    QColor m_paletteLinkVisited;

    //

    /// paletteLight
private:
    Q_PROPERTY(QColor paletteLight READ paletteLight WRITE setPaletteLight STORED false)
public:
    QColor paletteLight() const { return m_paletteLight; }
    void setPaletteLight(const QColor &paletteLight) { m_paletteLight = paletteLight; }
private:
    QColor m_paletteLight;

    /// paletteMidlight
private:
    Q_PROPERTY(QColor paletteMidlight READ paletteMidlight WRITE setPaletteMidlight STORED false)
public:
    QColor paletteMidlight() const { return m_paletteMidlight; }
    void setPaletteMidlight(const QColor &paletteMidlight) { m_paletteMidlight = paletteMidlight; }
private:
    QColor m_paletteMidlight;

    /// paletteDark
private:
    Q_PROPERTY(QColor paletteDark READ paletteDark WRITE setPaletteDark STORED false)
public:
    QColor paletteDark() const { return m_paletteDark; }
    void setPaletteDark(const QColor &paletteDark) { m_paletteDark = paletteDark; }
private:
    QColor m_paletteDark;

    /// paletteMid
private:
    Q_PROPERTY(QColor paletteMid READ paletteMid WRITE setPaletteMid STORED false)
public:
    QColor paletteMid() const { return m_paletteMid; }
    void setPaletteMid(const QColor &paletteMid) { m_paletteMid = paletteMid; }
private:
    QColor m_paletteMid;

    /// paletteShadow
private:
    Q_PROPERTY(QColor paletteShadow READ paletteShadow WRITE setPaletteShadow STORED false)
public:
    QColor paletteShadow() const { return m_paletteShadow; }
    void setPaletteShadow(const QColor &paletteShadow) { m_paletteShadow = paletteShadow; }
private:
    QColor m_paletteShadow;

    //

    /// paletteWindowDisabled
private:
    Q_PROPERTY(QColor paletteWindowDisabled READ paletteWindowDisabled WRITE setPaletteWindowDisabled STORED false)
public:
    QColor paletteWindowDisabled() const { return m_paletteWindowDisabled; }
    void setPaletteWindowDisabled(const QColor &paletteWindowDisabled) { m_paletteWindowDisabled = paletteWindowDisabled; }
private:
    QColor m_paletteWindowDisabled;

    /// paletteWindowTextDisabled
private:
    Q_PROPERTY(QColor paletteWindowTextDisabled READ paletteWindowTextDisabled WRITE setPaletteWindowTextDisabled STORED false)
public:
    QColor paletteWindowTextDisabled() const { return m_paletteWindowTextDisabled; }
    void setPaletteWindowTextDisabled(const QColor &paletteWindowTextDisabled) { m_paletteWindowTextDisabled = paletteWindowTextDisabled; }
private:
    QColor m_paletteWindowTextDisabled;

    /// paletteBaseDisabled
private:
    Q_PROPERTY(QColor paletteBaseDisabled READ paletteBaseDisabled WRITE setPaletteBaseDisabled STORED false)
public:
    QColor paletteBaseDisabled() const { return m_paletteBaseDisabled; }
    void setPaletteBaseDisabled(const QColor &paletteBaseDisabled) { m_paletteBaseDisabled = paletteBaseDisabled; }
private:
    QColor m_paletteBaseDisabled;

    /// paletteAlternateBaseDisabled
private:
    Q_PROPERTY(QColor paletteAlternateBaseDisabled READ paletteAlternateBaseDisabled WRITE setPaletteAlternateBaseDisabled STORED false)
public:
    QColor paletteAlternateBaseDisabled() const { return m_paletteAlternateBaseDisabled; }
    void setPaletteAlternateBaseDisabled(const QColor &paletteAlternateBaseDisabled) { m_paletteAlternateBaseDisabled = paletteAlternateBaseDisabled; }
private:
    QColor m_paletteAlternateBaseDisabled;

    /// paletteToolTipBaseDisabled
private:
    Q_PROPERTY(QColor paletteToolTipBaseDisabled READ paletteToolTipBaseDisabled WRITE setPaletteToolTipBaseDisabled STORED false)
public:
    QColor paletteToolTipBaseDisabled() const { return m_paletteToolTipBaseDisabled; }
    void setPaletteToolTipBaseDisabled(const QColor &paletteToolTipBaseDisabled) { m_paletteToolTipBaseDisabled = paletteToolTipBaseDisabled; }
private:
    QColor m_paletteToolTipBaseDisabled;

    /// paletteToolTipTextDisabled
private:
    Q_PROPERTY(QColor paletteToolTipTextDisabled READ paletteToolTipTextDisabled WRITE setPaletteToolTipTextDisabled STORED false)
public:
    QColor paletteToolTipTextDisabled() const { return m_paletteToolTipTextDisabled; }
    void setPaletteToolTipTextDisabled(const QColor &paletteToolTipTextDisabled) { m_paletteToolTipTextDisabled = paletteToolTipTextDisabled; }
private:
    QColor m_paletteToolTipTextDisabled;

    /// paletteTextDisabled
private:
    Q_PROPERTY(QColor paletteTextDisabled READ paletteTextDisabled WRITE setPaletteTextDisabled STORED false)
public:
    QColor paletteTextDisabled() const { return m_paletteTextDisabled; }
    void setPaletteTextDisabled(const QColor &paletteTextDisabled) { m_paletteTextDisabled = paletteTextDisabled; }
private:
    QColor m_paletteTextDisabled;

    /// paletteButtonDisabled
private:
    Q_PROPERTY(QColor paletteButtonDisabled READ paletteButtonDisabled WRITE setPaletteButtonDisabled STORED false)
public:
    QColor paletteButtonDisabled() const { return m_paletteButtonDisabled; }
    void setPaletteButtonDisabled(const QColor &paletteButtonDisabled) { m_paletteButtonDisabled = paletteButtonDisabled; }
private:
    QColor m_paletteButtonDisabled;

    /// paletteButtonTextDisabled
private:
    Q_PROPERTY(QColor paletteButtonTextDisabled READ paletteButtonTextDisabled WRITE setPaletteButtonTextDisabled STORED false)
public:
    QColor paletteButtonTextDisabled() const { return m_paletteButtonTextDisabled; }
    void setPaletteButtonTextDisabled(const QColor &paletteButtonTextDisabled) { m_paletteButtonTextDisabled = paletteButtonTextDisabled; }
private:
    QColor m_paletteButtonTextDisabled;

    /// paletteBrightTextDisabled
private:
    Q_PROPERTY(QColor paletteBrightTextDisabled READ paletteBrightTextDisabled WRITE setPaletteBrightTextDisabled STORED false)
public:
    QColor paletteBrightTextDisabled() const { return m_paletteBrightTextDisabled; }
    void setPaletteBrightTextDisabled(const QColor &paletteBrightTextDisabled) { m_paletteBrightTextDisabled = paletteBrightTextDisabled; }
private:
    QColor m_paletteBrightTextDisabled;

    /// paletteHighlightDisabled
private:
    Q_PROPERTY(QColor paletteHighlightDisabled READ paletteHighlightDisabled WRITE setPaletteHighlightDisabled STORED false)
public:
    QColor paletteHighlightDisabled() const { return m_paletteHighlightDisabled; }
    void setPaletteHighlightDisabled(const QColor &paletteHighlightDisabled) { m_paletteHighlightDisabled = paletteHighlightDisabled; }
private:
    QColor m_paletteHighlightDisabled;

    /// paletteHighlightedTextDisabled
private:
    Q_PROPERTY(QColor paletteHighlightedTextDisabled READ paletteHighlightedTextDisabled WRITE setPaletteHighlightedTextDisabled STORED false)
public:
    QColor paletteHighlightedTextDisabled() const { return m_paletteHighlightedTextDisabled; }
    void setPaletteHighlightedTextDisabled(const QColor &paletteHighlightedTextDisabled) { m_paletteHighlightedTextDisabled = paletteHighlightedTextDisabled; }
private:
    QColor m_paletteHighlightedTextDisabled;

    /// paletteLinkDisabled
private:
    Q_PROPERTY(QColor paletteLinkDisabled READ paletteLinkDisabled WRITE setPaletteLinkDisabled STORED false)
public:
    QColor paletteLinkDisabled() const { return m_paletteLinkDisabled; }
    void setPaletteLinkDisabled(const QColor &paletteLinkDisabled) { m_paletteLinkDisabled = paletteLinkDisabled; }
private:
    QColor m_paletteLinkDisabled;

    /// paletteLinkVisitedDisabled
private:
    Q_PROPERTY(QColor paletteLinkVisitedDisabled READ paletteLinkVisitedDisabled WRITE setPaletteLinkVisitedDisabled STORED false)
public:
    QColor paletteLinkVisitedDisabled() const { return m_paletteLinkVisitedDisabled; }
    void setPaletteLinkVisitedDisabled(const QColor &paletteLinkVisitedDisabled) { m_paletteLinkVisitedDisabled = paletteLinkVisitedDisabled; }
private:
    QColor m_paletteLinkVisitedDisabled;

    //

    /// paletteLightDisabled
private:
    Q_PROPERTY(QColor paletteLightDisabled READ paletteLightDisabled WRITE setPaletteLightDisabled STORED false)
public:
    QColor paletteLightDisabled() const { return m_paletteLightDisabled; }
    void setPaletteLightDisabled(const QColor &paletteLightDisabled) { m_paletteLightDisabled = paletteLightDisabled; }
private:
    QColor m_paletteLightDisabled;

    /// paletteMidlightDisabled
private:
    Q_PROPERTY(QColor paletteMidlightDisabled READ paletteMidlightDisabled WRITE setPaletteMidlightDisabled STORED false)
public:
    QColor paletteMidlightDisabled() const { return m_paletteMidlightDisabled; }
    void setPaletteMidlightDisabled(const QColor &paletteMidlightDisabled) { m_paletteMidlightDisabled = paletteMidlightDisabled; }
private:
    QColor m_paletteMidlightDisabled;

    /// paletteDarkDisabled
private:
    Q_PROPERTY(QColor paletteDarkDisabled READ paletteDarkDisabled WRITE setPaletteDarkDisabled STORED false)
public:
    QColor paletteDarkDisabled() const { return m_paletteDarkDisabled; }
    void setPaletteDarkDisabled(const QColor &paletteDarkDisabled) { m_paletteDarkDisabled = paletteDarkDisabled; }
private:
    QColor m_paletteDarkDisabled;

    /// paletteMidDisabled
private:
    Q_PROPERTY(QColor paletteMidDisabled READ paletteMidDisabled WRITE setPaletteMidDisabled STORED false)
public:
    QColor paletteMidDisabled() const { return m_paletteMidDisabled; }
    void setPaletteMidDisabled(const QColor &paletteMidDisabled) { m_paletteMidDisabled = paletteMidDisabled; }
private:
    QColor m_paletteMidDisabled;

    /// paletteShadowDisabled
private:
    Q_PROPERTY(QColor paletteShadowDisabled READ paletteShadowDisabled WRITE setPaletteShadowDisabled STORED false)
public:
    QColor paletteShadowDisabled() const { return m_paletteShadowDisabled; }
    void setPaletteShadowDisabled(const QColor &paletteShadowDisabled) { m_paletteShadowDisabled = paletteShadowDisabled; }
private:
    QColor m_paletteShadowDisabled;

};

#endif // QTUTILS_THEMEMANAGER_P_H_INCLUDED

