/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(ABSTRACT_PROGRESSIVE_IMAGE_PROVIDER_H_INCLUDED)
#define ABSTRACT_PROGRESSIVE_IMAGE_PROVIDER_H_INCLUDED

#include <QObject>

class QImage;
class QSize;

class AbstractProgressiveImageProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractProgressiveImageProvider)

signals:
    void updated();

public:
    AbstractProgressiveImageProvider(QObject *parent = NULL)
        : QObject(parent)
    {}

    virtual bool isValid() const = 0;
    virtual bool isFinal() const = 0;
    virtual QSize size() const = 0;
    virtual QImage image() const = 0;
};

#endif // ABSTRACT_PROGRESSIVE_IMAGE_PROVIDER_H_INCLUDED
