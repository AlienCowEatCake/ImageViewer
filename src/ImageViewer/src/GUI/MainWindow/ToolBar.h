/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

Q_SIGNALS:
    void polished();

public:
    explicit ToolBar(QWidget *parent = Q_NULLPTR);
    ~ToolBar();

    ControlsContainerEmitter *emitter() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onPolished();
    void onIconThemeChanged();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // TOOLBAR_H_INCLUDED
