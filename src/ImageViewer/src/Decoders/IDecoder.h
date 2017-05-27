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

#if !defined(IDECODER_H_INCLUDED)
#define IDECODER_H_INCLUDED

#include <QStringList>

class QGraphicsItem;

class IDecoder
{
public:
    virtual ~IDecoder() {}
    /// @brief Название декодера, должно быть уникальным
    virtual QString name() const = 0;
    /// @brief Основные (приоритетные) форматы, поддерживаемые декодером
    virtual QStringList supportedFormats() const = 0;
    /// @brief Дополнительные (не приоритетные) форматы, которые можно попытаться открыть
    virtual QStringList advancedFormats() const = 0;
    /// @brief Загрузка изображения по заданному пути в элемент сцены
    virtual QGraphicsItem *loadImage(const QString &filePath) = 0;
};

#endif
