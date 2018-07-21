/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cmath>
#include <algorithm>

#include <windows.h>
#include <mshtml.h>
#include <oleidl.h>

#include <QApplication>
#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QUrl>
#include <QByteArray>
#include <QVariant>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>
#include <QXmlStreamNamespaceDeclarations>

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItems/AbstractSVGWebBrowser.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/Scaling/IScaledImageProvider.h"
#if defined (HAS_ZLIB)
#include "Internal/Utils/ZLibUtils.h"
#endif

//#pragma comment(lib, "gdi32.lib")
//#pragma comment(lib, "ole32.lib") // CoInitializeEx, CoUninitialize, CoCreateInstance
//#pragma comment(lib, "oleaut32.lib") // SysAllocString, SysFreeString, SafeArrayDestroy, SafeArrayAccessData, SafeArrayCreateVector

namespace
{

// ====================================================================================================

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;

// ====================================================================================================

QImage QImageFromHBITMAP(HBITMAP bitmap)
{
    BITMAP bitmap_info;
    memset(&bitmap_info, 0, sizeof(BITMAP));
    if(!GetObject(bitmap, sizeof(BITMAP), &bitmap_info))
    {
        qWarning() << __FUNCTION__ << "Failed to get bitmap info";
        return QImage();
    }

    const int w = bitmap_info.bmWidth;
    const int h = bitmap_info.bmHeight;
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = -h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = w * h * 4;

    QImage result(w, h, QImage::Format_ARGB32_Premultiplied);
    HDC hdc = GetDC(0);
    if(!GetDIBits(hdc, bitmap, 0, h, result.bits(), &bmi, DIB_RGB_COLORS))
    {
        qWarning() << __FUNCTION__ << "Failed to get bitmap bits";
        result = QImage();
    }
    ReleaseDC(0, hdc);
    return result;
}

int compare(const QStringRef &s1, const QString &s2, Qt::CaseSensitivity cs)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    return s1.compare(s2, cs);
#else
    return s1.toString().compare(s2, cs);
#endif
}

// ====================================================================================================

class MSHTMLPixmapProvider : public IScaledImageProvider, public AbstractSVGWebBrowser
{
    Q_DISABLE_COPY(MSHTMLPixmapProvider)

public:
    MSHTMLPixmapProvider(const QString &filePath)
        : m_isValid(false)
        , m_htmlDocument2(NULL)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

        QByteArray inBuffer;
#if defined (HAS_ZLIB)
        if(QFileInfo(filePath).suffix().toLower() == QString::fromLatin1("svgz"))
        {
            inBuffer = ZLibUtils::InflateFile(filePath);
        }
        else
#endif
        {
            QFile inFile(filePath);
            if(!inFile.open(QIODevice::ReadOnly))
            {
                qWarning() << "Can't open" << filePath;
                return;
            }
            inBuffer = inFile.readAll();
        }

        if(inBuffer.isEmpty())
        {
            qWarning() << "Can't read" << filePath;
            return;
        }

        m_svgData = inBuffer;
        if(!rootElementIsSvg())
        {
            qWarning() << "Root element is not svg";
            return;
        }

        if(!prepareSVGData())
        {
            qWarning() << "Can't prepare svg data";
            return;
        }

        GUID CLSID_HTMLDocument;
        HRESULT hr = CLSIDFromProgID(L"htmlfile", &CLSID_HTMLDocument);
        if(!SUCCEEDED(hr))
        {
            qWarning() << "Can't get CLSID for mshtml" << QString::number(hr, 16);
            return;
        }

        hr = CoCreateInstance(CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER, IID_IHTMLDocument2, (void**)&m_htmlDocument2);
        if(!SUCCEEDED(hr))
        {
            qWarning() << "CoCreateInstance(CLSID_HTMLDocument) failed" << QString::number(hr, 16);
            return;
        }

#if defined (__IHTMLDocument3_INTERFACE_DEFINED__) /// @todo
        IHTMLDocument3 *htmlDocument3 = NULL;
        hr = m_htmlDocument2->QueryInterface(IID_IHTMLDocument3, (void**)&htmlDocument3);
        if(SUCCEEDED(hr))
        {
            const QString qstrBaseUrl = QUrl::fromLocalFile(QFileInfo(filePath).absolutePath()).toString();
            std::wstring wstrBaseUrl = qstrBaseUrl.toStdWString();
            BSTR bstrBaseUrl = SysAllocString(wstrBaseUrl.c_str());
            htmlDocument3->put_baseUrl(bstrBaseUrl);
            SysFreeString(bstrBaseUrl);
            htmlDocument3->Release();
        }
#endif

        if(!loadSVGData())
        {
            qWarning() << "Failed to load svg data";
            return;
        }

        m_svgRect = detectSvgRect();
        m_svgRect = QRectF(m_svgRect.topLeft(), m_svgRect.size().expandedTo(QSizeF(1, 1)));

        m_isValid = true;
        m_minScaleFactor = std::max(MIN_IMAGE_DIMENSION / m_svgRect.width(), MIN_IMAGE_DIMENSION / m_svgRect.height());
        m_maxScaleFactor = std::min(MAX_IMAGE_DIMENSION / m_svgRect.width(), MAX_IMAGE_DIMENSION / m_svgRect.height());
    }

    ~MSHTMLPixmapProvider()
    {
        if(m_htmlDocument2)
            m_htmlDocument2->Release();
        CoUninitialize();
    }

    bool isValid() const
    {
        return m_isValid;
    }

    bool requiresMainThread() const
    {
        return true;
    }

    QRectF boundingRect() const
    {
        return QRectF(0, 0, m_svgRect.width(), m_svgRect.height());
    }

    QImage image(const qreal scaleFactor)
    {
        if(!isValid())
            return QImage();
        const int width  = static_cast<int>(std::ceil(m_svgRect.width() * scaleFactor));
        const int height = static_cast<int>(std::ceil(m_svgRect.height() * scaleFactor));
        setDocumentViewport(width, height);

//        BSTR bgColorStr = SysAllocString(L"red");
//        VARIANT bgColor;
//        bgColor.vt = VT_BSTR;
//        bgColor.bstrVal = bgColorStr;
//        m_htmlDocument2->put_bgColor(bgColor);
//        SysFreeString(bgColorStr);

        return renderImage(width, height);
    }

    qreal minScaleFactor() const
    {
        return m_minScaleFactor;
    }

    qreal maxScaleFactor() const
    {
        return m_maxScaleFactor;
    }

protected:
    QSizeF svgSizeAttribute()
    {
        QXmlStreamReader reader(m_svgData);
        while(reader.readNext() != QXmlStreamReader::StartElement && !reader.atEnd());
        if(reader.atEnd())
            return QSizeF();
        const QXmlStreamAttributes attributes = reader.attributes();
        const QSizeF size = QSizeF(parseLength(attributes.value(QString::fromLatin1("width")).toString()),
                                   parseLength(attributes.value(QString::fromLatin1("height")).toString()));
        return size;
    }

    QRectF svgViewBoxAttribute()
    {
        QXmlStreamReader reader(m_svgData);
        while(reader.readNext() != QXmlStreamReader::StartElement && !reader.atEnd());
        if(reader.atEnd())
            return QRectF();
        const QStringList viewBoxData = reader.attributes().value(QString::fromLatin1("viewBox")).toString()
                .split(QRegExp(QString::fromLatin1("\\s")), QString::SkipEmptyParts);
        return (viewBoxData.size() == 4)
                ? QRectF(parseLength(viewBoxData.at(0)),
                         parseLength(viewBoxData.at(1)),
                         parseLength(viewBoxData.at(2)),
                         parseLength(viewBoxData.at(3)))
                : QRectF();
    }

    QRectF svgBoundingBoxRect()
    {
        return QRectF(); /// @todo
    }

    QRectF svgBoundingClientRect()
    {
        return QRectF(); /// @todo
    }

    bool rootElementIsSvg()
    {
        QXmlStreamReader reader(m_svgData);
        while(reader.readNext() != QXmlStreamReader::StartElement && !reader.atEnd());
        return !compare(reader.name(), QString::fromLatin1("svg"), Qt::CaseInsensitive);
    }

    QVariant evalJSImpl(const QString &scriptSource)
    {
        Q_UNUSED(scriptSource);
        qWarning() << __FUNCTION__ << "JavaScript is not available for MSHTML implementation";
        return QVariant();

//        HRESULT hr = E_FAIL;

//        IHTMLWindow2 *htmlWindow2 = NULL;
//        hr = m_htmlDocument2->get_parentWindow(&htmlWindow2);
//        if(!SUCCEEDED(hr))
//        {
//            qWarning() << "IHtmlDocument2::get_parentWindow() failed with error =" << QString::number(hr, 16);
//            return QVariant();
//        }

//        std::wstring wstrScriptSource = scriptSource.toStdWString();
//        BSTR bstrScriptSource = SysAllocString(wstrScriptSource.c_str());
//        BSTR bstrScriptLanguage = SysAllocString(L"javascript");

//        VARIANT result;
//        hr = htmlWindow2->execScript(bstrScriptSource, bstrScriptLanguage, &result);
//        SysFreeString(bstrScriptSource);
//        SysFreeString(bstrScriptLanguage);
//        htmlWindow2->Release();
//        if(!SUCCEEDED(hr))
//        {
//            qWarning() << "htmlWindow2::execScript() failed with error =" << QString::number(hr, 16);
//            return QVariant();
//        }

//        qDebug() << result.vt;
//        return QVariant();
    }

private:
    bool prepareSVGData()
    {
        const QRectF viewBox = svgViewBoxAttribute();
        const QSizeF size = svgSizeAttribute();
        const bool fakeViewBoxRequired = !size.isEmpty() && !viewBox.isValid();

        QByteArray preparedData;
        QXmlStreamWriter writer(&preparedData);
        /// @note Не выключать, иначе IE становится плохо от некоторых файлов!
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(-1);

        QXmlStreamReader reader(m_svgData);
        while(!reader.atEnd())
        {
            reader.readNext();

            if(reader.hasError())
            {
                qWarning() << __FUNCTION__ << reader.errorString();
                return false;
            }

            writer.writeCurrentToken(reader);
            if(reader.tokenType() == QXmlStreamReader::StartElement)
            {
                if(fakeViewBoxRequired && !compare(reader.name(), QString::fromLatin1("svg"), Qt::CaseInsensitive))
                {
                    writer.writeAttribute(QString::fromLatin1("viewBox"), QString::fromLatin1("%1 %2 %3 %4").arg(0).arg(0).arg(size.width()).arg(size.height()));
                    qDebug() << __FUNCTION__ << "Fake viewBox injected";
                }
            }
        }

        m_svgData = preparedData;
        return true;
    }

    bool loadSVGData()
    {
        const std::wstring wstrData = QString::fromLatin1(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
            "<html>\n"
            "<head>\n"
            "<title></title>\n"
            "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
            "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=9\">\n"
            "</head>\n"
            "<body style=\"padding: 0px; margin: 0px; overflow: hidden;\">\n"
            "<div>\n"
            "<img src=\"data:image/svg+xml;base64,%1\" alt="" width=\"100%\" height=\"100%\">\n"
            "</div>\n"
            "</body>\n"
            "</html>\n"
            ).arg(QString::fromLatin1(m_svgData.toBase64())).toStdWString();

        SAFEARRAY *safeArray = SafeArrayCreateVector(VT_VARIANT, 0, 1);
        VARIANT *param = NULL;
        HRESULT hr = SafeArrayAccessData(safeArray, (LPVOID*)&param);
        if(!SUCCEEDED(hr))
        {
            SafeArrayDestroy(safeArray);
            return false;
        }

        BSTR bstrData = SysAllocString(wstrData.c_str());
        param->vt = VT_BSTR;
        param->bstrVal = bstrData;
        hr = m_htmlDocument2->write(safeArray);
        SysFreeString(bstrData);
        SafeArrayDestroy(safeArray);
        if(!SUCCEEDED(hr))
            return false;

        hr = m_htmlDocument2->close();
        if(!SUCCEEDED(hr))
            return false;

        BSTR readyState;
        m_htmlDocument2->get_readyState(&readyState);
        while(std::wstring(readyState) == L"loading")
        {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
            SysFreeString(readyState);
            m_htmlDocument2->get_readyState(&readyState);
        }
        SysFreeString(readyState);
        return true;
    }

    bool setDocumentViewport(int width, int height)
    {
        const int HIMETRIC_INCH = 2540;

        HDC hdc = GetDC(0);
        const int x = MulDiv(width, HIMETRIC_INCH, GetDeviceCaps(hdc, LOGPIXELSX));
        const int y = MulDiv(height, HIMETRIC_INCH, GetDeviceCaps(hdc, LOGPIXELSY));
        ReleaseDC(0, hdc);

        IOleObject *oleObject = NULL;
        HRESULT hr = m_htmlDocument2->QueryInterface(IID_IOleObject, (void**)&oleObject);
        if(!SUCCEEDED(hr))
            return false;

        SIZEL size;
        memset(&size, 0, sizeof(SIZEL));
        size.cx = x;
        size.cy = y;
        hr = oleObject->SetExtent(DVASPECT_CONTENT, &size);
        oleObject->Release();
        return !!SUCCEEDED(hr);
    }

    QImage renderImage(int width, int height)
    {
        IViewObject *viewObject = NULL;
        if(!SUCCEEDED(m_htmlDocument2->QueryInterface(IID_IViewObject, (void**)&viewObject)))
            return QImage();

        RECTL clientRect;
        memset(&clientRect, 0, sizeof(RECTL));
        clientRect.right = width;
        clientRect.bottom = height;

        QImage image;
        HWND hwnd = 0;
        HDC hdc = GetDC(hwnd);
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmp = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(hdcMem, hbmp);
        HRESULT hr = viewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, hdcMem, &clientRect, NULL, NULL, 0);
        if(SUCCEEDED(hr))
            image = QImageFromHBITMAP(hbmp);
        DeleteObject(hbmp);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdc);
        viewObject->Release();

        return image;
    }

private:
    bool m_isValid;
    IHTMLDocument2 *m_htmlDocument2;
    QRectF m_svgRect;
    QByteArray m_svgData;
    qreal m_minScaleFactor;
    qreal m_maxScaleFactor;
};

// ====================================================================================================

class DecoderMSHTML : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderMSHTML");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("svg")
#if defined (HAS_ZLIB)
                << QString::fromLatin1("svgz")
#endif
                   ;
    }

    QStringList advancedFormats() const
    {
        return QStringList();
    }

    bool isAvailable() const
    {
        return true;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable() || !isAvailable())
            return QSharedPointer<IImageData>();
        return QSharedPointer<IImageData>(new ImageData(GraphicsItemsFactory::instance().createScalableItem(new MSHTMLPixmapProvider(filePath)), name()));
    }
};

DecoderAutoRegistrator registrator(new DecoderMSHTML);

// ====================================================================================================

} // namespace
