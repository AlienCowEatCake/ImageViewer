/*
   Copyright (C) 2018-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QSysInfo>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>
#include <QXmlStreamNamespaceDeclarations>

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItems/AbstractSVGWebBrowserNoJS.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"
#include "Internal/Scaling/IScaledImageProvider.h"
#include "Internal/Utils/MappedBuffer.h"
#include "Internal/Utils/XmlStreamReader.h"

//#pragma comment(lib, "ole32.lib") // CoInitializeEx, CoUninitialize, CoCreateInstance
//#pragma comment(lib, "oleaut32.lib") // SysAllocString, SysFreeString, SafeArrayDestroy, SafeArrayAccessData, SafeArrayCreateVector
//#pragma comment(lib, "uuid.lib")

namespace
{

// ====================================================================================================

const qreal MAX_IMAGE_DIMENSION = 16384;
const qreal MIN_IMAGE_DIMENSION = 1;

// ====================================================================================================

class OLE32
{
public:
    static OLE32 *instance()
    {
        static OLE32 _;
        if(!_.isValid())
        {
            LOG_WARNING() << LOGGING_CTX << "Failed to load ole32.dll";
            return Q_NULLPTR;
        }
        return &_;
    }

    HRESULT CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit)
    {
        typedef HRESULT(WINAPI *CoInitializeEx_t)(LPVOID, DWORD);
        CoInitializeEx_t CoInitializeEx_f = (CoInitializeEx_t)m_CoInitializeEx;
        return CoInitializeEx_f(pvReserved, dwCoInit);
    }

    HRESULT CoUninitialize()
    {
        typedef HRESULT(WINAPI *CoUninitialize_t)();
        CoUninitialize_t CoUninitialize_f = (CoUninitialize_t)m_CoUninitialize;
        return CoUninitialize_f();
    }

    HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
    {
        typedef HRESULT(WINAPI *CoCreateInstance_t)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
        CoCreateInstance_t CoCreateInstance_f = (CoCreateInstance_t)m_CoCreateInstance;
        return CoCreateInstance_f(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }

    HRESULT CLSIDFromProgID(LPCOLESTR lpszProgID, LPCLSID lpclsid)
    {
        typedef HRESULT(WINAPI *CLSIDFromProgID_t)(LPCOLESTR, LPCLSID);
        CLSIDFromProgID_t CLSIDFromProgID_f = (CLSIDFromProgID_t)m_CLSIDFromProgID;
        return CLSIDFromProgID_f(lpszProgID, lpclsid);
    }

    HRESULT IIDFromString(LPCOLESTR lpsz, LPIID lpiid)
    {
        typedef HRESULT(WINAPI *IIDFromString_t)(LPCOLESTR, LPIID);
        IIDFromString_t IIDFromString_f = (IIDFromString_t)m_IIDFromString;
        return IIDFromString_f(lpsz, lpiid);
    }

    HRESULT CLSIDFromString(LPCOLESTR lpsz, LPCLSID pclsid)
    {
        typedef HRESULT(WINAPI *CLSIDFromString_t)(LPCOLESTR, LPCLSID);
        CLSIDFromString_t CLSIDFromString_f = (CLSIDFromString_t)m_CLSIDFromString;
        return CLSIDFromString_f(lpsz, pclsid);
    }

private:
    OLE32()
        : m_CoInitializeEx(Q_NULLPTR)
        , m_CoUninitialize(Q_NULLPTR)
        , m_CoCreateInstance(Q_NULLPTR)
        , m_CLSIDFromProgID(Q_NULLPTR)
        , m_IIDFromString(Q_NULLPTR)
        , m_CLSIDFromString(Q_NULLPTR)
    {
        HMODULE library = ::LoadLibraryA("ole32");
        if(!library)
            return;

        m_CoInitializeEx = ::GetProcAddress(library, "CoInitializeEx");
        m_CoUninitialize = ::GetProcAddress(library, "CoUninitialize");
        m_CoCreateInstance = ::GetProcAddress(library, "CoCreateInstance");
        m_CLSIDFromProgID = ::GetProcAddress(library, "CLSIDFromProgID");
        m_IIDFromString = ::GetProcAddress(library, "IIDFromString");
        m_CLSIDFromString = ::GetProcAddress(library, "CLSIDFromString");
    }

    ~OLE32()
    {}

    bool isValid() const
    {
        return m_CoInitializeEx && m_CoUninitialize && m_CoCreateInstance &&
                m_CLSIDFromProgID && m_IIDFromString && m_CLSIDFromString;
    }

    FARPROC m_CoInitializeEx;
    FARPROC m_CoUninitialize;
    FARPROC m_CoCreateInstance;
    FARPROC m_CLSIDFromProgID;
    FARPROC m_IIDFromString;
    FARPROC m_CLSIDFromString;
};

HRESULT CoInitializeEx_WRAP(LPVOID pvReserved, DWORD dwCoInit)
{
    if(OLE32 *ole32 = OLE32::instance())
        return ole32->CoInitializeEx(pvReserved, dwCoInit);
    LOG_WARNING() << LOGGING_CTX << "Failed to load ole32.dll";
    return E_FAIL;
}
#define CoInitializeEx CoInitializeEx_WRAP

HRESULT CoUninitialize_WRAP()
{
    if(OLE32 *ole32 = OLE32::instance())
        return ole32->CoUninitialize();
    LOG_WARNING() << LOGGING_CTX << "Failed to load ole32.dll";
    return E_FAIL;
}
#define CoUninitialize CoUninitialize_WRAP

HRESULT CoCreateInstance_WRAP(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
    if(OLE32 *ole32 = OLE32::instance())
        return ole32->CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    LOG_WARNING() << LOGGING_CTX << "Failed to load ole32.dll";
    return E_FAIL;
}
#define CoCreateInstance CoCreateInstance_WRAP

HRESULT CLSIDFromProgID_WRAP(LPCOLESTR lpszProgID, LPCLSID lpclsid)
{
    if(OLE32 *ole32 = OLE32::instance())
        return ole32->CLSIDFromProgID(lpszProgID, lpclsid);
    LOG_WARNING() << LOGGING_CTX << "Failed to load ole32.dll";
    return E_FAIL;
}
#define CLSIDFromProgID CLSIDFromProgID_WRAP

HRESULT IIDFromString_WRAP(LPCOLESTR lpsz, LPIID lpiid)
{
    if(OLE32 *ole32 = OLE32::instance())
        return ole32->IIDFromString(lpsz, lpiid);
    LOG_WARNING() << LOGGING_CTX << "Failed to load ole32.dll";
    return E_FAIL;
}
#define IIDFromString IIDFromString_WRAP

HRESULT CLSIDFromString_WRAP(LPCOLESTR lpsz, LPCLSID pclsid)
{
    if(OLE32 *ole32 = OLE32::instance())
        return ole32->CLSIDFromString(lpsz, pclsid);
    LOG_WARNING() << LOGGING_CTX << "Failed to load ole32.dll";
    return E_FAIL;
}
#define CLSIDFromString CLSIDFromString_WRAP

// ====================================================================================================

class OLEAut32
{
public:
    static OLEAut32 *instance()
    {
        static OLEAut32 _;
        if(!_.isValid())
        {
            LOG_WARNING() << LOGGING_CTX << "Failed to load oleaut32.dll";
            return Q_NULLPTR;
        }
        return &_;
    }

    BSTR SysAllocString(const OLECHAR *psz)
    {
        typedef BSTR(WINAPI *ft)(const OLECHAR*);
        ft fp = (ft)m_SysAllocString;
        return fp(psz);
    }

    void SysFreeString(BSTR bstrString)
    {
        typedef void(WINAPI *ft)(const BSTR);
        ft fp = (ft)m_SysFreeString;
        return fp(bstrString);
    }

    SAFEARRAY *SafeArrayCreateVector(VARTYPE vt, LONG lLbound, ULONG cElements)
    {
        typedef SAFEARRAY*(WINAPI *ft)(VARTYPE, LONG, ULONG);
        ft fp = (ft)m_SafeArrayCreateVector;
        return fp(vt, lLbound, cElements);
    }

    HRESULT SafeArrayAccessData(SAFEARRAY *psa, void **ppvData)
    {
        typedef HRESULT(WINAPI *ft)(SAFEARRAY*, void**);
        ft fp = (ft)m_SafeArrayAccessData;
        return fp(psa, ppvData);
    }

    HRESULT SafeArrayDestroy(SAFEARRAY *psa)
    {
        typedef HRESULT(WINAPI *ft)(SAFEARRAY*);
        ft fp = (ft)m_SafeArrayDestroy;
        return fp(psa);
    }

private:
    OLEAut32()
        : m_SysAllocString(Q_NULLPTR)
        , m_SysFreeString(Q_NULLPTR)
        , m_SafeArrayCreateVector(Q_NULLPTR)
        , m_SafeArrayAccessData(Q_NULLPTR)
        , m_SafeArrayDestroy(Q_NULLPTR)
    {
        HMODULE library = ::LoadLibraryA("oleaut32");
        if(!library)
            return;

        m_SysAllocString = ::GetProcAddress(library, "SysAllocString");
        m_SysFreeString = ::GetProcAddress(library, "SysFreeString");
        m_SafeArrayCreateVector = ::GetProcAddress(library, "SafeArrayCreateVector");
        m_SafeArrayAccessData = ::GetProcAddress(library, "SafeArrayAccessData");
        m_SafeArrayDestroy = ::GetProcAddress(library, "SafeArrayDestroy");
    }

    ~OLEAut32()
    {}

    bool isValid() const
    {
        return m_SysAllocString && m_SysFreeString
                && m_SafeArrayCreateVector && m_SafeArrayAccessData && m_SafeArrayDestroy;
    }

    FARPROC m_SysAllocString;
    FARPROC m_SysFreeString;
    FARPROC m_SafeArrayCreateVector;
    FARPROC m_SafeArrayAccessData;
    FARPROC m_SafeArrayDestroy;
};

BSTR SysAllocString_WRAP(const OLECHAR *psz)
{
    if(OLEAut32 *oleaut32 = OLEAut32::instance())
        return oleaut32->SysAllocString(psz);
    LOG_WARNING() << LOGGING_CTX << "Failed to load oleaut32.dll";
    return Q_NULLPTR;
}
#define SysAllocString SysAllocString_WRAP

void SysFreeString_WRAP(BSTR bstrString)
{
    if(OLEAut32 *oleaut32 = OLEAut32::instance())
        return oleaut32->SysFreeString(bstrString);
    LOG_WARNING() << LOGGING_CTX << "Failed to load oleaut32.dll";
}
#define SysFreeString SysFreeString_WRAP

SAFEARRAY *SafeArrayCreateVector_WRAP(VARTYPE vt, LONG lLbound, ULONG cElements)
{
    if(OLEAut32 *oleaut32 = OLEAut32::instance())
        return oleaut32->SafeArrayCreateVector(vt, lLbound, cElements);
    LOG_WARNING() << LOGGING_CTX << "Failed to load oleaut32.dll";
    return Q_NULLPTR;
}
#define SafeArrayCreateVector SafeArrayCreateVector_WRAP

HRESULT SafeArrayAccessData_WRAP(SAFEARRAY *psa, void **ppvData)
{
    if(OLEAut32 *oleaut32 = OLEAut32::instance())
        return oleaut32->SafeArrayAccessData(psa, ppvData);
    LOG_WARNING() << LOGGING_CTX << "Failed to load oleaut32.dll";
    return E_FAIL;
}
#define SafeArrayAccessData SafeArrayAccessData_WRAP

HRESULT SafeArrayDestroy_WRAP(SAFEARRAY *psa)
{
    if(OLEAut32 *oleaut32 = OLEAut32::instance())
        return oleaut32->SafeArrayDestroy(psa);
    LOG_WARNING() << LOGGING_CTX << "Failed to load oleaut32.dll";
    return E_FAIL;
}
#define SafeArrayDestroy SafeArrayDestroy_WRAP

// ====================================================================================================

IID GetIID(LPCOLESTR lpsz)
{
    IID result;
    memset(&result, 0, sizeof(IID));
    HRESULT hr = IIDFromString(lpsz, &result);
    if(!SUCCEEDED(hr))
        LOG_WARNING() << LOGGING_CTX << "Can't get IID for" << QString::fromStdWString(lpsz);
    return result;
}

#define IID_IHTMLDocument2  GetIID(L"{332c4425-26cb-11d0-b483-00c04fd90119}")
#define IID_IHTMLDocument3  GetIID(L"{3050f485-98b5-11cf-bb82-00aa00bdce0b}")
#define IID_IOleObject      GetIID(L"{00000112-0000-0000-C000-000000000046}")
#define IID_IViewObject     GetIID(L"{0000010D-0000-0000-C000-000000000046}")

CLSID GetCLSID(LPCOLESTR lpsz)
{
    CLSID result;
    memset(&result, 0, sizeof(CLSID));
    HRESULT hr = CLSIDFromString(lpsz, &result);
    if(!SUCCEEDED(hr))
        LOG_WARNING() << LOGGING_CTX << "Can't get CLSID for" << QString::fromStdWString(lpsz);
    return result;
}

#define CLSID_HTMLDocument  GetCLSID(L"{25336920-03F9-11cf-8FD0-00AA00686F13}")

// ====================================================================================================

QImage QImageFromHBITMAP(HBITMAP bitmap)
{
    BITMAP bitmap_info;
    memset(&bitmap_info, 0, sizeof(BITMAP));
    if(!GetObject(bitmap, sizeof(BITMAP), &bitmap_info))
    {
        LOG_WARNING() << LOGGING_CTX << "Failed to get bitmap info";
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
        LOG_WARNING() << LOGGING_CTX << "Failed to get bitmap bits";
        result = QImage();
    }
    ReleaseDC(0, hdc);
    return result;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
int compare(const QStringView &s1, const QString &s2, Qt::CaseSensitivity cs)
#else
int compare(const QStringRef &s1, const QString &s2, Qt::CaseSensitivity cs)
#endif
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    return s1.compare(s2, cs);
#else
    return s1.toString().compare(s2, cs);
#endif
}

// ====================================================================================================

class MSHTMLPixmapProvider : public IScaledImageProvider, public AbstractSVGWebBrowserNoJS
{
    Q_DISABLE_COPY(MSHTMLPixmapProvider)

public:
    explicit MSHTMLPixmapProvider(const QString &filePath)
        : m_isValid(false)
        , m_coInitResult(S_OK)
        , m_htmlDocument2(Q_NULLPTR)
        , m_minScaleFactor(1)
        , m_maxScaleFactor(1)
    {
        m_coInitResult = CoInitializeEx(Q_NULLPTR, COINIT_APARTMENTTHREADED);

        m_svgData = MappedBuffer(filePath, MappedBuffer::AutoInflate).byteArray();
        if(m_svgData.isEmpty())
        {
            LOG_WARNING() << LOGGING_CTX << "Can't read" << filePath;
            return;
        }

        if(!rootElementIsSvg())
        {
            LOG_WARNING() << LOGGING_CTX << "Root element is not svg";
            return;
        }

        if(!prepareSVGData())
        {
            LOG_WARNING() << LOGGING_CTX << "Can't prepare svg data";
            return;
        }

        HRESULT hr = E_FAIL;

//        GUID CLSID_HTMLDocument;
//        hr = CLSIDFromProgID(L"htmlfile", &CLSID_HTMLDocument);
//        if(!SUCCEEDED(hr))
//        {
//            LOG_WARNING() << LOGGING_CTX << "Can't get CLSID for mshtml" << QString::number(hr, 16);
//            return;
//        }
        Q_UNUSED(&CLSIDFromProgID);

        hr = CoCreateInstance(CLSID_HTMLDocument, Q_NULLPTR, CLSCTX_INPROC_SERVER, IID_IHTMLDocument2, (void**)&m_htmlDocument2);
        if(!SUCCEEDED(hr))
        {
            LOG_WARNING() << LOGGING_CTX << "CoCreateInstance(CLSID_HTMLDocument) failed" << QString::number(hr, 16);
            return;
        }

#if defined (__IHTMLDocument3_INTERFACE_DEFINED__) /// @todo
        IHTMLDocument3 *htmlDocument3 = Q_NULLPTR;
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
            LOG_WARNING() << LOGGING_CTX << "Failed to load svg data";
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
        if(m_coInitResult == S_OK || m_coInitResult == S_FALSE)
            CoUninitialize();
    }

    bool isValid() const Q_DECL_OVERRIDE
    {
        return m_isValid;
    }

    bool requiresMainThread() const Q_DECL_OVERRIDE
    {
        return true;
    }

    QRectF boundingRect() const Q_DECL_OVERRIDE
    {
        return QRectF(0, 0, m_svgRect.width(), m_svgRect.height());
    }

    QImage image(const qreal scaleFactor) Q_DECL_OVERRIDE
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

    qreal minScaleFactor() const Q_DECL_OVERRIDE
    {
        return m_minScaleFactor;
    }

    qreal maxScaleFactor() const Q_DECL_OVERRIDE
    {
        return m_maxScaleFactor;
    }

protected:
    QByteArray getSvgData() Q_DECL_OVERRIDE
    {
        return m_svgData;
    }

//    QVariant evalJSImpl(const QString &scriptSource) Q_DECL_OVERRIDE
//    {
//        HRESULT hr = E_FAIL;

//        IHTMLWindow2 *htmlWindow2 = Q_NULLPTR;
//        hr = m_htmlDocument2->get_parentWindow(&htmlWindow2);
//        if(!SUCCEEDED(hr))
//        {
//            LOG_WARNING() << LOGGING_CTX << "IHtmlDocument2::get_parentWindow() failed with error =" << QString::number(hr, 16);
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
//            LOG_WARNING() << LOGGING_CTX << "htmlWindow2::execScript() failed with error =" << QString::number(hr, 16);
//            return QVariant();
//        }

//        LOG_DEBUG() << LOGGING_CTX << result.vt;
//        return QVariant();
//    }

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

        XmlStreamReader reader(m_svgData);
        while(!reader.atEnd())
        {
            reader.readNext();

            if(reader.hasError())
            {
                LOG_WARNING() << LOGGING_CTX << reader.errorString();
                return false;
            }

            writer.writeCurrentToken(reader);
            if(reader.tokenType() == QXmlStreamReader::StartElement)
            {
                if(fakeViewBoxRequired && !compare(reader.name(), QString::fromLatin1("svg"), Qt::CaseInsensitive))
                {
                    writer.writeAttribute(QString::fromLatin1("viewBox"), QString::fromLatin1("%1 %2 %3 %4").arg(0).arg(0).arg(size.width()).arg(size.height()));
                    LOG_DEBUG() << LOGGING_CTX << "Fake viewBox injected";
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
        VARIANT *param = Q_NULLPTR;
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

        IOleObject *oleObject = Q_NULLPTR;
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
        IViewObject *viewObject = Q_NULLPTR;
        if(!SUCCEEDED(m_htmlDocument2->QueryInterface(IID_IViewObject, (void**)&viewObject)))
            return QImage();

        RECTL clientRect;
        memset(&clientRect, 0, sizeof(RECTL));
        clientRect.right = width;
        clientRect.bottom = height;

        QImage result;
        HWND hwnd = 0;
        HDC hdc = GetDC(hwnd);
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmp = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(hdcMem, hbmp);
        HRESULT hr = viewObject->Draw(DVASPECT_CONTENT, -1, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR, hdcMem, &clientRect, Q_NULLPTR, Q_NULLPTR, 0);
        if(SUCCEEDED(hr))
            result = QImageFromHBITMAP(hbmp);
        DeleteObject(hbmp);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdc);
        viewObject->Release();

        return result;
    }

private:
    bool m_isValid;
    HRESULT m_coInitResult;
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
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderMSHTML");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList()
                << QString::fromLatin1("svg")
#if defined (HAS_ZLIB)
                << QString::fromLatin1("svgz")
#endif
                   ;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        static Qt::CheckState state = Qt::Unchecked;
        if(state != Qt::Unchecked)
            return state == Qt::Checked;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        if(QSysInfo::windowsVersion() < QSysInfo::WV_VISTA)
        {
            LOG_DEBUG() << LOGGING_CTX << "DecoderMSHTML unavailable, reason: windowsVersion() < QSysInfo::WV_VISTA";
            state = Qt::PartiallyChecked;
            return false;
        }
#endif

        QSettings settings(QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer"), QSettings::NativeFormat);
        const QString ieVersion = settings.value(QString::fromLatin1("Version")).toString();
        if(ieVersion.isEmpty())
        {
            LOG_DEBUG() << LOGGING_CTX << "DecoderMSHTML unavailable, reason: Can't detect IE version";
            state = Qt::PartiallyChecked;
            return false;
        }

        LOG_DEBUG() << LOGGING_CTX << "IE version:" << ieVersion;
        if(ieVersion.split(QChar::fromLatin1('.'), Qt_SkipEmptyParts).first().toInt() < 9)
        {
            LOG_DEBUG() << LOGGING_CTX << "DecoderMSHTML unavailable, reason: Required IE >= 9";
            state = Qt::PartiallyChecked;
            return false;
        }

        const bool result = OLE32::instance() && OLEAut32::instance();
        state = result ? Qt::Checked : Qt::PartiallyChecked;
        return result;
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable() || !isAvailable())
            return QSharedPointer<IImageData>();
        IScaledImageProvider *provider = new MSHTMLPixmapProvider(filePath);
        QGraphicsItem *item = GraphicsItemsFactory::instance().createScalableItem(provider);
        IImageMetaData *metaData = item ? ImageMetaData::createMetaData(filePath) : Q_NULLPTR;
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }
};

DecoderAutoRegistrator registrator(new DecoderMSHTML);

// ====================================================================================================

} // namespace
