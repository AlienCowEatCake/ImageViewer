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

#if !defined(ABOUTDIALOG_P_H_INCLUDED)
#define ABOUTDIALOG_P_H_INCLUDED

#include "AboutDialog.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QGridLayout>
#include <QApplication>
#include <QPixmap>

#include "Utils/ObjectsUtils.h"

namespace {

const int LAYOUT_SPACING = 14;
const int LAYOUT_MARGINS = 14;
const int TEXTBROWSER_HEIGHT = 80;

} // namespace

struct AboutDialog::UI
{
    QLabel *iconLabel;
    QLabel *titleLabel;
    QLabel *textLabel;
    QTextBrowser *textBrowser;
    QDialogButtonBox *buttonBox;

    UI(QWidget *parent)
        : CONSTRUCT_OBJECT(iconLabel, QLabel, (parent))
        , CONSTRUCT_OBJECT(titleLabel, QLabel, (parent))
        , CONSTRUCT_OBJECT(textLabel, QLabel, (parent))
        , CONSTRUCT_OBJECT(textBrowser, QTextBrowser, (parent))
        , CONSTRUCT_OBJECT(buttonBox, QDialogButtonBox, (QDialogButtonBox::Ok, parent))
    {
        QGridLayout *grid = new QGridLayout(parent);
        grid->setContentsMargins(LAYOUT_MARGINS, LAYOUT_MARGINS, LAYOUT_MARGINS, LAYOUT_MARGINS);
        grid->setSpacing(LAYOUT_SPACING);
        grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignCenter);
        grid->addWidget(titleLabel, 0, 1, 1, 1, Qt::AlignTop | Qt::AlignLeft);
        grid->addWidget(textLabel, 1, 1, 1, 1, Qt::AlignTop | Qt::AlignLeft);
        grid->addWidget(textBrowser, 2, 0, 1, 2, Qt::AlignCenter);
        grid->addWidget(buttonBox, 3, 0, 1, 2, Qt::AlignVCenter | Qt::AlignRight);

        textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        textBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        textBrowser->setOpenExternalLinks(true);
        textBrowser->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textBrowser->setFixedHeight(TEXTBROWSER_HEIGHT);

        QPalette palette = textBrowser->palette();
        palette.setColor(QPalette::Base, palette.color(QPalette::Window));
        textBrowser->setPalette(palette);

        titleLabel->setWordWrap(false);
        textLabel->setWordWrap(false);
        textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        parent->ensurePolished();
        parent->adjustSize();
        parent->setFixedSize(parent->minimumSize());
    }
};


#endif // ABOUTDIALOG_P_H_INCLUDED
