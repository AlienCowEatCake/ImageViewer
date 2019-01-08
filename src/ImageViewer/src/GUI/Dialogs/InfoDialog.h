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

#if !defined (INFO_DIALOG_H_INCLUDED)
#define INFO_DIALOG_H_INCLUDED

#include <QDialog>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"
#include "Utils/SharedPointer.h"

class IImageData;

class InfoDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(InfoDialog)

public:
    InfoDialog(const QSharedPointer<IImageData> &imageData, QWidget *parent = Q_NULLPTR);
    ~InfoDialog();

private:
    struct UI;
    QScopedPointer<UI> m_ui;
};

#endif // INFO_DIALOG_H_INCLUDED
