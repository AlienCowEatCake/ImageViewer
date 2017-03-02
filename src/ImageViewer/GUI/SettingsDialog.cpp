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
#include <QCheckBox>
#include <QDialogButtonBox>

#include "GUISettings.h"

struct SettingsDialog::Impl
{
    Impl(SettingsDialog *widget, GUISettings *settings)
        : settingsDialog(widget)
        , settings(settings)
        , askBeforeDelete(new QCheckBox(settingsDialog))
        , moveToTrash(new QCheckBox(settingsDialog))
        , buttonBox(new QDialogButtonBox(settingsDialog))
    {
        askBeforeDelete->setText(qApp->translate("SettingsDialog", "Ask before deleting images"));
        askBeforeDelete->setChecked(settings->askBeforeDelete());

        moveToTrash->setText(qApp->translate("SettingsDialog", "Move deleted images to trash"));
        moveToTrash->setChecked(settings->moveToTrash());

        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        QObject::connect(buttonBox, SIGNAL(rejected()), settingsDialog, SLOT(close()));
        QObject::connect(buttonBox, SIGNAL(accepted()), settingsDialog, SLOT(onSettingsAccepted()));

        QVBoxLayout *mainLayout = new QVBoxLayout(settingsDialog);
        mainLayout->addWidget(askBeforeDelete);
        mainLayout->addWidget(moveToTrash);
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
        settings->setAskBeforeDelete(askBeforeDelete->isChecked());
        settings->setMoveToTrash(moveToTrash->isChecked());
        settingsDialog->close();
    }

    SettingsDialog *settingsDialog;
    GUISettings *settings;

    QCheckBox *askBeforeDelete;
    QCheckBox *moveToTrash;
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
