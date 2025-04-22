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

#if !defined (QTUTILS_ADJUSTABLE_FRAME_H_INCLUDED)
#define QTUTILS_ADJUSTABLE_FRAME_H_INCLUDED

#include <QFrame>

#include "Utils/Global.h"

/// @brief Wrapper around QFrame which give access to associated layout
/// parameters from QSS styles.
class AdjustableFrame : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(AdjustableFrame)

    Q_PROPERTY(int layoutMargin         READ layoutMargin       WRITE setLayoutMargin       STORED false)
    Q_PROPERTY(int layoutMarginLeft     READ layoutMarginLeft   WRITE setLayoutMarginLeft   STORED false)
    Q_PROPERTY(int layoutMarginRight    READ layoutMarginRight  WRITE setLayoutMarginRight  STORED false)
    Q_PROPERTY(int layoutMarginTop      READ layoutMarginTop    WRITE setLayoutMarginTop    STORED false)
    Q_PROPERTY(int layoutMarginBottom   READ layoutMarginBottom WRITE setLayoutMarginBottom STORED false)
    Q_PROPERTY(int layoutSpacing        READ layoutSpacing      WRITE setLayoutSpacing      STORED false)
    Q_PROPERTY(int layoutHSpacing       READ layoutHSpacing     WRITE setLayoutHSpacing     STORED false)
    Q_PROPERTY(int layoutVSpacing       READ layoutVSpacing     WRITE setLayoutVSpacing     STORED false)

public:
    AdjustableFrame(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());

    int layoutMargin() const;
    void setLayoutMargin(int margin);

    int layoutMarginLeft() const;
    void setLayoutMarginLeft(int marginLeft);

    int layoutMarginRight() const;
    void setLayoutMarginRight(int marginRight);

    int layoutMarginTop() const;
    void setLayoutMarginTop(int marginTop);

    int layoutMarginBottom() const;
    void setLayoutMarginBottom(int marginBottom);

    int layoutSpacing() const;
    void setLayoutSpacing(int spacing);

    int layoutHSpacing() const;
    void setLayoutHSpacing(int hSpacing);

    int layoutVSpacing() const;
    void setLayoutVSpacing(int vSpacing);
};

#endif // QTUTILS_ADJUSTABLE_FRAME_H_INCLUDED

