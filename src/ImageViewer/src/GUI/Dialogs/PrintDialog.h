/*
   Copyright (C) 2021-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include "Utils/SharedPointer.h"

class QPrinterInfo;

class IImageData;

class PrintDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(PrintDialog)

public:
    PrintDialog(const QSharedPointer<IImageData> &imageData,
                int rotateAngle,
                const Qt::Orientations &flipOrientations,
                const QString &filePath,
                QWidget *parent = Q_NULLPTR);
    ~PrintDialog();

private Q_SLOTS:
    void onCurrentPrinterChanged(int index);
    void onPrintDialogButtonClicked();
    void onPrintClicked();
    void onPageSetupClicked();
    void onAutoRotateStateChanged();
    void onPortraitToggled(bool checked);
    void onLandscapeToggled(bool checked);
    void onNumCopiesChanged(int value);
    void onColorModeChanged(int index);
    void onGeometryChangeRequested(const QRectF &newGeometry);
    void onWidthChanged(double value);
    void onHeightChanged(double value);
    void onSizeUnitsChanged(int index);
    void onXResolutionChanged(double value);
    void onYResolutionChanged(double value);
    void onKeepAspectStateChanged();
    void onResolutionUnitsChanged(int index);
    void onLoadDefaultsClicked();
    void onLeftChanged(double value);
    void onRightChanged(double value);
    void onTopChanged(double value);
    void onBottomChanged(double value);
    void onCenterChanged(int index);
    void onIgnorePageMarginsStateChanged();
    void onIgnorePaperBoundsStateChanged();
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);
    void onExposureChanged(int value);
    void onGrayscaleToggled(bool checked);
    void onLegacyRendererToggled(bool checked);

private:
    void updatePrinterInfo(const QPrinterInfo& info);
    void updateColorMode(const QPrinterInfo& info);
    void updateNumCopies();
    void updatePageInfo();
    void updatePageOrientation();
    void updateImageGeometry();
    void updateEffects();

private:
    struct UI;
    QScopedPointer<UI> m_ui;
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // PRINT_DIALOG_H_INCLUDED
