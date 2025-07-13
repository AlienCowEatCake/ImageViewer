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

#include "PrintDialog.h"
#include "PrintDialog_p.h"

#include <QFileInfo>
#include <QLocale>
#include <QMessageBox>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>

#include "Utils/SettingsWrapper.h"
#include "Utils/SignalBlocker.h"

#include "Decoders/IImageData.h"

namespace {

QPrinter::Unit unitFromVariant(const QVariant &variant, QPrinter::Unit defaultValue)
{
    if(!variant.isValid())
        return defaultValue;
    bool ok = false;
    int value = variant.toInt(&ok);
    if(!ok)
        return defaultValue;
    switch(value)
    {
#define ADD_CASE(X) case X: return X
    ADD_CASE(QPrinter::Millimeter);
    ADD_CASE(QPrinter::Point);
    ADD_CASE(QPrinter::Inch);
    ADD_CASE(QPrinter::Pica);
    ADD_CASE(QPrinter::Didot);
    ADD_CASE(QPrinter::Cicero);
#undef ADD_CASE
    default:
        break;
    }
    return defaultValue;
}

QVariant unitToVariant(QPrinter::Unit unit)
{
    return static_cast<int>(unit);
}

const QString SIZE_UNIT_KEY         = QString::fromLatin1("SizeUnit");
const QString RESOLUTION_UNIT_KEY   = QString::fromLatin1("ResolutionUnit");

class ScopedFullPageGuard
{
private:
    struct Margins
    {
        qreal left;
        qreal top;
        qreal right;
        qreal bottom;

        Margins()
            : left(0)
            , top(0)
            , right(0)
            , bottom(0)
        {}

        Margins(qreal value)
            : left(value)
            , top(value)
            , right(value)
            , bottom(value)
        {}

        Margins(qreal left, qreal top, qreal right, qreal bottom)
            : left(left)
            , top(top)
            , right(right)
            , bottom(bottom)
        {}
    };

public:
    ScopedFullPageGuard(QPrinter *printer, bool fullPage = true)
        : m_printer(fullPage ? printer : Q_NULLPTR)
        , m_wasFullPage(false)
    {
        scopeEnter();
    }

    ~ScopedFullPageGuard()
    {
        scopeLeave();
    }

private:
    void scopeEnter()
    {
        if(!m_printer)
            return;

        m_wasFullPage = m_printer->fullPage();
        m_wasMargins = getMargins();
        setMargins(Margins());
        m_printer->setFullPage(true);
        if(setMargins(Margins()))
            return;

        LOG_WARNING() << LOGGING_CTX << "Can't set zero page margins, trying to fix this";
        const QList<qreal> fixValues = QList<qreal>() << 0.01 << 0.1 << 1;
        for(QList<qreal>::ConstIterator it = fixValues.begin(); it != fixValues.end(); ++it)
            if(setMargins(Margins(*it)))
                return;

        LOG_WARNING() << LOGGING_CTX << "Can't set any minimal page margins, disabling full page mode";
        scopeLeave();
        m_printer = Q_NULLPTR;
    }

    void scopeLeave()
    {
        if(!m_printer)
            return;

        m_printer->setFullPage(m_wasFullPage);
        setMargins(m_wasMargins);
    }

    Margins getMargins() const
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
        const QMarginsF margins = m_printer->pageLayout().margins(QPageLayout::Millimeter);
        return Margins(margins.left(), margins.top(), margins.right(), margins.bottom());
#else
        Margins result;
        m_printer->getPageMargins(&result.left, &result.top, &result.right, &result.bottom, QPrinter::Millimeter);
        return result;
#endif
    }

    bool setMargins(const Margins &margins)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
        m_printer->setPageMargins(QMarginsF(margins.left, margins.top, margins.right, margins.bottom), QPageLayout::Millimeter);
#else
        m_printer->setPageMargins(margins.left, margins.top, margins.right, margins.bottom, QPrinter::Millimeter);
#endif
        const Margins effectiveMargins = getMargins();
        return true
                && isEqual(effectiveMargins.left, margins.left)
                && isEqual(effectiveMargins.top, margins.top)
                && isEqual(effectiveMargins.right, margins.right)
                && isEqual(effectiveMargins.bottom, margins.bottom)
                ;
    }

    static bool isEqual(qreal lhs, qreal rhs)
    {
        const bool lnull = qFuzzyIsNull(lhs);
        const bool rnull = qFuzzyIsNull(rhs);
        if(lnull && rnull)
            return true;
        if((!lnull && rnull) || (lnull && !rnull))
            return false;
        return qFuzzyCompare(lhs, rhs);
    }

private:
    QPrinter *m_printer;
    bool m_wasFullPage;
    Margins m_wasMargins;
};

} // namespace

struct PrintDialog::Impl
{
    SettingsWrapper settings;
    QList<QPrinterInfo> availablePrinters;
    const QSharedPointer<IImageData> imageData;
    QGraphicsItem * const graphicsItem;
    const int rotateAngle;
    const Qt::Orientations flipOrientations;
    const QString filePath;
    QScopedPointer<QPrinter> printer;
    QRectF itemPrintRect;
    PrintEffect printEffect;

    Impl(const QSharedPointer<IImageData> &imageData, int rotateAngle, const Qt::Orientations &flipOrientations, const QString &filePath)
        : settings(QString::fromLatin1("PrintDialogSettings"))
        , availablePrinters(QPrinterInfo::availablePrinters())
        , imageData(imageData)
        , graphicsItem(imageData ? imageData->graphicsItem() : Q_NULLPTR)
        , rotateAngle(rotateAngle)
        , flipOrientations(flipOrientations)
        , filePath(filePath)
    {}

    QRectF itemBounds() const
    {
        if(!graphicsItem)
            return QRectF();
        const QRectF boundingRect = graphicsItem->boundingRect().normalized();
        const QPointF center = boundingRect.center();
        const QTransform transform = QTransform().translate(center.x(), center.y()).rotate(rotateAngle).translate(-center.x(), -center.y());
        return transform.mapRect(boundingRect);
    }

    Qt::Orientation itemOrientation() const
    {
        if(!graphicsItem)
            return Qt::Vertical;
        const QRectF boundingRect = itemBounds();
        return boundingRect.height() >= boundingRect.width() ? Qt::Vertical : Qt::Horizontal;
    }

    QSizeF itemSize(QPrinter::Unit unit) const
    {
        if(!graphicsItem || !imageData)
            return QSizeF();
        const QSizeF bounds = itemBounds().size();
        const QPair<qreal, qreal> dpi = imageData->dpi();
        return QSizeF(convert(bounds.width() / dpi.first, QPrinter::Inch, unit), convert(bounds.height() / dpi.second, QPrinter::Inch, unit));
    }

    QSizeF availableItemSize(QPrinter::Unit unit, bool ignoreMargins, bool keepAspect) const
    {
        if(!printer)
            return QSizeF();
        const QRectF paperRect = printerPaperRect(unit);
        const QRectF pageRect = printerPageRect(unit);
        const QRectF availableRect = ignoreMargins ? paperRect : pageRect;
        QSizeF availableSize = availableRect.size();
        if(keepAspect)
        {
            QSizeF fixedAvailableSize = itemSize(unit);
            fixedAvailableSize.scale(availableSize, Qt::KeepAspectRatio);
            availableSize = fixedAvailableSize;
        }
        return availableSize;
    }

    QSizeF preferredItemSize(QPrinter::Unit unit, bool ignoreMargins, bool ignoreBounds) const
    {
        const QSizeF availableSize = availableItemSize(unit, ignoreMargins, true);
        const QSizeF originalSize = itemSize(unit);
        if(ignoreBounds || (originalSize.width() <= availableSize.width() && originalSize.height() <= availableSize.height()))
            return originalSize;
        return availableSize;
    }

    QRectF printerPaperRect(QPrinter::Unit unit) const
    {
        if(!printer)
            return QRectF();
        return fixOrientation(printer->paperRect(unit), unit);
    }

    QRectF printerPageRect(QPrinter::Unit unit) const
    {
        if(!printer)
            return QRectF();
        return fixOrientation(printer->pageRect(unit), unit);
    }

    bool hasValidPage() const
    {
        return !printerPaperRect(QPrinter::DevicePixel).isEmpty() && !printerPageRect(QPrinter::DevicePixel).isEmpty();
    }

    double convert(double value, QPrinter::Unit from, QPrinter::Unit to) const
    {
        const int resolution = printer ? printer->resolution() : 1;
        return value * multiplierForUnit(from, resolution) / multiplierForUnit(to, resolution);
    }

    QPointF convert(const QPointF &value, QPrinter::Unit from, QPrinter::Unit to) const
    {
        return QPointF(convert(value.x(), from, to), convert(value.y(), from, to));
    }

    QSizeF convert(const QSizeF &value, QPrinter::Unit from, QPrinter::Unit to) const
    {
        return QSizeF(convert(value.width(), from, to), convert(value.height(), from, to));
    }

    QRectF convert(const QRectF &value, QPrinter::Unit from, QPrinter::Unit to) const
    {
        return QRectF(convert(value.topLeft(), from, to), convert(value.size(), from, to));
    }

private:
    QRectF fixOrientation(const QRectF &rect, QPrinter::Unit unit) const
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
        const bool isPortrait = printer->pageLayout().orientation() == QPageLayout::Portrait;
#else
        const bool isPortrait = printer->orientation() == QPrinter::Portrait;
#endif
        const QRectF paperRectPoint = printer->paperRect(QPrinter::Point);
        const bool isPaperPortrait = paperRectPoint.height() >= paperRectPoint.width();
        if((isPortrait && isPaperPortrait) || (!isPortrait && !isPaperPortrait) || qFuzzyCompare(paperRectPoint.height(), paperRectPoint.width()))
            return rect;
        LOG_WARNING() << LOGGING_CTX << "Invalid paper orientation, x =" << paperRectPoint.x() << "y =" << paperRectPoint.y() << "width =" << paperRectPoint.width() << "height =" << paperRectPoint.height()
                   << "expected =" << (isPortrait ? "portrait," : "landscape,") << "trying to fix this";
        const QRectF paperRect = printer->paperRect(unit);
        const qreal ml = rect.left() - paperRect.left();
        const qreal mr = paperRect.right() - rect.right();
        const qreal mt = rect.top() - paperRect.top();
        const qreal mb = paperRect.bottom() - rect.bottom();
        const qreal sizeCorrection = ml + mr - mt - mb;
        return QRectF(ml, mt, rect.height() - sizeCorrection, rect.width() + sizeCorrection);
    }

    static double multiplierForUnit(QPrinter::Unit unit, int resolution)
    {
        switch(unit)
        {
        case QPrinter::Millimeter:
            return 2.83464566929;
        case QPrinter::Point:
            return 1.0;
        case QPrinter::Inch:
            return 72.0;
        case QPrinter::Pica:
            return 12;
        case QPrinter::Didot:
            return 1.065826771;
        case QPrinter::Cicero:
            return 12.789921252;
        case QPrinter::DevicePixel:
            return 72.0/resolution;
        }
        return 1.0;
    }
};

PrintDialog::PrintDialog(const QSharedPointer<IImageData> &imageData,
                         int rotateAngle,
                         const Qt::Orientations &flipOrientations,
                         const QString &filePath,
                         QWidget *parent)
    : QDialog(parent)
    , m_ui(new UI(this))
    , m_impl(new Impl(imageData, rotateAngle, flipOrientations, filePath))
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                   Qt::WindowCloseButtonHint |
#endif
                   Qt::WindowSystemMenuHint | Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle(qApp->translate("PrintDialog", "Print", "Title"));
    setWindowModality(Qt::ApplicationModal);

    m_ui->copiesSpinBox->setMinimum(1);
    m_ui->copiesSpinBox->setMaximum(999);

    m_ui->sizeUnitsComboBox->addItem(qApp->translate("PrintDialog", "Millimeters (mm)"  , "Size unit"), static_cast<int>(QPrinter::Millimeter));
    m_ui->sizeUnitsComboBox->addItem(qApp->translate("PrintDialog", "Points (pt)"       , "Size unit"), static_cast<int>(QPrinter::Point));
    m_ui->sizeUnitsComboBox->addItem(qApp->translate("PrintDialog", "Inches (in)"       , "Size unit"), static_cast<int>(QPrinter::Inch));
    m_ui->sizeUnitsComboBox->addItem(qApp->translate("PrintDialog", "Pica (P\xcc\xb8)"  , "Size unit"), static_cast<int>(QPrinter::Pica));
    m_ui->sizeUnitsComboBox->addItem(qApp->translate("PrintDialog", "Didot (DD)"        , "Size unit"), static_cast<int>(QPrinter::Didot));
    m_ui->sizeUnitsComboBox->addItem(qApp->translate("PrintDialog", "Cicero (CC)"       , "Size unit"), static_cast<int>(QPrinter::Cicero));
    QPrinter::Unit sizeUnit = QPrinter::Millimeter;
    switch(QLocale::system().measurementSystem())
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    case QLocale::ImperialUSSystem:
    case QLocale::ImperialUKSystem:
#else
    case QLocale::ImperialSystem:
#endif
        sizeUnit = QPrinter::Inch;
        break;
    default:
        sizeUnit = QPrinter::Millimeter;
        break;
    }
    sizeUnit = unitFromVariant(m_impl->settings.value(SIZE_UNIT_KEY, unitToVariant(sizeUnit)), sizeUnit);
    m_ui->sizeUnitsComboBox->setCurrentIndex(m_ui->sizeUnitsComboBox->findData(static_cast<int>(sizeUnit)));

    m_ui->resolutionUnitsComboBox->addItem(qApp->translate("PrintDialog", "Pixels/Millimeter", "Resolution unit"), static_cast<int>(QPrinter::Millimeter));
    m_ui->resolutionUnitsComboBox->addItem(qApp->translate("PrintDialog", "Pixels/Point"     , "Resolution unit"), static_cast<int>(QPrinter::Point));
    m_ui->resolutionUnitsComboBox->addItem(qApp->translate("PrintDialog", "Pixels/Inch"      , "Resolution unit"), static_cast<int>(QPrinter::Inch));
    m_ui->resolutionUnitsComboBox->addItem(qApp->translate("PrintDialog", "Pixels/Pica"      , "Resolution unit"), static_cast<int>(QPrinter::Pica));
    m_ui->resolutionUnitsComboBox->addItem(qApp->translate("PrintDialog", "Pixels/Didot"     , "Resolution unit"), static_cast<int>(QPrinter::Didot));
    m_ui->resolutionUnitsComboBox->addItem(qApp->translate("PrintDialog", "Pixels/Cicero"    , "Resolution unit"), static_cast<int>(QPrinter::Cicero));
    QPrinter::Unit resolutionUnit = QPrinter::Inch;
    resolutionUnit = unitFromVariant(m_impl->settings.value(RESOLUTION_UNIT_KEY, unitToVariant(resolutionUnit)), resolutionUnit);
    m_ui->resolutionUnitsComboBox->setCurrentIndex(m_ui->resolutionUnitsComboBox->findData(static_cast<int>(resolutionUnit)));

    m_ui->centerComboBox->addItem(qApp->translate("PrintDialog", "None"        , "Centering option"), static_cast<int>(0));
    m_ui->centerComboBox->addItem(qApp->translate("PrintDialog", "Horizontally", "Centering option"), static_cast<int>(Qt::Horizontal));
    m_ui->centerComboBox->addItem(qApp->translate("PrintDialog", "Vertically"  , "Centering option"), static_cast<int>(Qt::Vertical));
    m_ui->centerComboBox->addItem(qApp->translate("PrintDialog", "Both"        , "Centering option"), static_cast<int>(Qt::Horizontal | Qt::Vertical));
    m_ui->centerComboBox->setCurrentIndex(m_ui->centerComboBox->findData(static_cast<int>(Qt::Horizontal | Qt::Vertical)));

    m_ui->previewWidget->setGraphicsItem(m_impl->graphicsItem, rotateAngle, flipOrientations);
    m_ui->keepAspectCheckBox->setChecked(true);
    m_ui->ignorePageMarginsCheckBox->setChecked(false);
    m_ui->ignorePaperBoundsCheckBox->setChecked(false);

    m_ui->brightnessSlider->setMaximum(100);
    m_ui->brightnessSlider->setMinimum(-m_ui->brightnessSlider->maximum());
    m_ui->brightnessSlider->setValue(0);
    m_ui->brightnessSpinBox->setMaximum(m_ui->brightnessSlider->maximum());
    m_ui->brightnessSpinBox->setMinimum(m_ui->brightnessSlider->minimum());
    m_ui->brightnessSpinBox->setValue(m_ui->brightnessSlider->value());

    m_ui->contrastSlider->setMaximum(100);
    m_ui->contrastSlider->setMinimum(-m_ui->contrastSlider->maximum());
    m_ui->contrastSlider->setValue(0);
    m_ui->contrastSpinBox->setMaximum(m_ui->contrastSlider->maximum());
    m_ui->contrastSpinBox->setMinimum(m_ui->contrastSlider->minimum());
    m_ui->contrastSpinBox->setValue(m_ui->contrastSlider->value());

    m_ui->exposureSlider->setMaximum(100);
    m_ui->exposureSlider->setMinimum(-m_ui->exposureSlider->maximum());
    m_ui->exposureSlider->setValue(0);
    m_ui->exposureSpinBox->setMaximum(m_ui->exposureSlider->maximum());
    m_ui->exposureSpinBox->setMinimum(m_ui->exposureSlider->minimum());
    m_ui->exposureSpinBox->setValue(m_ui->exposureSlider->value());

    m_ui->grayscaleCheckBox->setChecked(false);
    m_ui->legacyRendererCheckBox->setChecked(false);
    m_ui->effectsPreviewWidget->setGraphicsItem(m_impl->graphicsItem, rotateAngle, flipOrientations);

    m_ui->widthSpinBox->setDecimals(3);
    m_ui->heightSpinBox->setDecimals(3);
    m_ui->xResolutionSpinBox->setDecimals(3);
    m_ui->yResolutionSpinBox->setDecimals(3);
    m_ui->leftSpinBox->setDecimals(3);
    m_ui->rightSpinBox->setDecimals(3);
    m_ui->topSpinBox->setDecimals(3);
    m_ui->bottomSpinBox->setDecimals(3);

    for(QList<QPrinterInfo>::ConstIterator it = m_impl->availablePrinters.begin(); it != m_impl->availablePrinters.end(); ++it)
    {
        m_ui->printerSelectComboBox->addItem(it->printerName());
        if(it->isDefault())
            m_ui->printerSelectComboBox->setCurrentIndex(m_ui->printerSelectComboBox->count() - 1);
    }
    onCurrentPrinterChanged(m_ui->printerSelectComboBox->currentIndex());

    m_ui->autoRotateCheckBox->setChecked(true);
    onAutoRotateStateChanged();

    connect(m_ui->printerSelectComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentPrinterChanged(int)));
    connect(m_ui->printDialogButton, SIGNAL(clicked()), this, SLOT(onPrintDialogButtonClicked()));
    connect(m_ui->pageSetupButton, SIGNAL(clicked()), this, SLOT(onPageSetupClicked()));
    connect(m_ui->autoRotateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onAutoRotateStateChanged()));
    connect(m_ui->portraitRadioButton, SIGNAL(toggled(bool)), this, SLOT(onPortraitToggled(bool)));
    connect(m_ui->landscapeRadioButton, SIGNAL(toggled(bool)), this, SLOT(onLandscapeToggled(bool)));
    connect(m_ui->copiesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onNumCopiesChanged(int)));
    connect(m_ui->colorModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onColorModeChanged(int)));
    connect(m_ui->previewWidget, SIGNAL(geometryChangeRequested(QRectF)), this, SLOT(onGeometryChangeRequested(QRectF)));
    connect(m_ui->widthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onWidthChanged(double)));
    connect(m_ui->heightSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onHeightChanged(double)));
    connect(m_ui->sizeUnitsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSizeUnitsChanged(int)));
    connect(m_ui->xResolutionSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onXResolutionChanged(double)));
    connect(m_ui->yResolutionSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onYResolutionChanged(double)));
    connect(m_ui->keepAspectCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onKeepAspectStateChanged()));
    connect(m_ui->resolutionUnitsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionUnitsChanged(int)));
    connect(m_ui->loadDefaultsButton, SIGNAL(clicked()), this, SLOT(onLoadDefaultsClicked()));
    connect(m_ui->leftSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onLeftChanged(double)));
    connect(m_ui->rightSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onRightChanged(double)));
    connect(m_ui->topSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onTopChanged(double)));
    connect(m_ui->bottomSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBottomChanged(double)));
    connect(m_ui->centerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCenterChanged(int)));
    connect(m_ui->ignorePageMarginsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onIgnorePageMarginsStateChanged()));
    connect(m_ui->ignorePaperBoundsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onIgnorePaperBoundsStateChanged()));
    connect(m_ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(onBrightnessChanged(int)));
    connect(m_ui->brightnessSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onBrightnessChanged(int)));
    connect(m_ui->contrastSlider, SIGNAL(valueChanged(int)), this, SLOT(onContrastChanged(int)));
    connect(m_ui->contrastSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onContrastChanged(int)));
    connect(m_ui->exposureSlider, SIGNAL(valueChanged(int)), this, SLOT(onExposureChanged(int)));
    connect(m_ui->exposureSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onExposureChanged(int)));
    connect(m_ui->grayscaleCheckBox, SIGNAL(toggled(bool)), this, SLOT(onGrayscaleToggled(bool)));
    connect(m_ui->legacyRendererCheckBox, SIGNAL(toggled(bool)), this, SLOT(onLegacyRendererToggled(bool)));
    connect(m_ui->dialogButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(onPrintClicked()));
    connect(m_ui->dialogButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));

    ensurePolished();
    adjustSize();
    setFixedSize(minimumSize());
}

PrintDialog::~PrintDialog()
{}

void PrintDialog::onCurrentPrinterChanged(int index)
{
    m_impl->printer.reset();
    updatePrinterInfo(QPrinterInfo());
    updatePageInfo();
    m_ui->printDialogButton->setEnabled(false);
    m_ui->miscGroup->setEnabled(false);
    m_ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if(index < 0 || index >= m_impl->availablePrinters.size())
        return;

    const QPrinterInfo& info = m_impl->availablePrinters[index];
    m_impl->printer.reset(new QPrinter(info, QPrinter::HighResolution));
    QPageSetupDialog(m_impl->printer.data(), Q_NULLPTR).accept();
    QPrintDialog(m_impl->printer.data(), Q_NULLPTR).accept();

    m_impl->printer->setDocName(QFileInfo(m_impl->filePath).fileName());
    m_impl->printer->setCreator(qApp->applicationName() + QString::fromLatin1(" ") + qApp->applicationVersion());

    updateNumCopies();
    updateColorMode(info);
    updatePrinterInfo(info);
    updatePageInfo();
    m_ui->printDialogButton->setEnabled(true);
    m_ui->miscGroup->setEnabled(true);
    m_ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(m_impl->graphicsItem && m_impl->hasValidPage());
}

void PrintDialog::onPrintDialogButtonClicked()
{
    if(!m_impl->printer)
        return;

    QPrintDialog *dialog = new QPrintDialog(m_impl->printer.data(), this);
    dialog->setMinMax(1, 1);
    dialog->setFromTo(1, 1);
    dialog->setPrintRange(QPrintDialog::AllPages);
    const QPrintDialog::PrintDialogOptions options = QPrintDialog::PrintToFile | QPrintDialog::PrintShowPageSize;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    dialog->setOptions(options);
#else
    dialog->setEnabledOptions(options | QPrintDialog::DontUseSheet);
#endif
    if(dialog->exec() == QDialog::Accepted)
    {
        dialog->hide();
        QPrinterInfo info;
        const int currentIndex = m_ui->printerSelectComboBox->currentIndex();
        if(currentIndex >= 0 && currentIndex < m_impl->availablePrinters.size())
            info = m_impl->availablePrinters[currentIndex];
        const QString printerName = m_impl->printer->printerName();
        if(printerName != info.printerName())
        {
            int newIndex = -1;
            const QSignalBlocker guard(m_ui->printerSelectComboBox);
            for(int i = 0; i < m_impl->availablePrinters.size(); ++i)
            {
                if(m_impl->availablePrinters[i].printerName() != printerName)
                    continue;
                newIndex = i;
                break;
            }
            if(newIndex < 0)
            {
                QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
                for(QList<QPrinterInfo>::ConstIterator it = printers.begin(); it != printers.end(); ++it)
                {
                    if(it->printerName() != printerName)
                        continue;
                    newIndex = it - printers.begin();
                    break;
                }
                if(newIndex >= 0)
                {
                    m_impl->availablePrinters = printers;
                    m_ui->printerSelectComboBox->clear();
                    for(QList<QPrinterInfo>::ConstIterator it = m_impl->availablePrinters.begin(); it != m_impl->availablePrinters.end(); ++it)
                        m_ui->printerSelectComboBox->addItem(it->printerName());
                }
            }
            if(newIndex >= 0)
            {
                info = m_impl->availablePrinters[newIndex];
                m_ui->printerSelectComboBox->setCurrentIndex(newIndex);
            }
            else
            {
                LOG_WARNING() << LOGGING_CTX << "Can't find info for printer" << printerName;
                info = QPrinterInfo();
                m_ui->printerSelectComboBox->setCurrentIndex(-1);
            }
        }
        updateNumCopies();
        updateColorMode(info);
        updatePrinterInfo(info);
        updatePageInfo();
        if(!m_impl->hasValidPage())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid page detected";
            const QString title = qApp->translate("PrintDialog", "Error");
            const QString text = qApp->translate("PrintDialog", "Invalid Paper Size");
            QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical, title, text, QMessageBox::Ok, this);
            msgBox->exec();
            msgBox->deleteLater();
        }
        m_ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(m_impl->graphicsItem && m_impl->hasValidPage());
    }
    dialog->hide();
    dialog->deleteLater();
}

void PrintDialog::onPrintClicked()
{
    if(!m_impl->graphicsItem || !m_impl->printer || !m_impl->hasValidPage())
        return;

    const ScopedFullPageGuard fullPageGuard(m_impl->printer.data(), m_ui->ignorePageMarginsCheckBox->isChecked() || m_ui->ignorePaperBoundsCheckBox->isChecked());
    QPainter painter(m_impl->printer.data());
    if(!painter.isActive())
        return;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QRectF itemRect = m_impl->convert(m_impl->itemPrintRect, QPrinter::Point, QPrinter::DevicePixel);
    itemRect.moveTopLeft(itemRect.topLeft() - m_impl->convert(m_impl->printerPageRect(QPrinter::Point), QPrinter::Point, QPrinter::DevicePixel).topLeft());
    const QRectF boundingRect = m_impl->graphicsItem->boundingRect();
    const QRectF rotatedBoundingRect = QTransform()
            .translate(boundingRect.center().x(), boundingRect.center().y())
            .rotate(m_impl->rotateAngle)
            .translate(-boundingRect.center().x(), -boundingRect.center().y())
            .mapRect(boundingRect);
    painter.translate(itemRect.x(), itemRect.y());
    painter.scale(itemRect.width() / rotatedBoundingRect.width(), itemRect.height() / rotatedBoundingRect.height());
    painter.translate(-rotatedBoundingRect.x(), -rotatedBoundingRect.y());
    painter.translate(rotatedBoundingRect.center().x(), rotatedBoundingRect.center().y());
    painter.rotate(m_impl->rotateAngle);
    painter.translate(-rotatedBoundingRect.center().x(), -rotatedBoundingRect.center().y());
    if(!m_impl->printEffect.legacyRenderer())
    {
        const QTransform worldTransform = painter.worldTransform();
        const QRect deviceRect = worldTransform.mapRect(boundingRect).toAlignedRect();
        Qt::TransformationMode transformationMode = Qt::SmoothTransformation;
        if(QGraphicsPixmapItem *pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(m_impl->graphicsItem))
            transformationMode = pixmapItem->transformationMode();
        if(ITransformationMode *itemWithTransformationMode = dynamic_cast<ITransformationMode*>(m_impl->graphicsItem))
            transformationMode = itemWithTransformationMode->transformationMode();
        const qreal scaleFactor = qMax(deviceRect.width() / rotatedBoundingRect.width(), deviceRect.height() / rotatedBoundingRect.height());
        bool applyEffectsBeforeTransform = true;
        QImage image;
        if(IGrabScaledImage *itemWithGrabScaledImage = dynamic_cast<IGrabScaledImage*>(m_impl->graphicsItem))
        {
            image = itemWithGrabScaledImage->grabImage(qMax(scaleFactor, static_cast<qreal>(1.0)));
            applyEffectsBeforeTransform = scaleFactor >= 1.0;
        }
        else if(IGrabImage *itemWithGrabImage = dynamic_cast<IGrabImage*>(m_impl->graphicsItem))
        {
            image = itemWithGrabImage->grabImage();
            applyEffectsBeforeTransform = scaleFactor >= 1.0;
        }
        else
        {
            const qreal scaleFactor = qMax(deviceRect.width() / rotatedBoundingRect.width(), deviceRect.height() / rotatedBoundingRect.height());
            QSize size = (boundingRect.size() * scaleFactor).toSize();
            image = QImage(size, QImage::Format_ARGB32_Premultiplied);
            while(image.isNull() && !size.isEmpty())
            {
                LOG_WARNING() << LOGGING_CTX << "Image rendering failed, target size =" << size.width() << "x" << size.height();
                size = QSize(size.width() / 2, size.height() / 2);
                image = QImage(size, QImage::Format_ARGB32_Premultiplied);
            }
            image.fill(Qt::transparent);
            QPainter painterLocal;
            painterLocal.begin(&image);
            painterLocal.scale(image.width() / boundingRect.width(), image.height() / boundingRect.height());
            painterLocal.translate(-boundingRect.x(), -boundingRect.y());
            painterLocal.setRenderHint(QPainter::Antialiasing, transformationMode == Qt::SmoothTransformation);
            painterLocal.setRenderHint(QPainter::TextAntialiasing, transformationMode == Qt::SmoothTransformation);
            painterLocal.setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
            QStyleOptionGraphicsItem options;
            options.exposedRect = boundingRect;
            m_impl->graphicsItem->paint(&painterLocal, &options);
            painterLocal.end();
        }
        if(applyEffectsBeforeTransform)
            m_impl->printEffect.apply(image);
        if(m_impl->flipOrientations)
            QImage_flip(image, m_impl->flipOrientations);
        QSize unrotatedDeviceSize = deviceRect.size();
        if(m_impl->rotateAngle)
        {
            QTransform transform;
            transform.rotate(-m_impl->rotateAngle);
            unrotatedDeviceSize = transform.mapRect(deviceRect).size();
        }
        if(transformationMode == Qt::SmoothTransformation && unrotatedDeviceSize.width() != image.width() && unrotatedDeviceSize.height() != image.height())
        {
            const QImage scaledImage = image.scaled(unrotatedDeviceSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            if(!scaledImage.isNull())
                image = scaledImage;
            else
                LOG_WARNING() << LOGGING_CTX << "Image scaling failed, target size =" << unrotatedDeviceSize.width() << "x" << unrotatedDeviceSize.height();
        }
        if(!applyEffectsBeforeTransform)
            m_impl->printEffect.apply(image);
        painter.drawImage(worldTransform.inverted().mapRect(deviceRect), image);
    }
    else
    {
        if(m_impl->flipOrientations & Qt::Horizontal)
        {
            painter.scale(-1, 1);
            painter.translate(-boundingRect.width(), 0);
        }
        if(m_impl->flipOrientations & Qt::Vertical)
        {
            painter.scale(1, -1);
            painter.translate(0, -boundingRect.height());
        }
        if(IGrabScaledImage *itemWithGrabScaledImage = dynamic_cast<IGrabScaledImage*>(m_impl->graphicsItem))
        {
            const QTransform worldTransform = painter.worldTransform();
            const QRect deviceRect = worldTransform.mapRect(boundingRect).toAlignedRect();
            const qreal scaleFactor = qMax(qMax(deviceRect.width() / rotatedBoundingRect.width(), deviceRect.height() / rotatedBoundingRect.height()), static_cast<qreal>(1.0));
            QImage image = itemWithGrabScaledImage->grabImage(scaleFactor);
            Qt::TransformationMode transformationMode = Qt::SmoothTransformation;
            if(QGraphicsPixmapItem *pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(m_impl->graphicsItem))
                transformationMode = pixmapItem->transformationMode();
            if(ITransformationMode *itemWithTransformationMode = dynamic_cast<ITransformationMode*>(m_impl->graphicsItem))
                transformationMode = itemWithTransformationMode->transformationMode();
            if(transformationMode == Qt::SmoothTransformation/* && deviceRect.width() < image.width() && deviceRect.height() < image.height()*/)
            {
                const QImage scaledImage = image.scaled(deviceRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                if(!scaledImage.isNull())
                    image = scaledImage;
                else
                    LOG_WARNING() << LOGGING_CTX << "Image scaling failed, target size =" << deviceRect.width() << "x" << deviceRect.height();
            }
            painter.drawImage(worldTransform.inverted().mapRect(deviceRect), image);
        }
        else if(IGrabImage *itemWithGrabImage = dynamic_cast<IGrabImage*>(m_impl->graphicsItem))
        {
            QImage image = itemWithGrabImage->grabImage();
            const QTransform worldTransform = painter.worldTransform();
            const QRect deviceRect = worldTransform.mapRect(boundingRect).toAlignedRect();
            Qt::TransformationMode transformationMode = Qt::SmoothTransformation;
            if(QGraphicsPixmapItem *pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(m_impl->graphicsItem))
                transformationMode = pixmapItem->transformationMode();
            if(ITransformationMode *itemWithTransformationMode = dynamic_cast<ITransformationMode*>(m_impl->graphicsItem))
                transformationMode = itemWithTransformationMode->transformationMode();
            if(transformationMode == Qt::SmoothTransformation/* && deviceRect.width() < image.width() && deviceRect.height() < image.height()*/)
            {
                const QImage scaledImage = image.scaled(deviceRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                if(!scaledImage.isNull())
                    image = scaledImage;
                else
                    LOG_WARNING() << LOGGING_CTX << "Image scaling failed, target size =" << deviceRect.width() << "x" << deviceRect.height();
            }
            painter.drawImage(worldTransform.inverted().mapRect(deviceRect), image);
        }
        else
        {
            QStyleOptionGraphicsItem options;
            options.exposedRect = boundingRect;
            m_impl->graphicsItem->paint(&painter, &options);
        }
    }
    painter.end();

    close();
}

void PrintDialog::onPageSetupClicked()
{
    if(!m_impl->printer)
        return;

    QPageSetupDialog *dialog = new QPageSetupDialog(m_impl->printer.data(), this);
    if(dialog->exec() == QDialog::Accepted)
    {
        updatePageInfo();
        if(!m_impl->hasValidPage())
        {
            LOG_WARNING() << LOGGING_CTX << "Invalid page detected";
            const QString title = qApp->translate("PrintDialog", "Error");
            const QString text = qApp->translate("PrintDialog", "Invalid Paper Size");
            QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical, title, text, QMessageBox::Ok, this);
            msgBox->exec();
            msgBox->deleteLater();
        }
        m_ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(m_impl->graphicsItem && m_impl->hasValidPage());
    }
    dialog->hide();
    dialog->deleteLater();
}

void PrintDialog::onAutoRotateStateChanged()
{
    const bool autoRotate = m_ui->autoRotateCheckBox->checkState() == Qt::Checked;
    m_ui->portraitRadioButton->setDisabled(autoRotate);
    m_ui->landscapeRadioButton->setDisabled(autoRotate);
    updatePageInfo();
}

void PrintDialog::onPortraitToggled(bool checked)
{
    if(!checked || !m_impl->printer)
        return;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    m_impl->printer->setPageOrientation(QPageLayout::Portrait);
#else
    m_impl->printer->setOrientation(QPrinter::Portrait);
#endif
    updatePageInfo();
}

void PrintDialog::onLandscapeToggled(bool checked)
{
    if(!checked || !m_impl->printer)
        return;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    m_impl->printer->setPageOrientation(QPageLayout::Landscape);
#else
    m_impl->printer->setOrientation(QPrinter::Landscape);
#endif
    updatePageInfo();
}

void PrintDialog::onNumCopiesChanged(int value)
{
    if(!m_impl->printer)
        return;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    m_impl->printer->setCopyCount(value);
#else
    m_impl->printer->setNumCopies(value);
#endif
    const QSignalBlocker guard(m_ui->copiesSpinBox);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    m_ui->copiesSpinBox->setValue(m_impl->printer->copyCount());
#else
    m_ui->copiesSpinBox->setValue(m_impl->printer->numCopies());
#endif
}

void PrintDialog::onColorModeChanged(int index)
{
    if(!m_impl->printer)
        return;
    m_impl->printer->setColorMode(static_cast<QPrinter::ColorMode>(m_ui->colorModeComboBox->itemData(index).toInt()));
    const QSignalBlocker guard(m_ui->colorModeComboBox);
    m_ui->colorModeComboBox->setCurrentIndex(m_ui->colorModeComboBox->findData(static_cast<int>(m_impl->printer->colorMode())));
}

void PrintDialog::onGeometryChangeRequested(const QRectF &newGeometry)
{
    if(!qFuzzyCompare(newGeometry.center().x(), m_impl->itemPrintRect.center().x()))
    {
        const QSignalBlocker guard(m_ui->centerComboBox);
        int centeringOrientation = m_ui->centerComboBox->itemData(m_ui->centerComboBox->currentIndex()).toInt();
        centeringOrientation &= ~Qt::Horizontal;
        m_ui->centerComboBox->setCurrentIndex(m_ui->centerComboBox->findData(centeringOrientation));
    }
    if(!qFuzzyCompare(newGeometry.center().y(), m_impl->itemPrintRect.center().y()))
    {
        const QSignalBlocker guard(m_ui->centerComboBox);
        int centeringOrientation = m_ui->centerComboBox->itemData(m_ui->centerComboBox->currentIndex()).toInt();
        centeringOrientation &= ~Qt::Vertical;
        m_ui->centerComboBox->setCurrentIndex(m_ui->centerComboBox->findData(centeringOrientation));
    }
    const bool keepAspect = m_ui->keepAspectCheckBox->isChecked();
    if(keepAspect && !qFuzzyCompare(newGeometry.width() / newGeometry.height(), m_impl->itemPrintRect.width() / m_impl->itemPrintRect.height()))
    {
        const QSignalBlocker guard(m_ui->keepAspectCheckBox);
        m_ui->keepAspectCheckBox->setChecked(false);
    }
    const bool ignoreMargins = m_ui->ignorePageMarginsCheckBox->isChecked();
    const bool ignoreBounds = m_ui->ignorePaperBoundsCheckBox->isChecked();
    const QRectF availableRect = ignoreMargins ? m_impl->printerPaperRect(QPrinter::Point) : m_impl->printerPageRect(QPrinter::Point);
    if(!ignoreBounds && availableRect.intersected(newGeometry) != newGeometry)
    {
        if(m_impl->itemPrintRect.size() == newGeometry.size())
        {
            QRectF fixedGeometry = newGeometry;
            fixedGeometry.moveBottom(qMin(newGeometry.bottom(), availableRect.bottom()));
            fixedGeometry.moveTop(qMax(newGeometry.top(), availableRect.top()));
            fixedGeometry.moveRight(qMin(newGeometry.right(), availableRect.right()));
            fixedGeometry.moveLeft(qMax(newGeometry.left(), availableRect.left()));
            m_impl->itemPrintRect = fixedGeometry;
        }
        else
        {
            const bool keepAspect = qFuzzyCompare(newGeometry.width() / newGeometry.height(), m_impl->itemPrintRect.width() / m_impl->itemPrintRect.height());
            QSizeF size = m_impl->itemPrintRect.size();
            size.scale(availableRect.intersected(newGeometry).size(), keepAspect ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
            QRectF fixedGeometry(0, 0, size.width(), size.height());
            if(qFuzzyCompare(m_impl->itemPrintRect.left(), newGeometry.left()))
            {
                if(qFuzzyCompare(m_impl->itemPrintRect.top(), newGeometry.top()))
                    fixedGeometry.moveTopLeft(newGeometry.topLeft());
                else
                    fixedGeometry.moveBottomLeft(newGeometry.bottomLeft());
            }
            else
            {
                if(qFuzzyCompare(m_impl->itemPrintRect.top(), newGeometry.top()))
                    fixedGeometry.moveTopRight(newGeometry.topRight());
                else
                    fixedGeometry.moveBottomRight(newGeometry.bottomRight());
            }
            m_impl->itemPrintRect = fixedGeometry;
        }
    }
    else
    {
        m_impl->itemPrintRect = newGeometry;
    }
    updateImageGeometry();
}

void PrintDialog::onWidthChanged(double value)
{
    const QPrinter::Unit sizeUnit = static_cast<QPrinter::Unit>(m_ui->sizeUnitsComboBox->itemData(m_ui->sizeUnitsComboBox->currentIndex()).toInt());
    const bool keepAspect = m_ui->keepAspectCheckBox->isChecked();
    m_impl->itemPrintRect.setWidth(m_impl->convert(value, sizeUnit, QPrinter::Point));
    if(keepAspect)
    {
        const QSizeF originalSize = m_impl->itemBounds().size();
        const qreal aspect = originalSize.width() / originalSize.height();
        m_impl->itemPrintRect.setHeight(m_impl->itemPrintRect.width() / aspect);
    }
    updateImageGeometry();
}

void PrintDialog::onHeightChanged(double value)
{
    const QPrinter::Unit sizeUnit = static_cast<QPrinter::Unit>(m_ui->sizeUnitsComboBox->itemData(m_ui->sizeUnitsComboBox->currentIndex()).toInt());
    const bool keepAspect = m_ui->keepAspectCheckBox->isChecked();
    m_impl->itemPrintRect.setHeight(m_impl->convert(value, sizeUnit, QPrinter::Point));
    if(keepAspect)
    {
        const QSizeF originalSize = m_impl->itemBounds().size();
        const qreal aspect = originalSize.width() / originalSize.height();
        m_impl->itemPrintRect.setWidth(m_impl->itemPrintRect.height() * aspect);
    }
    updateImageGeometry();
}

void PrintDialog::onSizeUnitsChanged(int index)
{
    updateImageGeometry();
    const QPrinter::Unit sizeUnit = static_cast<QPrinter::Unit>(m_ui->sizeUnitsComboBox->itemData(index).toInt());
    m_impl->settings.setValue(SIZE_UNIT_KEY, unitToVariant(sizeUnit));
}

void PrintDialog::onXResolutionChanged(double value)
{
    const QPrinter::Unit resolutionUnit = static_cast<QPrinter::Unit>(m_ui->resolutionUnitsComboBox->itemData(m_ui->resolutionUnitsComboBox->currentIndex()).toInt());
    const QSizeF originalSize = m_impl->itemBounds().size();
    const bool keepAspect = m_ui->keepAspectCheckBox->isChecked();
    m_impl->itemPrintRect.setWidth(m_impl->convert(originalSize.width() / value, resolutionUnit, QPrinter::Point));
    if(keepAspect)
    {
        const qreal aspect = originalSize.width() / originalSize.height();
        m_impl->itemPrintRect.setHeight(m_impl->itemPrintRect.width() / aspect);
    }
    updateImageGeometry();
}

void PrintDialog::onYResolutionChanged(double value)
{
    const QPrinter::Unit resolutionUnit = static_cast<QPrinter::Unit>(m_ui->resolutionUnitsComboBox->itemData(m_ui->resolutionUnitsComboBox->currentIndex()).toInt());
    const QSizeF originalSize = m_impl->itemBounds().size();
    const bool keepAspect = m_ui->keepAspectCheckBox->isChecked();
    m_impl->itemPrintRect.setHeight(m_impl->convert(originalSize.height() / value, resolutionUnit, QPrinter::Point));
    if(keepAspect)
    {
        const qreal aspect = originalSize.width() / originalSize.height();
        m_impl->itemPrintRect.setWidth(m_impl->itemPrintRect.height() * aspect);
    }
    updateImageGeometry();
}

void PrintDialog::onKeepAspectStateChanged()
{
    updateImageGeometry();
}

void PrintDialog::onResolutionUnitsChanged(int index)
{
    updateImageGeometry();
    const QPrinter::Unit resolutionUnit = static_cast<QPrinter::Unit>(m_ui->resolutionUnitsComboBox->itemData(index).toInt());
    m_impl->settings.setValue(RESOLUTION_UNIT_KEY, unitToVariant(resolutionUnit));
}

void PrintDialog::onLoadDefaultsClicked()
{
    m_impl->itemPrintRect = QRectF();
    updateImageGeometry();
}

void PrintDialog::onLeftChanged(double value)
{
    const QPrinter::Unit sizeUnit = static_cast<QPrinter::Unit>(m_ui->sizeUnitsComboBox->itemData(m_ui->sizeUnitsComboBox->currentIndex()).toInt());
    m_impl->itemPrintRect.moveLeft(m_impl->convert(value, sizeUnit, QPrinter::Point));
    int centeringOrientation = m_ui->centerComboBox->itemData(m_ui->centerComboBox->currentIndex()).toInt();
    if(centeringOrientation & Qt::Horizontal)
    {
        centeringOrientation &= ~Qt::Horizontal;
        const QSignalBlocker guard(m_ui->centerComboBox);
        m_ui->centerComboBox->setCurrentIndex(m_ui->centerComboBox->findData(centeringOrientation));
    }
    updateImageGeometry();
}

void PrintDialog::onRightChanged(double value)
{
    const QPrinter::Unit sizeUnit = static_cast<QPrinter::Unit>(m_ui->sizeUnitsComboBox->itemData(m_ui->sizeUnitsComboBox->currentIndex()).toInt());
    m_impl->itemPrintRect.moveRight(m_impl->convert(value, sizeUnit, QPrinter::Point));
    int centeringOrientation = m_ui->centerComboBox->itemData(m_ui->centerComboBox->currentIndex()).toInt();
    if(centeringOrientation & Qt::Horizontal)
    {
        centeringOrientation &= ~Qt::Horizontal;
        const QSignalBlocker guard(m_ui->centerComboBox);
        m_ui->centerComboBox->setCurrentIndex(m_ui->centerComboBox->findData(centeringOrientation));
    }
    updateImageGeometry();
}

void PrintDialog::onTopChanged(double value)
{
    const QPrinter::Unit sizeUnit = static_cast<QPrinter::Unit>(m_ui->sizeUnitsComboBox->itemData(m_ui->sizeUnitsComboBox->currentIndex()).toInt());
    m_impl->itemPrintRect.moveTop(m_impl->convert(value, sizeUnit, QPrinter::Point));
    int centeringOrientation = m_ui->centerComboBox->itemData(m_ui->centerComboBox->currentIndex()).toInt();
    if(centeringOrientation & Qt::Vertical)
    {
        centeringOrientation &= ~Qt::Vertical;
        const QSignalBlocker guard(m_ui->centerComboBox);
        m_ui->centerComboBox->setCurrentIndex(m_ui->centerComboBox->findData(centeringOrientation));
    }
    updateImageGeometry();
}

void PrintDialog::onBottomChanged(double value)
{
    const QPrinter::Unit sizeUnit = static_cast<QPrinter::Unit>(m_ui->sizeUnitsComboBox->itemData(m_ui->sizeUnitsComboBox->currentIndex()).toInt());
    m_impl->itemPrintRect.moveBottom(m_impl->convert(value, sizeUnit, QPrinter::Point));
    int centeringOrientation = m_ui->centerComboBox->itemData(m_ui->centerComboBox->currentIndex()).toInt();
    if(centeringOrientation & Qt::Vertical)
    {
        centeringOrientation &= ~Qt::Vertical;
        const QSignalBlocker guard(m_ui->centerComboBox);
        m_ui->centerComboBox->setCurrentIndex(m_ui->centerComboBox->findData(centeringOrientation));
    }
    updateImageGeometry();
}

void PrintDialog::onCenterChanged(int index)
{
    Q_UNUSED(index);
    updateImageGeometry();
}

void PrintDialog::onIgnorePageMarginsStateChanged()
{
    updateImageGeometry();
}

void PrintDialog::onIgnorePaperBoundsStateChanged()
{
    m_ui->ignorePageMarginsCheckBox->setEnabled(!m_ui->ignorePaperBoundsCheckBox->isChecked());
    updateImageGeometry();
}

void PrintDialog::onBrightnessChanged(int value)
{
    const QSignalBlocker sliderGuard(m_ui->brightnessSlider);
    m_ui->brightnessSlider->setValue(value);
    const QSignalBlocker spinBoxGuard(m_ui->brightnessSpinBox);
    m_ui->brightnessSpinBox->setValue(value);
    updateEffects();
}

void PrintDialog::onContrastChanged(int value)
{
    const QSignalBlocker sliderGuard(m_ui->contrastSlider);
    m_ui->contrastSlider->setValue(value);
    const QSignalBlocker spinBoxGuard(m_ui->contrastSpinBox);
    m_ui->contrastSpinBox->setValue(value);
    updateEffects();
}

void PrintDialog::onExposureChanged(int value)
{
    const QSignalBlocker sliderGuard(m_ui->exposureSlider);
    m_ui->exposureSlider->setValue(value);
    const QSignalBlocker spinBoxGuard(m_ui->exposureSpinBox);
    m_ui->exposureSpinBox->setValue(value);
    updateEffects();
}

void PrintDialog::onGrayscaleToggled(bool /*checked*/)
{
    updateEffects();
}

void PrintDialog::onLegacyRendererToggled(bool /*checked*/)
{
    updateEffects();
}

void PrintDialog::updatePrinterInfo(const QPrinterInfo& info)
{
    m_ui->printerNameLabel->setText(info.printerName());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_ui->printerDescriptionLabel->setText(info.description());
#else
    m_ui->printerDescriptionHeaderLabel->setEnabled(false);
    m_ui->printerDescriptionLabel->setEnabled(false);
#endif

    if(info.isDefault())
        m_ui->printerDefaultLabel->setText(qApp->translate("PrintDialog", "Yes", "Default"));
    else
        m_ui->printerDefaultLabel->setText(qApp->translate("PrintDialog", "No", "Default"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    if(info.isRemote())
        m_ui->printerRemoteLabel->setText(qApp->translate("PrintDialog", "Yes", "Remote"));
    else
        m_ui->printerRemoteLabel->setText(qApp->translate("PrintDialog", "No", "Remote"));
#else
    m_ui->printerRemoteHeaderLabel->setEnabled(false);
    m_ui->printerRemoteLabel->setEnabled(false);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_ui->printerLocationLabel->setText(info.location());
#else
    m_ui->printerLocationHeaderLabel->setEnabled(false);
    m_ui->printerLocationLabel->setEnabled(false);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_ui->printerMakeAndModelLabel->setText(info.makeAndModel());
#else
    m_ui->printerMakeAndModelHeaderLabel->setEnabled(false);
    m_ui->printerMakeAndModelLabel->setEnabled(false);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    switch(info.state())
    {
    case QPrinter::Idle:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Idle", "State"));
        break;
    case QPrinter::Active:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Active", "State"));
        break;
    case QPrinter::Aborted:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Aborted", "State"));
        break;
    case QPrinter::Error:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Error", "State"));
        break;
    default:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Unknown (%1)", "State").arg(static_cast<int>(info.state())));
        break;
    }
#else
    m_ui->printerStateHeaderLabel->setEnabled(false);
    m_ui->printerStateLabel->setEnabled(false);
#endif

    if(info.isNull())
    {
        m_ui->printerNameLabel->setText(QString());
        m_ui->printerDescriptionLabel->setText(QString());
        m_ui->printerDefaultLabel->setText(QString());
        m_ui->printerRemoteLabel->setText(QString());
        m_ui->printerLocationLabel->setText(QString());
        m_ui->printerMakeAndModelLabel->setText(QString());
        m_ui->printerStateLabel->setText(QString());
    }
}

void PrintDialog::updateColorMode(const QPrinterInfo &info)
{
    const QSignalBlocker guard(m_ui->colorModeComboBox);

    if(!m_impl->printer)
    {
        m_ui->colorModeComboBox->clear();
        m_ui->colorModeComboBox->setCurrentIndex(-1);
        return;
    }

    QList<QPrinter::ColorMode> supportedColorModes;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    if(!info.isNull())
        supportedColorModes = info.supportedColorModes();
    if(!supportedColorModes.contains(m_impl->printer->colorMode()))
#else
    Q_UNUSED(info);
    if(true)
#endif
        supportedColorModes = QList<QPrinter::ColorMode>() << QPrinter::Color << QPrinter::GrayScale;
    m_ui->colorModeComboBox->clear();
    if(supportedColorModes.contains(QPrinter::Color))
        m_ui->colorModeComboBox->addItem(qApp->translate("PrintDialog", "Color", "Color mode"), static_cast<int>(QPrinter::Color));
    if(supportedColorModes.contains(QPrinter::GrayScale))
        m_ui->colorModeComboBox->addItem(qApp->translate("PrintDialog", "Grayscale", "Color mode"), static_cast<int>(QPrinter::GrayScale));
    m_ui->colorModeComboBox->setCurrentIndex(m_ui->colorModeComboBox->findData(static_cast<int>(m_impl->printer->colorMode())));
}

void PrintDialog::updateNumCopies()
{
    const QSignalBlocker guard(m_ui->copiesSpinBox);

    if(!m_impl->printer)
    {
        m_ui->copiesSpinBox->setValue(1);
        m_ui->copiesSpinBox->setEnabled(false);
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    m_ui->copiesSpinBox->setValue(m_impl->printer->copyCount());
    m_ui->copiesSpinBox->setEnabled(m_impl->printer->supportsMultipleCopies());
#else
    m_ui->copiesSpinBox->setValue(m_impl->printer->numCopies());
    m_ui->copiesSpinBox->setEnabled(true);
#endif
}

void PrintDialog::updatePageInfo()
{
    m_impl->itemPrintRect = QRectF();
    updatePageOrientation();
    updateImageGeometry();
}

void PrintDialog::updatePageOrientation()
{
    const QSignalBlocker portraitRadioButtonBlocker(m_ui->portraitRadioButton);
    const QSignalBlocker landscapeRadioButtonBlocker(m_ui->landscapeRadioButton);

    if(!m_impl->printer || !m_impl->graphicsItem)
    {
        m_ui->pageGroup->setEnabled(false);
        m_ui->portraitRadioButton->setChecked(false);
        m_ui->landscapeRadioButton->setChecked(false);
        return;
    }

    m_ui->pageGroup->setEnabled(true);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    QPageLayout::Orientation orientation;
    const QPageLayout::Orientation portrait = QPageLayout::Portrait;
    const QPageLayout::Orientation landscape = QPageLayout::Landscape;
#else
    QPrinter::Orientation orientation;
    const QPrinter::Orientation portrait = QPrinter::Portrait;
    const QPrinter::Orientation landscape = QPrinter::Landscape;
#endif

    const bool autoRotate = m_ui->autoRotateCheckBox->checkState() == Qt::Checked;
    if(autoRotate)
    {
        orientation = (m_impl->itemOrientation() == Qt::Vertical) ? portrait : landscape;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
        m_impl->printer->setPageOrientation(orientation);
#else
        m_impl->printer->setOrientation(orientation);
#endif
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    orientation = m_impl->printer->pageLayout().orientation();
#else
    orientation = m_impl->printer->orientation();
#endif

    m_ui->portraitRadioButton->setChecked(orientation == portrait);
    m_ui->landscapeRadioButton->setChecked(orientation == landscape);
}

void PrintDialog::updateImageGeometry()
{
    const QSignalBlocker widthSpinBoxGuard(m_ui->widthSpinBox);
    const QSignalBlocker heightSpinBoxGuard(m_ui->heightSpinBox);
    const QSignalBlocker xResolutionSpinBoxGuard(m_ui->xResolutionSpinBox);
    const QSignalBlocker yResolutionSpinBoxGuard(m_ui->yResolutionSpinBox);
    const QSignalBlocker leftSpinBoxGuard(m_ui->leftSpinBox);
    const QSignalBlocker rightSpinBoxGuard(m_ui->rightSpinBox);
    const QSignalBlocker topSpinBoxGuard(m_ui->topSpinBox);
    const QSignalBlocker bottomSpinBoxGuard(m_ui->bottomSpinBox);

    if(!m_impl->printer || !m_impl->graphicsItem || !m_impl->hasValidPage())
    {
        m_ui->imageSettingsTabFrame->setEnabled(false);
        m_ui->effectsTabFrame->setEnabled(false);

        m_ui->widthSpinBox->setMinimum(0.0);
        m_ui->widthSpinBox->setMaximum(0.0);
        m_ui->widthSpinBox->setValue(0.0);
        m_ui->heightSpinBox->setMinimum(0.0);
        m_ui->heightSpinBox->setMaximum(0.0);
        m_ui->heightSpinBox->setValue(0.0);
        m_ui->leftSpinBox->setMinimum(0.0);
        m_ui->leftSpinBox->setMaximum(0.0);
        m_ui->leftSpinBox->setValue(0.0);
        m_ui->rightSpinBox->setMinimum(0.0);
        m_ui->rightSpinBox->setMaximum(0.0);
        m_ui->rightSpinBox->setValue(0.0);
        m_ui->topSpinBox->setMinimum(0.0);
        m_ui->topSpinBox->setMaximum(0.0);
        m_ui->topSpinBox->setValue(0.0);
        m_ui->bottomSpinBox->setMinimum(0.0);
        m_ui->bottomSpinBox->setMaximum(0.0);
        m_ui->bottomSpinBox->setValue(0.0);

        m_ui->xResolutionSpinBox->setMinimum(0.0);
        m_ui->xResolutionSpinBox->setMaximum(0.0);
        m_ui->xResolutionSpinBox->setValue(0.0);
        m_ui->yResolutionSpinBox->setMinimum(0.0);
        m_ui->yResolutionSpinBox->setMaximum(0.0);
        m_ui->yResolutionSpinBox->setValue(0.0);

        m_ui->previewWidget->setPaperRect(QRectF());
        m_ui->previewWidget->setPageRect(QRectF());
        m_ui->previewWidget->setItemRect(QRectF());

        m_ui->effectsPreviewWidget->setPaperRect(QRectF());
        m_ui->effectsPreviewWidget->setPageRect(QRectF());
        m_ui->effectsPreviewWidget->setItemRect(QRectF());
        return;
    }

    m_ui->imageSettingsTabFrame->setEnabled(true);
    m_ui->effectsTabFrame->setEnabled(true);

    const bool ignoreMargins = m_ui->ignorePageMarginsCheckBox->isChecked();
    const bool ignoreBounds = m_ui->ignorePaperBoundsCheckBox->isChecked();
    const bool keepAspect = m_ui->keepAspectCheckBox->isChecked();
    const int centering = m_ui->centerComboBox->itemData(m_ui->centerComboBox->currentIndex()).toInt();

    const QRectF paperRect = m_impl->printerPaperRect(QPrinter::Point);
    const QRectF pageRect = m_impl->printerPageRect(QPrinter::Point);
    const QSizeF preferredSize = m_impl->preferredItemSize(QPrinter::Point, ignoreMargins, ignoreBounds);
    QRectF availableRect = ignoreMargins ? paperRect : pageRect;
    if(ignoreBounds)
    {
        const qreal scale = 100;
        const QSizeF adjust = QSizeF(availableRect.width() * scale, availableRect.height() * scale);
        availableRect.adjust(-adjust.width(), -adjust.height(), adjust.width(), adjust.height());
    }

    if(m_impl->itemPrintRect.isEmpty())
    {
        m_impl->itemPrintRect = QRectF(QPointF(0, 0), preferredSize);
        const QPointF center = availableRect.center();
        const QPointF topLeft = availableRect.topLeft() + m_impl->itemPrintRect.center();
        QPointF targetCenter = topLeft;
        if(centering & Qt::Horizontal)
            targetCenter.setX(center.x());
        if(centering & Qt::Vertical)
            targetCenter.setY(center.y());
        m_impl->itemPrintRect.moveCenter(targetCenter);
    }

    if(!ignoreBounds && (m_impl->itemPrintRect.width() > availableRect.width() || m_impl->itemPrintRect.height() > availableRect.height()))
    {
        QSizeF targetSize = m_impl->itemPrintRect.size();
        targetSize.scale(availableRect.size(), keepAspect ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
        targetSize = QSizeF(qMin(targetSize.width(), m_impl->itemPrintRect.width()), qMin(targetSize.height(), m_impl->itemPrintRect.height()));
        const QSizeF delta = m_impl->itemPrintRect.size() - targetSize;
        QPointF correction;
        if(centering & Qt::Horizontal)
            correction.setX(delta.width() / 2);
        if(centering & Qt::Vertical)
            correction.setY(delta.height() / 2);
        m_impl->itemPrintRect = QRectF(m_impl->itemPrintRect.topLeft() + correction, targetSize);
    }

    if(keepAspect && !qFuzzyCompare(preferredSize.width() / preferredSize.height(), m_impl->itemPrintRect.width() / m_impl->itemPrintRect.height()))
    {
        QSizeF targetSize = preferredSize;
        targetSize.scale(m_impl->itemPrintRect.size(), Qt::KeepAspectRatio);
        const QSizeF delta = m_impl->itemPrintRect.size() - targetSize;
        QPointF correction;
        if(centering & Qt::Horizontal)
            correction.setX(delta.width() / 2);
        if(centering & Qt::Vertical)
            correction.setY(delta.height() / 2);
        m_impl->itemPrintRect = QRectF(m_impl->itemPrintRect.topLeft() + correction, targetSize);
    }

    if(centering & Qt::Horizontal && !qFuzzyCompare(m_impl->itemPrintRect.center().x(), availableRect.center().x()))
        m_impl->itemPrintRect.moveCenter(QPointF(availableRect.center().x(), m_impl->itemPrintRect.center().y()));
    if(centering & Qt::Vertical && !qFuzzyCompare(m_impl->itemPrintRect.center().y(), availableRect.center().y()))
        m_impl->itemPrintRect.moveCenter(QPointF(m_impl->itemPrintRect.center().x(), availableRect.center().y()));

    if(!ignoreBounds)
    {
        if(m_impl->itemPrintRect.right() > availableRect.right())
            m_impl->itemPrintRect.moveRight(availableRect.right());
        if(m_impl->itemPrintRect.bottom() > availableRect.bottom())
            m_impl->itemPrintRect.moveBottom(availableRect.bottom());
        if(m_impl->itemPrintRect.top() < availableRect.top())
            m_impl->itemPrintRect.moveTop(availableRect.top());
        if(m_impl->itemPrintRect.left() < availableRect.left())
            m_impl->itemPrintRect.moveLeft(availableRect.left());
    }

    const QPrinter::Unit sizeUnit = static_cast<QPrinter::Unit>(m_ui->sizeUnitsComboBox->itemData(m_ui->sizeUnitsComboBox->currentIndex()).toInt());
    m_ui->widthSpinBox->setMinimum(m_impl->convert(std::pow(10.0, static_cast<double>(-m_ui->widthSpinBox->decimals())), QPrinter::Point, sizeUnit));
    m_ui->widthSpinBox->setMaximum(m_impl->convert(availableRect.width(), QPrinter::Point, sizeUnit));
    m_ui->widthSpinBox->setValue(m_impl->convert(m_impl->itemPrintRect.width(), QPrinter::Point, sizeUnit));
    m_ui->heightSpinBox->setMinimum(m_impl->convert(std::pow(10.0, static_cast<double>(-m_ui->heightSpinBox->decimals())), QPrinter::Point, sizeUnit));
    m_ui->heightSpinBox->setMaximum(m_impl->convert(availableRect.height(), QPrinter::Point, sizeUnit));
    m_ui->heightSpinBox->setValue(m_impl->convert(m_impl->itemPrintRect.height(), QPrinter::Point, sizeUnit));
    m_ui->leftSpinBox->setMinimum(m_impl->convert(availableRect.left(), QPrinter::Point, sizeUnit));
    m_ui->leftSpinBox->setMaximum(m_impl->convert(availableRect.right() - m_impl->itemPrintRect.width(), QPrinter::Point, sizeUnit));
    m_ui->leftSpinBox->setValue(m_impl->convert(m_impl->itemPrintRect.left(), QPrinter::Point, sizeUnit));
    m_ui->rightSpinBox->setMinimum(m_impl->convert(availableRect.left() + m_impl->itemPrintRect.width(), QPrinter::Point, sizeUnit));
    m_ui->rightSpinBox->setMaximum(m_impl->convert(availableRect.right(), QPrinter::Point, sizeUnit));
    m_ui->rightSpinBox->setValue(m_impl->convert(m_impl->itemPrintRect.right(), QPrinter::Point, sizeUnit));
    m_ui->topSpinBox->setMinimum(m_impl->convert(availableRect.top(), QPrinter::Point, sizeUnit));
    m_ui->topSpinBox->setMaximum(m_impl->convert(availableRect.bottom() - m_impl->itemPrintRect.height(), QPrinter::Point, sizeUnit));
    m_ui->topSpinBox->setValue(m_impl->convert(m_impl->itemPrintRect.top(), QPrinter::Point, sizeUnit));
    m_ui->bottomSpinBox->setMinimum(m_impl->convert(availableRect.top() + m_impl->itemPrintRect.height(), QPrinter::Point, sizeUnit));
    m_ui->bottomSpinBox->setMaximum(m_impl->convert(availableRect.bottom(), QPrinter::Point, sizeUnit));
    m_ui->bottomSpinBox->setValue(m_impl->convert(m_impl->itemPrintRect.bottom(), QPrinter::Point, sizeUnit));

    const QPrinter::Unit resolutionUnit = static_cast<QPrinter::Unit>(m_ui->resolutionUnitsComboBox->itemData(m_ui->resolutionUnitsComboBox->currentIndex()).toInt());
    const QSizeF originalSize = m_impl->itemBounds().size();
    m_ui->xResolutionSpinBox->setMinimum(originalSize.width() / m_impl->convert(m_ui->widthSpinBox->maximum(), sizeUnit, resolutionUnit));
    m_ui->xResolutionSpinBox->setMaximum(originalSize.width() / m_impl->convert(m_ui->widthSpinBox->minimum(), sizeUnit, resolutionUnit));
    m_ui->xResolutionSpinBox->setValue(originalSize.width() / m_impl->convert(m_impl->itemPrintRect.width(), QPrinter::Point, resolutionUnit));
    m_ui->yResolutionSpinBox->setMinimum(originalSize.height() / m_impl->convert(m_ui->heightSpinBox->maximum(), sizeUnit, resolutionUnit));
    m_ui->yResolutionSpinBox->setMaximum(originalSize.height() / m_impl->convert(m_ui->heightSpinBox->minimum(), sizeUnit, resolutionUnit));
    m_ui->yResolutionSpinBox->setValue(originalSize.height() / m_impl->convert(m_impl->itemPrintRect.height(), QPrinter::Point, resolutionUnit));

    m_ui->previewWidget->setPaperRect(m_impl->printerPaperRect(QPrinter::Point));
    m_ui->previewWidget->setPageRect(m_impl->printerPageRect(QPrinter::Point));
    m_ui->previewWidget->setItemRect(m_impl->itemPrintRect);

    m_ui->effectsPreviewWidget->setPaperRect(m_impl->printerPaperRect(QPrinter::Point));
    m_ui->effectsPreviewWidget->setPageRect(m_impl->printerPageRect(QPrinter::Point));
    m_ui->effectsPreviewWidget->setItemRect(m_impl->itemPrintRect);
}

void PrintDialog::updateEffects()
{
    const int brightness = m_ui->brightnessSlider->value();
    m_ui->previewWidget->setBrightness(brightness);
    m_ui->effectsPreviewWidget->setBrightness(brightness);
    m_impl->printEffect.setBrightness(brightness);

    const int contrast = m_ui->contrastSlider->value();
    m_ui->previewWidget->setContrast(contrast);
    m_ui->effectsPreviewWidget->setContrast(contrast);
    m_impl->printEffect.setContrast(contrast);

    const int exposure = m_ui->exposureSlider->value();
    m_ui->previewWidget->setExposure(exposure);
    m_ui->effectsPreviewWidget->setExposure(exposure);
    m_impl->printEffect.setExposure(exposure);

    const bool grayscale = m_ui->grayscaleCheckBox->isChecked();
    m_ui->previewWidget->setGrayscale(grayscale);
    m_ui->effectsPreviewWidget->setGrayscale(grayscale);
    m_impl->printEffect.setGrayscale(grayscale);

    const bool legacyRenderer = m_ui->legacyRendererCheckBox->isChecked();
    m_ui->previewWidget->setLegacyRenderer(legacyRenderer);
    m_ui->effectsPreviewWidget->setLegacyRenderer(legacyRenderer);
    m_impl->printEffect.setLegacyRenderer(legacyRenderer);

    const bool effectsEnabled = !legacyRenderer;
    m_ui->brightnessLabel->setEnabled(effectsEnabled);
    m_ui->brightnessSlider->setEnabled(effectsEnabled);
    m_ui->brightnessSpinBox->setEnabled(effectsEnabled);
    m_ui->contrastLabel->setEnabled(effectsEnabled);
    m_ui->contrastSlider->setEnabled(effectsEnabled);
    m_ui->contrastSpinBox->setEnabled(effectsEnabled);
    m_ui->exposureLabel->setEnabled(effectsEnabled);
    m_ui->exposureSlider->setEnabled(effectsEnabled);
    m_ui->exposureSpinBox->setEnabled(effectsEnabled);
    m_ui->grayscaleCheckBox->setEnabled(effectsEnabled);
}
