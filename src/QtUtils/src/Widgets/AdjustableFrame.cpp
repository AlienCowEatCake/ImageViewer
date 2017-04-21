/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "AdjustableFrame.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>

namespace {
const int INVALID_INT_VALUE = -1;
} // namespace

AdjustableFrame::AdjustableFrame(QWidget *parent, Qt::WindowFlags flags)
    : QFrame(parent, flags)
{}

int AdjustableFrame::layoutMargin() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    const QMargins margins = layout()->contentsMargins();
    if(margins.left() == margins.right() && margins.top() == margins.bottom() && margins.left() == margins.top())
        return margins.left();
    return INVALID_INT_VALUE;
}

void AdjustableFrame::setLayoutMargin(int margin)
{
    if(!layout())
        return;
    layout()->setContentsMargins(margin, margin, margin, margin);
}

int AdjustableFrame::layoutMarginLeft() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    return layout()->contentsMargins().left();
}

void AdjustableFrame::setLayoutMarginLeft(int marginLeft)
{
    if(!layout())
        return;
    QMargins margins = layout()->contentsMargins();
    margins.setLeft(marginLeft);
    layout()->setContentsMargins(margins);
}

int AdjustableFrame::layoutMarginRight() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    return layout()->contentsMargins().right();
}

void AdjustableFrame::setLayoutMarginRight(int marginRight)
{
    if(!layout())
        return;
    QMargins margins = layout()->contentsMargins();
    margins.setRight(marginRight);
    layout()->setContentsMargins(margins);
}

int AdjustableFrame::layoutMarginTop() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    return layout()->contentsMargins().top();
}

void AdjustableFrame::setLayoutMarginTop(int marginTop)
{
    if(!layout())
        return;
    QMargins margins = layout()->contentsMargins();
    margins.setTop(marginTop);
    layout()->setContentsMargins(margins);
}

int AdjustableFrame::layoutMarginBottom() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    return layout()->contentsMargins().bottom();
}

void AdjustableFrame::setLayoutMarginBottom(int marginBottom)
{
    if(!layout())
        return;
    QMargins margins = layout()->contentsMargins();
    margins.setBottom(marginBottom);
    layout()->setContentsMargins(margins);
}

int AdjustableFrame::layoutSpacing() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    return layout()->spacing();
}

void AdjustableFrame::setLayoutSpacing(int spacing)
{
    if(!layout())
        return;
    layout()->setSpacing(spacing);
}

int AdjustableFrame::layoutHSpacing() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    if(gridLayout)
        return gridLayout->horizontalSpacing();
    QFormLayout *formLayout = qobject_cast<QFormLayout*>(layout());
    if(formLayout)
        return formLayout->horizontalSpacing();
    QHBoxLayout *hBoxLayout = qobject_cast<QHBoxLayout*>(layout());
    if(hBoxLayout)
        return hBoxLayout->spacing();
    return INVALID_INT_VALUE;
}

void AdjustableFrame::setLayoutHSpacing(int hSpacing)
{
    if(!layout())
        return;
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    if(gridLayout)
        return gridLayout->setHorizontalSpacing(hSpacing);
    QFormLayout *formLayout = qobject_cast<QFormLayout*>(layout());
    if(formLayout)
        return formLayout->setHorizontalSpacing(hSpacing);
    QHBoxLayout *hBoxLayout = qobject_cast<QHBoxLayout*>(layout());
    if(hBoxLayout)
        return hBoxLayout->setSpacing(hSpacing);
}

int AdjustableFrame::layoutVSpacing() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    if(gridLayout)
        return gridLayout->verticalSpacing();
    QFormLayout *formLayout = qobject_cast<QFormLayout*>(layout());
    if(formLayout)
        return formLayout->verticalSpacing();
    QVBoxLayout *vBoxLayout = qobject_cast<QVBoxLayout*>(layout());
    if(vBoxLayout)
        return vBoxLayout->spacing();
    return INVALID_INT_VALUE;
}

void AdjustableFrame::setLayoutVSpacing(int vSpacing)
{
    if(!layout())
        return;
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    if(gridLayout)
        return gridLayout->setVerticalSpacing(vSpacing);
    QFormLayout *formLayout = qobject_cast<QFormLayout*>(layout());
    if(formLayout)
        return formLayout->setVerticalSpacing(vSpacing);
    QVBoxLayout *vBoxLayout = qobject_cast<QVBoxLayout*>(layout());
    if(vBoxLayout)
        return vBoxLayout->setSpacing(vSpacing);
}
