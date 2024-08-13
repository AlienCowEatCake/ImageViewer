/*
   Copyright (C) 2021-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <windows.h>
#include <wincodec.h>

#include <QImage>
#include <QFileInfo>
#include <QSettings>
#include <QVariant>

#include "Utils/Global.h"
#include "Utils/Logging.h"

#include "../IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/GraphicsItemsFactory.h"
#include "Internal/ImageData.h"
#include "Internal/ImageMetaData.h"

//#pragma comment(lib, "ole32.lib") // CoInitialize, CoUninitialize, CoCreateInstance
//#pragma comment(lib, "windowscodecs.lib")

namespace {

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

    HRESULT CoInitialize(LPVOID pvReserved)
    {
        typedef HRESULT(WINAPI *CoInitialize_t)(LPVOID);
        CoInitialize_t CoInitialize_f = (CoInitialize_t)m_CoInitialize;
        return CoInitialize_f(pvReserved);
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
        : m_CoInitialize(Q_NULLPTR)
        , m_CoUninitialize(Q_NULLPTR)
        , m_CoCreateInstance(Q_NULLPTR)
        , m_IIDFromString(Q_NULLPTR)
        , m_CLSIDFromString(Q_NULLPTR)
    {
        HMODULE library = ::LoadLibraryA("ole32");
        if(!library)
            return;

        m_CoInitialize = ::GetProcAddress(library, "CoInitialize");
        m_CoUninitialize = ::GetProcAddress(library, "CoUninitialize");
        m_CoCreateInstance = ::GetProcAddress(library, "CoCreateInstance");
        m_IIDFromString = ::GetProcAddress(library, "IIDFromString");
        m_CLSIDFromString = ::GetProcAddress(library, "CLSIDFromString");
    }

    ~OLE32()
    {}

    bool isValid() const
    {
        return m_CoInitialize && m_CoUninitialize && m_CoCreateInstance &&
                m_IIDFromString && m_CLSIDFromString;
    }

    FARPROC m_CoInitialize;
    FARPROC m_CoUninitialize;
    FARPROC m_CoCreateInstance;
    FARPROC m_IIDFromString;
    FARPROC m_CLSIDFromString;
};

HRESULT CoInitialize_WRAP(LPVOID pvReserved)
{
    if(OLE32 *ole32 = OLE32::instance())
        return ole32->CoInitialize(pvReserved);
    LOG_WARNING() << LOGGING_CTX << "Failed to load ole32.dll";
    return E_FAIL;
}
#define CoInitialize CoInitialize_WRAP

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

IID GetIID(LPCOLESTR lpsz)
{
    IID result;
    memset(&result, 0, sizeof(IID));
    HRESULT hr = IIDFromString(lpsz, &result);
    if(!SUCCEEDED(hr))
        LOG_WARNING() << LOGGING_CTX << "Can't get IID for" << QString::fromStdWString(lpsz);
    return result;
}

#define IID_IWICImagingFactory GetIID(L"{ec5ec8a9-c395-4314-9c77-54d7a935ff70}")
#define IID_IWICBitmapCodecInfo GetIID(L"{E87A44C4-B76E-4c47-8B09-298EB12A2714}")

CLSID GetCLSID(LPCOLESTR lpsz)
{
    CLSID result;
    memset(&result, 0, sizeof(CLSID));
    HRESULT hr = CLSIDFromString(lpsz, &result);
    if(!SUCCEEDED(hr))
        LOG_WARNING() << LOGGING_CTX << "Can't get CLSID for" << QString::fromStdWString(lpsz);
    return result;
}

#if defined (CLSID_WICImagingFactory)
#undef CLSID_WICImagingFactory
#endif
#define CLSID_WICImagingFactory GetCLSID(L"{cacaf262-9370-4615-a13b-9f5539da4c0a}")

const GUID GUID_WICPixelFormat32bppBGRA_WRAP = { 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0f} };
#define GUID_WICPixelFormat32bppBGRA GUID_WICPixelFormat32bppBGRA_WRAP

// ====================================================================================================

class DecoderWIC : public IDecoder
{
public:
    QString name() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("DecoderWIC");
    }

    QStringList supportedFormats() const Q_DECL_OVERRIDE
    {
        if(!isAvailable())
            return QStringList();

        QStringList result = GetWICDecodersExtensions()
                // https://docs.microsoft.com/en-us/windows/win32/wic/bmp-format-overview
                << QString::fromLatin1("bmp")
                << QString::fromLatin1("dib")
                // https://docs.microsoft.com/en-us/windows/win32/wic/dds-format-overview
                << QString::fromLatin1("dds")
                // https://docs.microsoft.com/en-us/windows/win32/wic/dng-format-overview
                << QString::fromLatin1("dng")
                // https://docs.microsoft.com/en-us/windows/win32/wic/gif-format-overview
                << QString::fromLatin1("gif")
                // https://docs.microsoft.com/en-us/windows/win32/wic/hdphoto-format-overview
                << QString::fromLatin1("wdp")
                // https://docs.microsoft.com/en-us/windows/win32/wic/ico-format-overview
                << QString::fromLatin1("ico")
                // https://docs.microsoft.com/en-us/windows/win32/wic/jpeg-format-overview
                << QString::fromLatin1("jpe")
                << QString::fromLatin1("jpeg")
                << QString::fromLatin1("jpg")
                // https://docs.microsoft.com/en-us/windows/win32/wic/jpeg-xr-codec
                << QString::fromLatin1("jxr")
                << QString::fromLatin1("wdp")
                // https://docs.microsoft.com/en-us/windows/win32/wic/png-format-overview
                << QString::fromLatin1("png")
                // https://docs.microsoft.com/en-us/windows/win32/wic/tiff-format-overview
                << QString::fromLatin1("tiff")
                << QString::fromLatin1("tif");

        // https://github.com/ReneSlijkhuis/gimp-wic-plugin/blob/6afd01211334007963ff9a33f75d3b64759c5062/wic_library/Utilities/WicUtilities.cpp
        const QSettings settings(QString::fromLatin1("HKEY_CLASSES_ROOT\\CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance"), QSettings::NativeFormat);
        const QStringList wicCodecs = settings.childGroups();
        for(QStringList::ConstIterator it = wicCodecs.constBegin(); it != wicCodecs.constEnd(); ++it)
        {
            const QSettings settings(QString::fromLatin1("HKEY_CLASSES_ROOT\\CLSID\\%1").arg(*it), QSettings::NativeFormat);
            const QStringList extensions = settings.value(QString::fromLatin1("FileExtensions"))
                    .toString()
                    .toLower()
                    .remove(QChar::fromLatin1('.'))
                    .remove(QChar::fromLatin1(' '))
                    .split(QChar::fromLatin1(','));
            for(QStringList::ConstIterator jt = extensions.constBegin(); jt != extensions.constEnd(); ++jt)
                result.append(*jt);
        }

#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
        result.removeDuplicates();
#else
        result = QStringList::fromSet(result.toSet());
#endif
        return result;
    }

    QStringList advancedFormats() const Q_DECL_OVERRIDE
    {
        return QStringList();
    }

    bool isAvailable() const Q_DECL_OVERRIDE
    {
        return !!OLE32::instance();
    }

    QSharedPointer<IImageData> loadImage(const QString &filePath) Q_DECL_OVERRIDE
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return QSharedPointer<IImageData>();

        const HRESULT coInitResult = CoInitialize(Q_NULLPTR);

        IWICImagingFactory *factory = Q_NULLPTR;
        if(FAILED(CoCreateInstance(CLSID_WICImagingFactory, Q_NULLPTR, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<LPVOID*>(&factory))))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: CoCreateInstance failed";
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QSharedPointer<IImageData>();
        }

        IWICBitmapDecoder *decoder = Q_NULLPTR;
        if(FAILED(factory->CreateDecoderFromFilename(reinterpret_cast<LPCWSTR>(filePath.constData()), Q_NULLPTR, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: factory->CreateDecoderFromFilename failed";
            factory->Release();
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QSharedPointer<IImageData>();
        }

        IWICBitmapFrameDecode *frame = Q_NULLPTR;
        if(FAILED(decoder->GetFrame(0, &frame)))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: decoder->GetFrame failed";
            decoder->Release();
            factory->Release();
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QSharedPointer<IImageData>();
        }

        UINT width = 0;
        UINT height = 0;
        if(FAILED(frame->GetSize(&width, &height)))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: frame->GetSize failed";
            frame->Release();
            decoder->Release();
            factory->Release();
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QSharedPointer<IImageData>();
        }

        IWICFormatConverter *formatConverter = Q_NULLPTR;
        if(FAILED(factory->CreateFormatConverter(&formatConverter)))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: factory->CreateFormatConverter failed";
            frame->Release();
            decoder->Release();
            factory->Release();
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QSharedPointer<IImageData>();
        }

        if(FAILED(formatConverter->Initialize(frame, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, Q_NULLPTR, 0.0f, WICBitmapPaletteTypeCustom)))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: formatConverter->Initialize failed";
            formatConverter->Release();
            frame->Release();
            decoder->Release();
            factory->Release();
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QSharedPointer<IImageData>();
        }

        QImage image(width, height, QImage::Format_ARGB32);
        if(image.isNull())
        {
            LOG_WARNING() << LOGGING_CTX << "Error: invalid image size";
            formatConverter->Release();
            frame->Release();
            decoder->Release();
            factory->Release();
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QSharedPointer<IImageData>();
        }

        if(FAILED(formatConverter->CopyPixels(Q_NULLPTR, static_cast<UINT>(image.bytesPerLine()), static_cast<UINT>(image.bytesPerLine() * image.height()), reinterpret_cast<BYTE*>(image.bits()))))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: formatConverter->CopyPixels failed";
            formatConverter->Release();
            frame->Release();
            decoder->Release();
            factory->Release();
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QSharedPointer<IImageData>();
        }

        formatConverter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        if(coInitResult == S_OK || coInitResult == S_FALSE)
            CoUninitialize();

        ImageMetaData *metaData = ImageMetaData::createMetaData(filePath);
        if(metaData)
            metaData->applyExifOrientation(&image);

        QGraphicsItem *item = GraphicsItemsFactory::instance().createImageItem(image);
        return QSharedPointer<IImageData>(new ImageData(item, filePath, name(), metaData));
    }

private:
    QStringList GetWICDecodersExtensions() const
    {
        const HRESULT coInitResult = CoInitialize(Q_NULLPTR);

        IWICImagingFactory *factory = Q_NULLPTR;
        if(FAILED(CoCreateInstance(CLSID_WICImagingFactory, Q_NULLPTR, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<LPVOID*>(&factory))))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: CoCreateInstance failed";
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QStringList();
        }

        IEnumUnknown *enumerator = Q_NULLPTR;
        if(FAILED(factory->CreateComponentEnumerator(WICDecoder, WICComponentEnumerateRefresh, &enumerator)))
        {
            LOG_WARNING() << LOGGING_CTX << "Error: CreateComponentEnumerator failed";
            factory->Release();
            if(coInitResult == S_OK || coInitResult == S_FALSE)
                CoUninitialize();
            return QStringList();
        }

        QStringList allExtensions;
        ULONG num = 0;
        IUnknown *unk = Q_NULLPTR;
        while(enumerator->Next(1, &unk, &num) == S_OK && num == 1)
        {
            IWICBitmapCodecInfo *codecInfo = Q_NULLPTR;
            if(SUCCEEDED(unk->QueryInterface(IID_IWICBitmapCodecInfo, (void**)&codecInfo)))
            {
                UINT strLen = 0;
                QString friendlyName;
                if(SUCCEEDED(codecInfo->GetFriendlyName(0, 0, &strLen)) && strLen > 0)
                {
                    QVector<WCHAR> friendlyNameData(strLen + 1, 0);
                    codecInfo->GetFriendlyName(strLen, friendlyNameData.data(), &strLen);
                    friendlyName = QString::fromStdWString(std::wstring(friendlyNameData.data(), strLen - 1));
                }
                QString fileExtensions;
                if(SUCCEEDED(codecInfo->GetFileExtensions(0, 0, &strLen)) && strLen > 0)
                {
                    QVector<WCHAR> fileExtensionsData(strLen + 1, 0);
                    codecInfo->GetFileExtensions(strLen, fileExtensionsData.data(), &strLen);
                    fileExtensions = QString::fromStdWString(std::wstring(fileExtensionsData.data(), strLen - 1));
                }
                LOG_INFO() << LOGGING_CTX << friendlyName << "-" << fileExtensions;

                const QStringList extensionsList = fileExtensions
                        .toLower()
                        .remove(QChar::fromLatin1('.'))
                        .remove(QChar::fromLatin1(' '))
                        .split(QChar::fromLatin1(','));
                for(QStringList::ConstIterator jt = extensionsList.constBegin(); jt != extensionsList.constEnd(); ++jt)
                    allExtensions.append(*jt);

                codecInfo->Release();
            }
            unk->Release();
        }

        enumerator->Release();
        factory->Release();
        if(coInitResult == S_OK || coInitResult == S_FALSE)
            CoUninitialize();
        return allExtensions;
    }
};

DecoderAutoRegistrator registrator(new DecoderWIC, false);

// ====================================================================================================

} // namespace
