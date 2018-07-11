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

    Q_PROPERTY(QString preferredStyles MEMBER preferredStyles STORED false)

    Q_PROPERTY(QColor paletteWindow MEMBER paletteWindow STORED false)
    Q_PROPERTY(QColor paletteWindowText MEMBER paletteWindowText STORED false)
    Q_PROPERTY(QColor paletteBase MEMBER paletteBase STORED false)
    Q_PROPERTY(QColor paletteAlternateBase MEMBER paletteAlternateBase STORED false)
    Q_PROPERTY(QColor paletteToolTipBase MEMBER paletteToolTipBase STORED false)
    Q_PROPERTY(QColor paletteToolTipText MEMBER paletteToolTipText STORED false)
    Q_PROPERTY(QColor paletteText MEMBER paletteText STORED false)
    Q_PROPERTY(QColor paletteButton MEMBER paletteButton STORED false)
    Q_PROPERTY(QColor paletteButtonText MEMBER paletteButtonText STORED false)
    Q_PROPERTY(QColor paletteBrightText MEMBER paletteBrightText STORED false)
    Q_PROPERTY(QColor paletteHighlight MEMBER paletteHighlight STORED false)
    Q_PROPERTY(QColor paletteHighlightedText MEMBER paletteHighlightedText STORED false)
    Q_PROPERTY(QColor paletteLink MEMBER paletteLink STORED false)
    Q_PROPERTY(QColor paletteLinkVisited MEMBER paletteLinkVisited STORED false)

    Q_PROPERTY(QColor paletteLight MEMBER paletteLight STORED false)
    Q_PROPERTY(QColor paletteMidlight MEMBER paletteMidlight STORED false)
    Q_PROPERTY(QColor paletteDark MEMBER paletteDark STORED false)
    Q_PROPERTY(QColor paletteMid MEMBER paletteMid STORED false)
    Q_PROPERTY(QColor paletteShadow MEMBER paletteShadow STORED false)

    Q_PROPERTY(QColor paletteWindowDisabled MEMBER paletteWindowDisabled STORED false)
    Q_PROPERTY(QColor paletteWindowTextDisabled MEMBER paletteWindowTextDisabled STORED false)
    Q_PROPERTY(QColor paletteBaseDisabled MEMBER paletteBaseDisabled STORED false)
    Q_PROPERTY(QColor paletteAlternateBaseDisabled MEMBER paletteAlternateBaseDisabled STORED false)
    Q_PROPERTY(QColor paletteToolTipBaseDisabled MEMBER paletteToolTipBaseDisabled STORED false)
    Q_PROPERTY(QColor paletteToolTipTextDisabled MEMBER paletteToolTipTextDisabled STORED false)
    Q_PROPERTY(QColor paletteTextDisabled MEMBER paletteTextDisabled STORED false)
    Q_PROPERTY(QColor paletteButtonDisabled MEMBER paletteButtonDisabled STORED false)
    Q_PROPERTY(QColor paletteButtonTextDisabled MEMBER paletteButtonTextDisabled STORED false)
    Q_PROPERTY(QColor paletteBrightTextDisabled MEMBER paletteBrightTextDisabled STORED false)
    Q_PROPERTY(QColor paletteHighlightDisabled MEMBER paletteHighlightDisabled STORED false)
    Q_PROPERTY(QColor paletteHighlightedTextDisabled MEMBER paletteHighlightedTextDisabled STORED false)
    Q_PROPERTY(QColor paletteLinkDisabled MEMBER paletteLinkDisabled STORED false)
    Q_PROPERTY(QColor paletteLinkVisitedDisabled MEMBER paletteLinkVisitedDisabled STORED false)

    Q_PROPERTY(QColor paletteLightDisabled MEMBER paletteLightDisabled STORED false)
    Q_PROPERTY(QColor paletteMidlightDisabled MEMBER paletteMidlightDisabled STORED false)
    Q_PROPERTY(QColor paletteDarkDisabled MEMBER paletteDarkDisabled STORED false)
    Q_PROPERTY(QColor paletteMidDisabled MEMBER paletteMidDisabled STORED false)
    Q_PROPERTY(QColor paletteShadowDisabled MEMBER paletteShadowDisabled STORED false)

public:
    StylableTheme()
    {
        ensurePolished();
    }

    QString preferredStyles;

    QColor paletteWindow;
    QColor paletteWindowText;
    QColor paletteBase;
    QColor paletteAlternateBase;
    QColor paletteToolTipBase;
    QColor paletteToolTipText;
    QColor paletteText;
    QColor paletteButton;
    QColor paletteButtonText;
    QColor paletteBrightText;
    QColor paletteHighlight;
    QColor paletteHighlightedText;
    QColor paletteLink;
    QColor paletteLinkVisited;

    QColor paletteLight;
    QColor paletteMidlight;
    QColor paletteDark;
    QColor paletteMid;
    QColor paletteShadow;

    QColor paletteWindowDisabled;
    QColor paletteWindowTextDisabled;
    QColor paletteBaseDisabled;
    QColor paletteAlternateBaseDisabled;
    QColor paletteToolTipBaseDisabled;
    QColor paletteToolTipTextDisabled;
    QColor paletteTextDisabled;
    QColor paletteButtonDisabled;
    QColor paletteButtonTextDisabled;
    QColor paletteBrightTextDisabled;
    QColor paletteHighlightDisabled;
    QColor paletteHighlightedTextDisabled;
    QColor paletteLinkDisabled;
    QColor paletteLinkVisitedDisabled;

    QColor paletteLightDisabled;
    QColor paletteMidlightDisabled;
    QColor paletteDarkDisabled;
    QColor paletteMidDisabled;
    QColor paletteShadowDisabled;
};

#endif // QTUTILS_THEMEMANAGER_P_H_INCLUDED

