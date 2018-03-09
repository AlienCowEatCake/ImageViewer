/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "AnimationWidget.h"
#include "AnimationObject.h"
#include "IAnimationProvider.h"

struct AnimationWidget::Impl
{
    Impl(AnimationWidget *widget)
        : widget(widget)
        , animation(NULL)
    {}

    AnimationWidget *widget;
    AnimationObject *animation;
};

AnimationWidget::AnimationWidget(QWidget *parent, Qt::WindowFlags flags)
    : QLabel(parent, flags)
    , m_impl(new Impl(this))
{}

AnimationWidget::~AnimationWidget()
{}

void AnimationWidget::setAnimationProvider(IAnimationProvider *provider)
{
    clear();
    if(!provider)
        return;

    m_impl->animation = new AnimationObject(provider, this);
    connect(m_impl->animation, SIGNAL(updated()), this, SLOT(animationUpdated()));
    animationUpdated();
}

void AnimationWidget::clear()
{
    if(m_impl->animation)
    {
        disconnect(m_impl->animation, SIGNAL(updated()), this, SLOT(animationUpdated()));
        m_impl->animation->deleteLater();
    }
    m_impl->animation = NULL;
    QLabel::clear();
}

void AnimationWidget::animationUpdated()
{
    if(m_impl->animation && m_impl->animation->isValid())
    {
        setPixmap(m_impl->animation->currentPixmap());
        resize(pixmap()->size());
    }
}
