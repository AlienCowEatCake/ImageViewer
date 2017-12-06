/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

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

#if !defined(TOOLBAR_H_INCLUDED)
#define TOOLBAR_H_INCLUDED

#include "Utils/ScopedPointer.h"
#include "Widgets/AdjustableFrame.h"

#include "IControlsContainer.h"

class ToolBar : public AdjustableFrame, public IControlsContainer
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolBar)

    DECLARE_CONTROLS_CONTAINER_FUNCTIONS

signals:
    void openRequested();
    void saveAsRequested();
    void newWindowRequested();
    void navigatePreviousRequested();
    void navigateNextRequested();
    void startSlideShowRequested();
    void preferencesRequested();
    void exitRequested();
    void rotateCounterclockwiseRequested();
    void rotateClockwiseRequested();
    void flipHorizontalRequested();
    void flipVerticalRequested();
    void deleteFileRequested();
    void zoomOutRequested();
    void zoomInRequested();
    void zoomResetRequested();
    void zoomFitToWindowRequested();
    void zoomOriginalSizeRequested();
    void zoomFullScreenRequested();
    void showMenuBarRequested();
    void showToolBarRequested();
    void aboutRequested();
    void aboutQtRequested();

public:
    ToolBar(QWidget *parent = NULL);
    ~ToolBar();

protected:
    void changeEvent(QEvent *event);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // TOOLBAR_H_INCLUDED
