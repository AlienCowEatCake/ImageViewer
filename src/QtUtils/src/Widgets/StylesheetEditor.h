/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_STYLESHEET_EDITOR_H_INCLUDED)
#define QTUTILS_STYLESHEET_EDITOR_H_INCLUDED

#include <QDialog>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

class StylesheetEditor : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(StylesheetEditor)

public:
    explicit StylesheetEditor(QWidget *parent = Q_NULLPTR);
    ~StylesheetEditor();

    bool isProtected() const;
    void setProtected(bool isProtected);

private slots:
    void applyStylesheet();
    void resetStyleSheet();
    void readStyleSheet();
    void updateProtection();
    void updateAutoApply();

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // QTUTILS_STYLESHEET_EDITOR_H_INCLUDED

