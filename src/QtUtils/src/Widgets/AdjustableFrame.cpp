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
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    if(left == right && top == bottom && left == top)
        return left;
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
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    return left;
}

void AdjustableFrame::setLayoutMarginLeft(int marginLeft)
{
    if(!layout())
        return;
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    layout()->setContentsMargins(marginLeft, top, right, bottom);
}

int AdjustableFrame::layoutMarginRight() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    return right;
}

void AdjustableFrame::setLayoutMarginRight(int marginRight)
{
    if(!layout())
        return;
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    layout()->setContentsMargins(left, top, marginRight, bottom);
}

int AdjustableFrame::layoutMarginTop() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    return top;
}

void AdjustableFrame::setLayoutMarginTop(int marginTop)
{
    if(!layout())
        return;
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    layout()->setContentsMargins(left, marginTop, right, bottom);
}

int AdjustableFrame::layoutMarginBottom() const
{
    if(!layout())
        return INVALID_INT_VALUE;
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    return bottom;
}

void AdjustableFrame::setLayoutMarginBottom(int marginBottom)
{
    if(!layout())
        return;
    int left = INVALID_INT_VALUE, top = INVALID_INT_VALUE, right = INVALID_INT_VALUE, bottom = INVALID_INT_VALUE;
    layout()->getContentsMargins(&left, &top, &right, &bottom);
    layout()->setContentsMargins(left, top, right, marginBottom);
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
