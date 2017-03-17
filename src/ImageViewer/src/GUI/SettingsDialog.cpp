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

#include "SettingsDialog.h"
#include "SettingsDialog_p.h"

#include <QColorDialog>

struct SettingsDialog::Impl
{
    Impl(SettingsDialog *widget, GUISettings *settings)
        : dialog(widget)
        , ui(dialog->m_ui.data())
        , settings(settings)
    {
        onBackgroundColorChanged(settings->backgroundColor());
        QObject::connect(ui->backgroundColorButton, SIGNAL(clicked()), dialog, SLOT(onColorDialogRequested()));
        QObject::connect(ui->buttonBox, SIGNAL(rejected()), dialog, SLOT(close()));
        QObject::connect(ui->buttonBox, SIGNAL(accepted()), dialog, SLOT(onSettingsAccepted()));
        QObject::connect(settings, SIGNAL(backgroundColorChanged(const QColor&)), dialog, SLOT(onBackgroundColorChanged(const QColor&)));
    }

    void onSettingsAccepted()
    {
        settings->setAskBeforeDelete(ui->askBeforeDeleteCheckbox->isChecked());
        settings->setMoveToTrash(ui->moveToTrashCheckbox->isChecked());
        settings->setSmoothTransformation(ui->smoothTransformationCheckbox->isChecked());
        settings->setBackgroundColor(background);
        dialog->close();
    }

    void onBackgroundColorChanged(const QColor &color)
    {
        background = color;
        ui->updateBackgroundColorButton(color);
    }

    SettingsDialog *dialog;
    SettingsDialog::UI *ui;
    GUISettings *settings;
    QColor background;
};

SettingsDialog::SettingsDialog(GUISettings *settings, QWidget *parent)
    : QDialog(parent)
    , m_ui(new UI(this, settings))
    , m_impl(new Impl(this, settings))
{}

SettingsDialog::~SettingsDialog()
{}

void SettingsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    adjustSize();
}

void SettingsDialog::onSettingsAccepted()
{
    m_impl->onSettingsAccepted();
}

void SettingsDialog::onBackgroundColorChanged(const QColor &color)
{
    m_impl->onBackgroundColorChanged(color);
}

void SettingsDialog::onColorDialogRequested()
{
    const QColor oldColor = m_impl->settings->backgroundColor();
#if !defined (Q_OS_MAC)
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    QColorDialog dialog(this);
    dialog.setOption(QColorDialog::ShowAlphaChannel, true);
    dialog.setCurrentColor(oldColor);
    setWindowTitle(tr("Select Background Color"));
    dialog.exec();
    const QColor newColor = dialog.currentColor();
#else
    bool ok = true;
    const QColor newColor = QColorDialog::getRgba(oldColor.rgba(), &ok, this);
    if(!ok)
        return;
#endif
    if(newColor.isValid() && newColor != oldColor)
        onBackgroundColorChanged(newColor);
#else
    QColorDialog dialog(this);
    dialog.setOption(QColorDialog::NoButtons);
    dialog.setOption(QColorDialog::ShowAlphaChannel, true);
    dialog.setCurrentColor(oldColor);
    connect(&dialog, SIGNAL(colorSelected(const QColor&)), this, SLOT(onBackgroundColorChanged(const QColor&)));
    dialog.exec();
#endif
}
