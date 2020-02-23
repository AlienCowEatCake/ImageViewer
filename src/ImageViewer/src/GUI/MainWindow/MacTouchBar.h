/*
   Copyright (C) 2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(MAC_TOUCHBAR_H_INCLUDED)
#define MAC_TOUCHBAR_H_INCLUDED

#include "Utils/ScopedPointer.h"

#include "IControlsContainer.h"

class MacTouchBar : public ControlsContainerEmitter, public IControlsContainer
{
    Q_OBJECT
    Q_DISABLE_COPY(MacTouchBar)

    DECLARE_CONTROLS_CONTAINER_FUNCTIONS

public:
    explicit MacTouchBar(QObject *parent = Q_NULLPTR);
    ~MacTouchBar();

    ControlsContainerEmitter *emitter() Q_DECL_OVERRIDE;

    void attachToWindow(QWidget *widget);
    void detachFromWindow();

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MAC_TOUCHBAR_H_INCLUDED
