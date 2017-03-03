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

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QPixmap>
#include <QColorDialog>
#include <QDialogButtonBox>

#include "GUISettings.h"

namespace {

const QSize COLOR_BUTTON_SIZE   (40, 24);
const QSize COLOR_ICON_SIZE     (30, 14);

} // namespace

struct SettingsDialog::Impl
{
    Impl(SettingsDialog *widget, GUISettings *settings)
        : settingsDialog(widget)
        , settings(settings)
        , askBeforeDeleteCheckbox(new QCheckBox(settingsDialog))
        , moveToTrashCheckbox(new QCheckBox(settingsDialog))
        , backgroundColorFrame(new QFrame(settingsDialog))
        , backgroundColorLabel(new QLabel(backgroundColorFrame))
        , backgroundColorButton(new QToolButton(backgroundColorFrame))
        , buttonBox(new QDialogButtonBox(settingsDialog))
    {
        askBeforeDeleteCheckbox->setText(qApp->translate("SettingsDialog", "Ask before deleting images"));
        askBeforeDeleteCheckbox->setChecked(settings->askBeforeDelete());

        moveToTrashCheckbox->setText(qApp->translate("SettingsDialog", "Move deleted images to trash"));
        moveToTrashCheckbox->setChecked(settings->moveToTrash());

        backgroundColorLabel->setText(qApp->translate("SettingsDialog", "Background color"));

        backgroundColorButton->resize(COLOR_BUTTON_SIZE);
        backgroundColorButton->setIconSize(COLOR_ICON_SIZE);
        onBackgroundColorChanged(settings->backgroundColor());
        QObject::connect(backgroundColorButton, SIGNAL(clicked()), settingsDialog, SLOT(onColorDialogRequested()));
        QObject::connect(settings, SIGNAL(backgroundColorChanged(const QColor&)), settingsDialog, SLOT(onBackgroundColorChanged(const QColor&)));

        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        QObject::connect(buttonBox, SIGNAL(rejected()), settingsDialog, SLOT(close()));
        QObject::connect(buttonBox, SIGNAL(accepted()), settingsDialog, SLOT(onSettingsAccepted()));

        QHBoxLayout *backgroundColorLayout = new QHBoxLayout(backgroundColorFrame);
        backgroundColorLayout->setContentsMargins(0, 0, 0, 0);
        backgroundColorLayout->addWidget(backgroundColorLabel);
        backgroundColorLayout->addWidget(backgroundColorButton);
        backgroundColorLayout->addStretch();

        QVBoxLayout *mainLayout = new QVBoxLayout(settingsDialog);
        mainLayout->addWidget(askBeforeDeleteCheckbox);
        mainLayout->addWidget(moveToTrashCheckbox);
        mainLayout->addWidget(backgroundColorFrame);
        mainLayout->addWidget(buttonBox);

        settingsDialog->setWindowTitle(qApp->translate("SettingsDialog", "Preferences"));
        settingsDialog->ensurePolished();
        settingsDialog->adjustSize();
        settingsDialog->resize(widget->minimumSize());
        settingsDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                               Qt::WindowCloseButtonHint |
#endif
                               Qt::WindowSystemMenuHint | Qt::MSWindowsFixedSizeDialogHint);
        settingsDialog->setWindowModality(Qt::ApplicationModal);
    }

    void onSettingsAccepted()
    {
        settings->setAskBeforeDelete(askBeforeDeleteCheckbox->isChecked());
        settings->setMoveToTrash(moveToTrashCheckbox->isChecked());
        settings->setBackgroundColor(backgroundColor);
        settingsDialog->close();
    }

    void onBackgroundColorChanged(const QColor &color)
    {
        backgroundColor = color;
        QPixmap pixmap(COLOR_ICON_SIZE);
        pixmap.fill(color);
        backgroundColorButton->setIcon(QIcon(pixmap));
    }

    SettingsDialog *settingsDialog;
    GUISettings *settings;
    QColor backgroundColor;

    QCheckBox *askBeforeDeleteCheckbox;
    QCheckBox *moveToTrashCheckbox;
    QFrame *backgroundColorFrame;
    QLabel *backgroundColorLabel;
    QToolButton *backgroundColorButton;
    QDialogButtonBox *buttonBox;
};

SettingsDialog::SettingsDialog(GUISettings *settings, QWidget *parent)
    : QDialog(parent)
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
    const QColor newColor = QColorDialog::getColor(oldColor, this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
        , tr("Select Background Color")
#endif
        );
    if(newColor.isValid() && newColor != oldColor)
        onBackgroundColorChanged(newColor);
#else
   QColorDialog dialog(oldColor, this);
   dialog.setOption(QColorDialog::NoButtons);
   connect(&dialog, SIGNAL(colorSelected(const QColor&)), this, SLOT(onBackgroundColorChanged(const QColor&)));
   dialog.exec();
#endif
}
