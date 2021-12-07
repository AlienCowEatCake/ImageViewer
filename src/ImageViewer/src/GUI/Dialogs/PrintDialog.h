/*
   Copyright (C) 2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (PRINT_DIALOG_H_INCLUDED)
#define PRINT_DIALOG_H_INCLUDED

#include <QDialog>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

class QGraphicsItem;
class QPrinterInfo;

class PrintDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(PrintDialog)

public:
    PrintDialog(QGraphicsItem *graphicsItem, QWidget *parent = Q_NULLPTR);
    ~PrintDialog();

private Q_SLOTS:
    void onCurrentPrinterChanged(int index);
    void onPrintClicked();

    void updatePrinterInfo(const QPrinterInfo& info);

private:
    struct UI;
    QScopedPointer<UI> m_ui;
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // PRINT_DIALOG_H_INCLUDED
