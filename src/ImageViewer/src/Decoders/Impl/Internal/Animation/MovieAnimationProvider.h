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

#if !defined(MOVIE_ANIMATION_PROVIDER_H_INCLUDED)
#define MOVIE_ANIMATION_PROVIDER_H_INCLUDED

#include <QImage>
#include <QPixmap>
#include "IAnimationProvider.h"

template<typename Movie>
class MovieAnimationProvider : public IAnimationProvider
{
public:
    MovieAnimationProvider(Movie *movie)
        : m_movie(movie)
    {
        m_movie->setParent(NULL);
        m_movie->start();
        m_movie->stop();
    }

    ~MovieAnimationProvider()
    {
        m_movie->deleteLater();
    }

    bool isValid() const
    {
        return m_movie->isValid();
    }

    bool isSingleFrame() const
    {
        return m_movie->frameCount() == 1;
    }

    int nextImageDelay() const
    {
        return m_movie->nextFrameDelay();
    }

    bool jumpToNextImage()
    {
        return m_movie->jumpToNextFrame();
    }

    QPixmap currentPixmap() const
    {
        return m_movie->currentPixmap();
    }

    QImage currentImage() const
    {
        return m_movie->currentImage();
    }

protected:
    Movie *m_movie;
};

#endif // MOVIE_ANIMATION_PROVIDER_H_INCLUDED
