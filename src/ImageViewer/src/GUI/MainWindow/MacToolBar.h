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

#if !defined(MAC_TOOLBAR_H_INCLUDED)
#define MAC_TOOLBAR_H_INCLUDED

#include "Utils/ScopedPointer.h"

#include "IControlsContainer.h"

class MacToolBar : public ControlsContainerEmitter, public IControlsContainer
{
    Q_OBJECT
    Q_DISABLE_COPY(MacToolBar)

    DECLARE_CONTROLS_CONTAINER_FUNCTIONS

public:
    MacToolBar(QObject *parent = NULL);
    ~MacToolBar();

    ControlsContainerEmitter *emitter();

    void attachToWindow(QWidget *widget);
    void detachFromWindow();

    void setVisible(bool isVisible);

private slots:
    void retranslate();

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MAC_TOOLBAR_H_INCLUDED
