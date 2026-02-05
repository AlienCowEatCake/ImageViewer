/*
   Copyright (C) 2021-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
//#include <wincodec.h>

#include <QImage>
#include <QFileInfo>
#include <QSet>
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

// ====================================================================================================

namespace {

#if defined (WICDecodeOptions)
#undef WICDecodeOptions
#endif
#define WICDecodeOptions WICDecodeOptions_WRAP
typedef enum WICDecodeOptions
{
    WICDecodeMetadataCacheOnDemand      = 0,
    WICDecodeMetadataCacheOnLoad        = 0x1,
    WICMETADATACACHEOPTION_FORCE_DWORD  = 0x7fffffff
} WICDecodeOptions;

#if defined (WICBitmapDitherType)
#undef WICBitmapDitherType
#endif
#define WICBitmapDitherType WICBitmapDitherType_WRAP
typedef enum WICBitmapDitherType
{
    WICBitmapDitherTypeNone             = 0,
    WICBitmapDitherTypeSolid            = 0,
    WICBitmapDitherTypeOrdered4x4       = 0x1,
    WICBitmapDitherTypeOrdered8x8       = 0x2,
    WICBitmapDitherTypeOrdered16x16     = 0x3,
    WICBitmapDitherTypeSpiral4x4        = 0x4,
    WICBitmapDitherTypeSpiral8x8        = 0x5,
    WICBitmapDitherTypeDualSpiral4x4    = 0x6,
    WICBitmapDitherTypeDualSpiral8x8    = 0x7,
    WICBitmapDitherTypeErrorDiffusion   = 0x8,
    WICBITMAPDITHERTYPE_FORCE_DWORD     = 0x7fffffff
} WICBitmapDitherType;

#if defined (WICBitmapPaletteType)
#undef WICBitmapPaletteType
#endif
#define WICBitmapPaletteType WICBitmapPaletteType_WRAP
typedef enum WICBitmapPaletteType
{
    WICBitmapPaletteTypeCustom              = 0,
    WICBitmapPaletteTypeMedianCut           = 0x1,
    WICBitmapPaletteTypeFixedBW             = 0x2,
    WICBitmapPaletteTypeFixedHalftone8      = 0x3,
    WICBitmapPaletteTypeFixedHalftone27     = 0x4,
    WICBitmapPaletteTypeFixedHalftone64     = 0x5,
    WICBitmapPaletteTypeFixedHalftone125    = 0x6,
    WICBitmapPaletteTypeFixedHalftone216    = 0x7,
    WICBitmapPaletteTypeFixedWebPalette     = WICBitmapPaletteTypeFixedHalftone216,
    WICBitmapPaletteTypeFixedHalftone252    = 0x8,
    WICBitmapPaletteTypeFixedHalftone256    = 0x9,
    WICBitmapPaletteTypeFixedGray4          = 0xa,
    WICBitmapPaletteTypeFixedGray16         = 0xb,
    WICBitmapPaletteTypeFixedGray256        = 0xc,
    WICBITMAPPALETTETYPE_FORCE_DWORD        = 0x7fffffff
} WICBitmapPaletteType;

#if defined (WICComponentType)
#undef WICComponentType
#endif
#define WICComponentType WICComponentType_WRAP
typedef enum WICComponentType
{
    WICDecoder                      = 0x1,
    WICEncoder                      = 0x2,
    WICPixelFormatConverter         = 0x4,
    WICMetadataReader               = 0x8,
    WICMetadataWriter               = 0x10,
    WICPixelFormat                  = 0x20,
    WICAllComponents                = 0x3f,
    WICCOMPONENTTYPE_FORCE_DWORD    = 0x7fffffff
} WICComponentType;

#if defined (WICComponentEnumerateOptions)
#undef WICComponentEnumerateOptions
#endif
#define WICComponentEnumerateOptions WICComponentEnumerateOptions_WRAP
typedef enum WICComponentEnumerateOptions
{
    WICComponentEnumerateDefault                = 0,
    WICComponentEnumerateRefresh                = 0x1,
    WICComponentEnumerateDisabled               = 0x80000000,
    WICComponentEnumerateUnsigned               = 0x40000000,
    WICComponentEnumerateBuiltInOnly            = 0x20000000,
    WICCOMPONENTENUMERATEOPTIONS_FORCE_DWORD    = 0x7fffffff
} WICComponentEnumerateOptions;

#if defined (WICBitmapCreateCacheOption)
#undef WICBitmapCreateCacheOption
#endif
#define WICBitmapCreateCacheOption WICBitmapCreateCacheOption_WRAP
typedef int WICBitmapCreateCacheOption;

#if defined (WICBitmapAlphaChannelOption)
#undef WICBitmapAlphaChannelOption
#endif
#define WICBitmapAlphaChannelOption WICBitmapAlphaChannelOption_WRAP
typedef int WICBitmapAlphaChannelOption;

} // namespace

// ====================================================================================================

#if defined (REFWICPixelFormatGUID)
#undef REFWICPixelFormatGUID
#endif
#define REFWICPixelFormatGUID REFWICPixelFormatGUID_WRAP
typedef REFGUID REFWICPixelFormatGUID;

#if defined (WICPixelFormatGUID)
#undef WICPixelFormatGUID
#endif
#define WICPixelFormatGUID WICPixelFormatGUID_WRAP
typedef GUID WICPixelFormatGUID;

// ====================================================================================================

#if defined (IWICPalette)
#undef IWICPalette
#endif
#define IWICPalette IWICPalette_WRAP
struct IWICPalette;

#if defined (WICRect)
#undef WICRect
#endif
#define WICRect WICRect_WRAP
struct WICRect;

#if defined (IWICMetadataQueryReader)
#undef IWICMetadataQueryReader
#endif
#define IWICMetadataQueryReader IWICMetadataQueryReader_WRAP
struct IWICMetadataQueryReader;

#if defined (IWICColorContext)
#undef IWICColorContext
#endif
#define IWICColorContext IWICColorContext_WRAP
struct IWICColorContext;

#if defined (IWICBitmapDecoderInfo)
#undef IWICBitmapDecoderInfo
#endif
#define IWICBitmapDecoderInfo IWICBitmapDecoderInfo_WRAP
struct IWICBitmapDecoderInfo;

#if defined (IWICBitmapEncoder)
#undef IWICBitmapEncoder
#endif
#define IWICBitmapEncoder IWICBitmapEncoder_WRAP
struct IWICBitmapEncoder;

#if defined (IWICBitmapScaler)
#undef IWICBitmapScaler
#endif
#define IWICBitmapScaler IWICBitmapScaler_WRAP
struct IWICBitmapScaler;

#if defined (IWICBitmapClipper)
#undef IWICBitmapClipper
#endif
#define IWICBitmapClipper IWICBitmapClipper_WRAP
struct IWICBitmapClipper;

#if defined (IWICBitmapFlipRotator)
#undef IWICBitmapFlipRotator
#endif
#define IWICBitmapFlipRotator IWICBitmapFlipRotator_WRAP
struct IWICBitmapFlipRotator;

#if defined (IWICStream)
#undef IWICStream
#endif
#define IWICStream IWICStream_WRAP
struct IWICStream;

#if defined (IWICColorTransform)
#undef IWICColorTransform
#endif
#define IWICColorTransform IWICColorTransform_WRAP
struct IWICColorTransform;

#if defined (IWICBitmap)
#undef IWICBitmap
#endif
#define IWICBitmap IWICBitmap_WRAP
struct IWICBitmap;

#if defined (IWICFastMetadataEncoder)
#undef IWICFastMetadataEncoder
#endif
#define IWICFastMetadataEncoder IWICFastMetadataEncoder_WRAP
struct IWICFastMetadataEncoder;

#if defined (IWICMetadataQueryWriter)
#undef IWICMetadataQueryWriter
#endif
#define IWICMetadataQueryWriter IWICMetadataQueryWriter_WRAP
struct IWICMetadataQueryWriter;

// ====================================================================================================

#if !defined (STDMETHODCALLTYPE)
#define STDMETHODCALLTYPE WINAPI
#endif

#if defined (IWICComponentInfo)
#undef IWICComponentInfo
#endif
#define IWICComponentInfo IWICComponentInfo_WRAP
struct IWICComponentInfo : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetComponentType(WICComponentType *pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCLSID(CLSID *pclsid) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSigningStatus(DWORD *pStatus) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAuthor(UINT cchAuthor, WCHAR *wzAuthor, UINT *pcchActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVendorGUID(GUID *pguidVendor) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVersion(UINT cchVersion, WCHAR *wzVersion, UINT *pcchActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSpecVersion(UINT cchSpecVersion, WCHAR *wzSpecVersion, UINT *pcchActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFriendlyName(UINT cchFriendlyName, WCHAR *wzFriendlyName, UINT *pcchActual) = 0;
};

#if defined (IWICBitmapCodecInfo)
#undef IWICBitmapCodecInfo
#endif
#define IWICBitmapCodecInfo IWICBitmapCodecInfo_WRAP
struct IWICBitmapCodecInfo : public IWICComponentInfo
{
    virtual HRESULT STDMETHODCALLTYPE GetContainerFormat(GUID *pguidContainerFormat) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPixelFormats(UINT cFormats, GUID *pguidPixelFormats, UINT *pcActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetColorManagementVersion(UINT cchColorManagementVersion, WCHAR *wzColorManagementVersion, UINT *pcchActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceManufacturer(UINT cchDeviceManufacturer, WCHAR *wzDeviceManufacturer, UINT *pcchActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceModels(UINT cchDeviceModels, WCHAR *wzDeviceModels, UINT *pcchActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMimeTypes(UINT cchMimeTypes, WCHAR *wzMimeTypes, UINT *pcchActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFileExtensions(UINT cchFileExtensions, WCHAR *wzFileExtensions, UINT *pcchActual) = 0;
    virtual HRESULT STDMETHODCALLTYPE DoesSupportAnimation(BOOL *pfSupportAnimation) = 0;
    virtual HRESULT STDMETHODCALLTYPE DoesSupportChromakey(BOOL *pfSupportChromakey) = 0;
    virtual HRESULT STDMETHODCALLTYPE DoesSupportLossless(BOOL *pfSupportLossless) = 0;
    virtual HRESULT STDMETHODCALLTYPE DoesSupportMultiframe(BOOL *pfSupportMultiframe) = 0;
    virtual HRESULT STDMETHODCALLTYPE MatchesMimeType(LPCWSTR wzMimeType, BOOL *pfMatches) = 0;
};

#if defined (IWICBitmapSource)
#undef IWICBitmapSource
#endif
#define IWICBitmapSource IWICBitmapSource_WRAP
struct IWICBitmapSource : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetSize(UINT *puiWidth, UINT *puiHeight) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPixelFormat(WICPixelFormatGUID *pPixelFormat) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetResolution(double *pDpiX, double *pDpiY) = 0;
    virtual HRESULT STDMETHODCALLTYPE CopyPalette(IWICPalette *pIPalette) = 0;
    virtual HRESULT STDMETHODCALLTYPE CopyPixels(const WICRect *prc, UINT cbStride, UINT cbBufferSize, BYTE *pbBuffer) = 0;
};

#if defined (IWICFormatConverter)
#undef IWICFormatConverter
#endif
#define IWICFormatConverter IWICFormatConverter_WRAP
struct IWICFormatConverter : public IWICBitmapSource
{
    virtual HRESULT STDMETHODCALLTYPE Initialize(IWICBitmapSource *pISource, REFWICPixelFormatGUID dstFormat, WICBitmapDitherType dither, IWICPalette *pIPalette, double alphaThresholdPercent, WICBitmapPaletteType paletteTranslate) = 0;
    virtual HRESULT STDMETHODCALLTYPE CanConvert(REFWICPixelFormatGUID srcPixelFormat, REFWICPixelFormatGUID dstPixelFormat, BOOL *pfCanConvert) = 0;
};

#if defined (IWICBitmapFrameDecode)
#undef IWICBitmapFrameDecode
#endif
#define IWICBitmapFrameDecode IWICBitmapFrameDecode_WRAP
struct IWICBitmapFrameDecode : public IWICBitmapSource
{
    virtual HRESULT STDMETHODCALLTYPE GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetColorContexts(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetThumbnail(IWICBitmapSource **ppIThumbnail) = 0;
};

#if defined (IWICBitmapDecoder)
#undef IWICBitmapDecoder
#endif
#define IWICBitmapDecoder IWICBitmapDecoder_WRAP
struct IWICBitmapDecoder : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE QueryCapability(IStream *pIStream, DWORD *pdwCapability) = 0;
    virtual HRESULT STDMETHODCALLTYPE Initialize(IStream *pIStream, WICDecodeOptions cacheOptions) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetContainerFormat(GUID *pguidContainerFormat) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDecoderInfo(IWICBitmapDecoderInfo **ppIDecoderInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE CopyPalette(IWICPalette *pIPalette) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPreview(IWICBitmapSource **ppIBitmapSource) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetColorContexts(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetThumbnail(IWICBitmapSource **ppIThumbnail) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFrameCount(UINT *pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFrame(UINT index, IWICBitmapFrameDecode **ppIBitmapFrame) = 0;
};

#if defined (IWICImagingFactory)
#undef IWICImagingFactory
#endif
#define IWICImagingFactory IWICImagingFactory_WRAP
struct IWICImagingFactory : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE CreateDecoderFromFilename(LPCWSTR wzFilename, const GUID *pguidVendor, DWORD dwDesiredAccess, WICDecodeOptions metadataOptions, IWICBitmapDecoder **ppIDecoder) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDecoderFromStream(IStream *pIStream, const GUID *pguidVendor, WICDecodeOptions metadataOptions, IWICBitmapDecoder **ppIDecoder) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDecoderFromFileHandle(ULONG_PTR hFile, const GUID *pguidVendor, WICDecodeOptions metadataOptions, IWICBitmapDecoder **ppIDecoder) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateComponentInfo(REFCLSID clsidComponent, IWICComponentInfo **ppIInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDecoder(REFGUID guidContainerFormat, const GUID *pguidVendor, IWICBitmapDecoder **ppIDecoder) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateEncoder(REFGUID guidContainerFormat, const GUID *pguidVendor, IWICBitmapEncoder **ppIEncoder) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreatePalette(IWICPalette **ppIPalette) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateFormatConverter(IWICFormatConverter **ppIFormatConverter) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmapScaler(IWICBitmapScaler **ppIBitmapScaler) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmapClipper(IWICBitmapClipper **ppIBitmapClipper) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmapFlipRotator(IWICBitmapFlipRotator **ppIBitmapFlipRotator) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateStream(IWICStream **ppIWICStream) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateColorContext(IWICColorContext **ppIWICColorContext) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateColorTransformer(IWICColorTransform **ppIWICColorTransform) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmap(UINT uiWidth, UINT uiHeight, REFWICPixelFormatGUID pixelFormat, WICBitmapCreateCacheOption option, IWICBitmap **ppIBitmap) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmapFromSource(IWICBitmapSource *pIBitmapSource, WICBitmapCreateCacheOption option, IWICBitmap **ppIBitmap) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmapFromSourceRect(IWICBitmapSource *pIBitmapSource, UINT x, UINT y, UINT width, UINT height, IWICBitmap **ppIBitmap) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmapFromMemory(UINT uiWidth, UINT uiHeight, REFWICPixelFormatGUID pixelFormat, UINT cbStride, UINT cbBufferSize, BYTE *pbBuffer, IWICBitmap **ppIBitmap) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmapFromHBITMAP(HBITMAP hBitmap, HPALETTE hPalette, WICBitmapAlphaChannelOption options, IWICBitmap **ppIBitmap) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateBitmapFromHICON(HICON hIcon, IWICBitmap **ppIBitmap) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateComponentEnumerator(DWORD componentTypes, DWORD options, IEnumUnknown **ppIEnumUnknown) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateFastMetadataEncoderFromDecoder(IWICBitmapDecoder *pIDecoder, IWICFastMetadataEncoder **ppIFastEncoder) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateFastMetadataEncoderFromFrameDecode(IWICBitmapFrameDecode *pIFrameDecoder, IWICFastMetadataEncoder **ppIFastEncoder) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateQueryWriter(REFGUID guidMetadataFormat, const GUID *pguidVendor, IWICMetadataQueryWriter **ppIQueryWriter) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateQueryWriterFromReader(IWICMetadataQueryReader *pIQueryReader, const GUID *pguidVendor, IWICMetadataQueryWriter **ppIQueryWriter) = 0;
};

// ====================================================================================================

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

private:
    OLE32()
        : m_CoInitialize(Q_NULLPTR)
        , m_CoUninitialize(Q_NULLPTR)
        , m_CoCreateInstance(Q_NULLPTR)
    {
        HMODULE library = ::LoadLibraryA("ole32");
        if(!library)
            return;

        m_CoInitialize = ::GetProcAddress(library, "CoInitialize");
        m_CoUninitialize = ::GetProcAddress(library, "CoUninitialize");
        m_CoCreateInstance = ::GetProcAddress(library, "CoCreateInstance");
    }

    ~OLE32()
    {}

    bool isValid() const
    {
        return m_CoInitialize && m_CoUninitialize && m_CoCreateInstance;
    }

    FARPROC m_CoInitialize;
    FARPROC m_CoUninitialize;
    FARPROC m_CoCreateInstance;
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

// ====================================================================================================

#if defined (IID_IWICImagingFactory)
#undef IID_IWICImagingFactory
#endif
#define IID_IWICImagingFactory IID_IWICImagingFactory_WRAP
const IID IID_IWICImagingFactory = { 0xec5ec8a9, 0xc395, 0x4314, { 0x9c, 0x77, 0x54, 0xd7, 0xa9, 0x35, 0xff, 0x70 } };

#if defined (IID_IWICBitmapCodecInfo)
#undef IID_IWICBitmapCodecInfo
#endif
#define IID_IWICBitmapCodecInfo IID_IWICBitmapCodecInfo_WRAP
const IID IID_IWICBitmapCodecInfo = { 0xe87a44c4, 0xb76e, 0x4c47, { 0x8b, 0x09, 0x29, 0x8e, 0xb1, 0x2a, 0x27, 0x14 } };

#if defined (CLSID_WICImagingFactory)
#undef CLSID_WICImagingFactory
#endif
#define CLSID_WICImagingFactory CLSID_WICImagingFactory_WRAP
const CLSID CLSID_WICImagingFactory = { 0xcacaf262, 0x9370, 0x4615, { 0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0x0a } };

#if defined (GUID_WICPixelFormat32bppBGRA)
#undef GUID_WICPixelFormat32bppBGRA
#endif
#define GUID_WICPixelFormat32bppBGRA GUID_WICPixelFormat32bppBGRA_WRAP
const GUID GUID_WICPixelFormat32bppBGRA = { 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0f } };

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
                LOG_DEBUG() << LOGGING_CTX << friendlyName << "-" << fileExtensions;

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
