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

#if !defined(ANIMATIONWIDGET_H_INCLUDED)
#define ANIMATIONWIDGET_H_INCLUDED

#include <QLabel>
#include "Utils/ScopedPointer.h"

class IAnimationProvider;

class AnimationWidget : public QLabel
{
    Q_OBJECT

public:
    explicit AnimationWidget(QWidget *parent = NULL, Qt::WindowFlags flags = Qt::WindowFlags());
    ~AnimationWidget();

public slots:
    void setAnimationProvider(IAnimationProvider *provider);
    void clear();

private slots:
    void animationUpdated();

private:
    Q_DISABLE_COPY(AnimationWidget)
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // ANIMATIONWIDGET_H_INCLUDED
