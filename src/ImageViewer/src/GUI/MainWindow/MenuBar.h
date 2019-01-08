/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(MENUBAR_H_INCLUDED)
#define MENUBAR_H_INCLUDED

#include <QMenuBar>

#include "Utils/ScopedPointer.h"

#include "IControlsContainer.h"

class MenuBar : public QMenuBar, public IControlsContainer
{
    Q_OBJECT
    Q_DISABLE_COPY(MenuBar)

    DECLARE_CONTROLS_CONTAINER_FUNCTIONS

public:
    MenuBar(QWidget *parent = Q_NULLPTR);
    ~MenuBar();

    ControlsContainerEmitter *emitter() Q_DECL_OVERRIDE;

    QMenu *contextMenu();
    QMenu *menuReopenWith();

protected:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MENUBAR_H_INCLUDED
