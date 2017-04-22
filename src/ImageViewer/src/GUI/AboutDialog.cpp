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

#include "AboutDialog.h"
#include "AboutDialog_p.h"

#include <cstring>

#include <QApplication>
#include <QSysInfo>

#include "Decoders/DecodersManager.h"

#if defined (HAS_LIBPNG)
#include <png.h>
#endif
#if defined (HAS_LIBJPEG)
#include <jpeglib.h>
#endif
#if defined (HAS_LIBJASPER)
#include <jasper/jasper.h>
#endif
#if defined (HAS_LIBMNG)
#include <libmng.h>
#endif
#if defined (HAS_LCMS2)
#include <lcms2.h>
#endif
#if defined (HAS_ZLIB)
#include <zlib.h>
#endif
#if defined (HAS_POSHLIB)
#include "posh.h"
#endif

#if defined (Q_OS_WIN)
#include <windows.h>
#include <lm.h>
#if !defined (SM_TABLETPC)
#define SM_TABLETPC 86
#endif
#if !defined (SM_MEDIACENTER)
#define SM_MEDIACENTER 87
#endif
#if !defined (SM_STARTER)
#define SM_STARTER 88
#endif
#if !defined (SM_SERVERR2)
#define SM_SERVERR2 89
#endif
#if !defined (VER_SUITE_WH_SERVER)
#define VER_SUITE_WH_SERVER 0x00008000
#endif
#if !defined (VER_MINORVERSION)
#define VER_MINORVERSION 0x0000001
#endif
#if !defined (VER_EQUAL)
#define VER_EQUAL 1
#endif
#else
#include <sys/utsname.h>
#endif

namespace {

const char *ABOUT_PIXMAP_PATH     = ":/icon/icon_64.png";
const char *ABOUT_URL_STRING      = "https://fami.codefreak.ru/gitlab/peter/ImageViewer";
const char *ABOUT_LICENSE_STRIG   = "<a href=\"http://www.gnu.org/copyleft/gpl.html\">GNU GPL v3</a>";
const char *ABOUT_YEAR_STRING     = "2017";
const char *ABOUT_AUTHOR_STRING   = QT_TRANSLATE_NOOP("AboutDialog", "Peter S. Zhigalov");
const char *ABOUT_EMAIL_STRING    = "peter.zhigalov@gmail.com";

QString letterByNumFrom0(int num)
{
    static const QString alphabet = QString::fromLatin1("abcdefghijklmnopqrstuvwxyz");
    if(num < 0 || num > 26)
        return QString();
    return QString(alphabet[num]);
}

QString letterByNumFrom1(int num)
{
    return letterByNumFrom0(num - 1);
}

QString systemDescription()
{
#if defined (Q_OS_WIN)
    // https://stackoverflow.com/questions/2877295/get-os-in-c-win32-for-all-versions-of-win
    HMODULE hNetapi32 = LoadLibraryA("netapi32.dll");
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");

    SYSTEM_INFO sysInfo;
    memset(&sysInfo, 0, sizeof(sysInfo));
    if(hKernel32)
    {
        typedef void(WINAPI *GetNativeSystemInfo_t)(LPSYSTEM_INFO);
        GetNativeSystemInfo_t GetNativeSystemInfo_f = (GetNativeSystemInfo_t)GetProcAddress(hKernel32, "GetNativeSystemInfo");
        if(!GetNativeSystemInfo_f)
            GetNativeSystemInfo_f = &GetSystemInfo;
        GetNativeSystemInfo_f(&sysInfo);
    }

    DWORD osMajorVersion = 0;
    DWORD osMinorVersion = 0;
    WORD osProductType = VER_NT_WORKSTATION;
    WORD osServicePack = 0;
    WORD osSuiteMask = 0;
    DWORD osPlatform = VER_PLATFORM_WIN32_NT;
    QString osCSDVersion;
    if(hKernel32)
    {
        typedef BOOL(WINAPI *GetVersionExA_t)(LPOSVERSIONINFOEXA);
        GetVersionExA_t GetVersionExA_f = (GetVersionExA_t)GetProcAddress(hKernel32, "GetVersionExA");
        typedef BOOL(WINAPI *GetVersionA_t)(LPOSVERSIONINFOA);
        GetVersionA_t GetVersionA_f = (GetVersionA_t)GetProcAddress(hKernel32, "GetVersionA");
        bool exStatus = false;
        if(GetVersionExA_f)
        {
            OSVERSIONINFOEXA osver;
            memset(&osver, 0, sizeof(osver));
            osver.dwOSVersionInfoSize = sizeof(osver);
            if(GetVersionExA_f(&osver))
            {
                osMajorVersion = osver.dwMajorVersion;
                osMinorVersion = osver.dwMinorVersion;
                osProductType = osver.wProductType;
                osServicePack = osver.wServicePackMajor;
                osSuiteMask = osver.wSuiteMask;
                osPlatform = osver.dwPlatformId;
                osCSDVersion = QString::fromLocal8Bit(osver.szCSDVersion).simplified();
                exStatus = true;
            }
        }
        if(!exStatus && GetVersionA_f)
        {
            OSVERSIONINFOA osver;
            memset(&osver, 0, sizeof(osver));
            osver.dwOSVersionInfoSize = sizeof(osver);
            if(GetVersionA_f(&osver))
            {
                osMajorVersion = osver.dwMajorVersion;
                osMinorVersion = osver.dwMinorVersion;
                osPlatform = osver.dwPlatformId;
                osCSDVersion = QString::fromLocal8Bit(osver.szCSDVersion).simplified();
            }
        }
        if(osMajorVersion == 6 && osMinorVersion == 2)
        {
            typedef ULONGLONG(WINAPI *VerSetConditionMask_t)(ULONGLONG, DWORD, BYTE);
            VerSetConditionMask_t VerSetConditionMask_f = (VerSetConditionMask_t)GetProcAddress(hKernel32, "VerSetConditionMask");
            typedef BOOL(WINAPI *VerifyVersionInfoA_t)(LPOSVERSIONINFOEXA, DWORD, DWORDLONG);
            VerifyVersionInfoA_t VerifyVersionInfoA_f = (VerifyVersionInfoA_t)GetProcAddress(hKernel32, "VerifyVersionInfoA");
            if(VerSetConditionMask_f && VerifyVersionInfoA_f)
            {
                OSVERSIONINFOEXA osvi;
                ULONGLONG cm = 0;
                cm = VerSetConditionMask_f(cm, VER_MINORVERSION, VER_EQUAL);
                memset(&osvi, 0, sizeof(osvi));
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                osvi.dwMinorVersion = 3;
                if(VerifyVersionInfoA_f(&osvi, VER_MINORVERSION, cm))
                    osMinorVersion = 3;
            }
        }
    }

    if(hNetapi32)
    {
        typedef NET_API_STATUS(NET_API_FUNCTION *NetWkstaGetInfo_t)(LPWSTR, DWORD, LPBYTE*);
        NetWkstaGetInfo_t NetWkstaGetInfo_f = (NetWkstaGetInfo_t)GetProcAddress(hNetapi32, "NetWkstaGetInfo");
        typedef NET_API_STATUS(NET_API_FUNCTION *NetApiBufferFree_t)(LPVOID);
        NetApiBufferFree_t NetApiBufferFree_f = (NetApiBufferFree_t)GetProcAddress(hNetapi32, "NetApiBufferFree");
        if(NetWkstaGetInfo_f && NetApiBufferFree_f)
        {
            LPBYTE pinfoRawData = NULL;
            if(NERR_Success == NetWkstaGetInfo_f(NULL, 100, &pinfoRawData) && pinfoRawData)
            {
                WKSTA_INFO_100* pworkstationInfo = (WKSTA_INFO_100*)pinfoRawData;
                osMajorVersion = pworkstationInfo->wki100_ver_major;
                osMinorVersion = pworkstationInfo->wki100_ver_minor;
                NetApiBufferFree_f(pinfoRawData);
            }
        }
    }

    FreeLibrary(hNetapi32);
    FreeLibrary(hKernel32);

    QString winVersion;
    if     (osMajorVersion == 10 && osMinorVersion >= 0 && osProductType != VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows 10 Server");
    else if(osMajorVersion == 10 && osMinorVersion >= 0 && osProductType == VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows 10");
    else if(osMajorVersion == 6 && osMinorVersion == 3 && osProductType != VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows Server 2012 R2");
    else if(osMajorVersion == 6 && osMinorVersion == 3 && osProductType == VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows 8.1");
    else if(osMajorVersion == 6 && osMinorVersion == 2 && osProductType != VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows Server 2012");
    else if(osMajorVersion == 6 && osMinorVersion == 2 && osProductType == VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows 8");
    else if(osMajorVersion == 6 && osMinorVersion == 1 && osProductType != VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows Server 2008 R2");
    else if(osMajorVersion == 6 && osMinorVersion == 1 && osProductType == VER_NT_WORKSTATION && GetSystemMetrics(SM_STARTER))
        winVersion = QString::fromLatin1("Windows 7 Starter Edition");
    else if(osMajorVersion == 6 && osMinorVersion == 1 && osProductType == VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows 7");
    else if(osMajorVersion == 6 && osMinorVersion == 0 && osProductType != VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows Server 2008");
    else if(osMajorVersion == 6 && osMinorVersion == 0 && osProductType == VER_NT_WORKSTATION && GetSystemMetrics(SM_STARTER))
        winVersion = QString::fromLatin1("Windows Vista Starter");
    else if(osMajorVersion == 6 && osMinorVersion == 0 && osProductType == VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows Vista");
    else if(osMajorVersion == 5 && osMinorVersion == 2 && osSuiteMask & VER_SUITE_WH_SERVER)
        winVersion = QString::fromLatin1("Windows Home Server");
    else if(osMajorVersion == 5 && osMinorVersion == 2 && osProductType == VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows XP Professional x64 Edition");
    else if(osMajorVersion == 5 && osMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2))
        winVersion = QString::fromLatin1("Windows Server 2003 R2");
    else if(osMajorVersion == 5 && osMinorVersion == 2)
        winVersion = QString::fromLatin1("Windows Server 2003");
    else if(osMajorVersion == 5 && osMinorVersion == 1 && GetSystemMetrics(SM_MEDIACENTER))
        winVersion = QString::fromLatin1("Windows XP Media Center Edition");
    else if(osMajorVersion == 5 && osMinorVersion == 1 && GetSystemMetrics(SM_STARTER))
        winVersion = QString::fromLatin1("Windows XP Starter Edition");
    else if(osMajorVersion == 5 && osMinorVersion == 1 && GetSystemMetrics(SM_TABLETPC))
        winVersion = QString::fromLatin1("Windows XP Tablet PC Edition");
    else if(osMajorVersion == 5 && osMinorVersion == 1)
        winVersion = QString::fromLatin1("Windows XP");
    else if(osMajorVersion == 5 && osMinorVersion == 0)
        winVersion = QString::fromLatin1("Windows 2000");
    else if(osMajorVersion == 4 && osMinorVersion == 90)
        winVersion = QString::fromLatin1("Windows ME");
    else if(osMajorVersion == 4 && osMinorVersion == 10)
        winVersion = QString::fromLatin1("Windows 98") + (osCSDVersion.isEmpty() ? QString() : QString::fromLatin1(" SE"));
    else if(osMajorVersion == 4 && osMinorVersion == 0 && osPlatform == VER_PLATFORM_WIN32_NT)
        winVersion = QString::fromLatin1("Windows NT 4.0") + (osCSDVersion.isEmpty() ? QString() : QString::fromLatin1(" %1").arg(osCSDVersion));
    else if(osMajorVersion == 4 && osMinorVersion == 0)
        winVersion = QString::fromLatin1("Windows 95");
    else
        winVersion = QString::fromLatin1("Windows %1.%2").arg(osMajorVersion).arg(osMinorVersion);

    if(osMajorVersion > 5 || (osMajorVersion == 5 && osMinorVersion == 2  && osProductType != VER_NT_WORKSTATION))
    {
        switch(sysInfo.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_INTEL:
            winVersion.append(QString::fromLatin1(" x86"));
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            winVersion.append(QString::fromLatin1(" x64"));
            break;
        default:
            break;
        }
    }

    if(osServicePack)
        winVersion.append(QString::fromLatin1(" Service Pack %1").arg(osServicePack));

    return winVersion;
#else
    utsname buf;
    memset(&buf, 0, sizeof(buf));
    if(uname(&buf))
        return QString();
    return QString::fromLatin1("%1 %2 %3 %4 %5")
            .arg(QString::fromLocal8Bit(buf.sysname))
            .arg(QString::fromLocal8Bit(buf.nodename))
            .arg(QString::fromLocal8Bit(buf.release))
            .arg(QString::fromLocal8Bit(buf.version))
            .arg(QString::fromLocal8Bit(buf.machine));
#endif
}

QString compilerDescription()
{
#if defined(__clang__)
    return QString::fromLatin1("Clang %1.%2.%3").arg(__clang_major__).arg(__clang_minor__).arg(__clang_patchlevel__);
#elif defined (__GNUC__)
    QString prefix;
#if defined (__MINGW32__)
    prefix = QString::fromLatin1("MinGW");
#elif defined (__CYGWIN__)
    prefix = QString::fromLatin1("Cygwin");
#endif
    return QString::fromLatin1("%1 GCC %2.%3.%4").arg(prefix).arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    switch (_MSC_VER)
    {
    case 1100: return QString::fromLatin1("MSVC++ 5.0 (Visual Studio 97)");
    case 1200: return QString::fromLatin1("MSVC++ 6.0 (Visual Studio 6.0)");
    case 1300: return QString::fromLatin1("MSVC++ 7.0 (Visual Studio .NET 2002)");
    case 1310: return QString::fromLatin1("MSVC++ 7.1 (Visual Studio .NET 2003)");
    case 1400: return QString::fromLatin1("MSVC++ 8.0 (Visual Studio 2005)");
    case 1500: return QString::fromLatin1("MSVC++ 9.0 (Visual Studio 2008)");
    case 1600: return QString::fromLatin1("MSVC++ 10.0 (Visual Studio 2010)");
    case 1700: return QString::fromLatin1("MSVC++ 11.0 (Visual Studio 2012)");
    case 1800: return QString::fromLatin1("MSVC++ 12.0 (Visual Studio 2013)");
    case 1900: return QString::fromLatin1("MSVC++ 14.0 (Visual Studio 2015)");
    case 1910: return QString::fromLatin1("MSVC++ 14.1 (Visual Studio 2017)");
    default:   return QString::fromLatin1("MSVC++");
    }
#elif defined (HAS_POSHLIB)
    return QString::fromLatin1(POSH_COMPILER_STRING);
#else
    return QString::fromLatin1("Unknown");
#endif
}

QString targetDescription()
{
#if (defined(__ILP32__) && defined(__x86_64__))
    return QString::fromLatin1("x86-32");
#elif (defined(_M_IX86 ) || defined(__X86__ ) || defined(__i386  ) || \
    defined(__IA32__) || defined(__I86__ ) || defined(__i386__) || \
    defined(__i486__) || defined(__i586__) || defined(__i686__))
    return QString::fromLatin1("x86");
#elif (defined(_M_X64  ) || defined(__x86_64) || defined(__x86_64__) || \
    defined(_M_AMD64) || defined(__amd64 ) || defined(__amd64__ ))
    return QString::fromLatin1("x86-64");
#elif defined (HAS_POSHLIB)
    return QString::fromLatin1(POSH_CPU_STRING);
#else
    return QString::fromLatin1("Unknown");
#endif
}

QString formatItem(const QString &title, const QString &name, const QString &version, const QString &url)
{
    QString result;
    if(!title.isEmpty())
        result.append(QString::fromLatin1("<i>%1</i><br>").arg(title));
    result.append(QString::fromLatin1("<b>%1</b>").arg(name));
    if(!version.isEmpty())
        result.append(QString::fromLatin1(", version %1").arg(version));
    if(!url.isEmpty())
        result.append(QString::fromLatin1("<br><a href=\"%1\">%1</a>").arg(url));
    result.append(QString::fromLatin1("<br><br>"));
    return result;
}

QString getTextBrowserContent()
{
    QString result;

    result.append(QString::fromLatin1("<b>%1</b> %2<br>")
                  .arg(QString::fromLatin1("System:"))
                  .arg(systemDescription())
                  );

    result.append(QString::fromLatin1("<b>%1</b> %2, %3<br>")
                  .arg(QString::fromLatin1("Compiler:"))
                  .arg(compilerDescription())
                  .arg(targetDescription())
                  );

    QStringList availableDecoders = DecodersManager::getInstance().registeredDecoders();
    for(QStringList::Iterator it = availableDecoders.begin(); it != availableDecoders.end(); ++it)
    {
        const QString decoderPrefix = QString::fromLatin1("Decoder");
        if(it->startsWith(decoderPrefix))
            *it = it->mid(decoderPrefix.length()).toLower();
        else
            *it = it->toLower();
    }
    availableDecoders.sort();

    result.append(QString::fromLatin1("<b>%1</b> %2<br><br>")
            .arg(QString::fromLatin1("Available image decoders:"))
            .arg(availableDecoders.join(QString::fromLatin1(", "))));

    result.append(formatItem(
                      QString::fromLatin1("This software uses the Qt framework"),
                      QString::fromLatin1("qt"),
                      QString::fromLatin1(QT_VERSION_STR),
                      QString::fromLatin1("https://www.qt.io/")
                      ));

#if defined (HAS_LIBPNG)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the PNG reference library"),
                      QString::fromLatin1("libpng"),
                      QString::fromLatin1("%1.%2.%3").arg(PNG_LIBPNG_VER_MAJOR).arg(PNG_LIBPNG_VER_MINOR).arg(PNG_LIBPNG_VER_RELEASE),
                      QString::fromLatin1("http://www.libpng.org/pub/png/libpng.html")
                      ));
#if defined (PNG_APNG_SUPPORTED)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the APNG patch for libpng"),
                      QString::fromLatin1("libpng-apng"),
                      QString::fromLatin1("%1.%2.%3").arg(PNG_LIBPNG_VER_MAJOR).arg(PNG_LIBPNG_VER_MINOR).arg(PNG_LIBPNG_VER_RELEASE),
                      QString::fromLatin1("https://sourceforge.net/projects/libpng-apng/")
                      ));
#endif
#endif

#if defined (HAS_LIBJPEG)
    result.append(formatItem(
                      QString::fromLatin1("This software is based in part on the work of the Independent JPEG Group"),
                      QString::fromLatin1("libjpeg"),
                      QString::number(JPEG_LIB_VERSION_MAJOR) + letterByNumFrom1(JPEG_LIB_VERSION_MINOR),
                      QString::fromLatin1("http://www.ijg.org/")
                      ));
#else
    Q_UNUSED(letterByNumFrom1);
#endif

#if defined (HAS_LIBJASPER)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the JasPer image processing/coding tool kit"),
                      QString::fromLatin1("libjasper"),
                      strcmp(JAS_VERSION, "unknown") ? QString::fromLatin1(JAS_VERSION) : QString(),
                      QString::fromLatin1("https://www.ece.uvic.ca/~frodo/jasper/")
                      ));
#endif

#if defined (HAS_LIBMNG)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the Multiple-image Network Graphics (MNG) reference library"),
                      QString::fromLatin1("libmng"),
                      QString::fromLatin1("%1.%2.%3").arg(MNG_VERSION_MAJOR).arg(MNG_VERSION_MINOR).arg(MNG_VERSION_RELEASE),
                      QString::fromLatin1("https://sourceforge.net/projects/libmng/")
                      ));
#endif

#if defined (HAS_LIBEXIF)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the libexif C EXIF library"),
                      QString::fromLatin1("libexif"),
                      QString(),
                      QString::fromLatin1("http://libexif.sourceforge.net/")
                      ));
#endif

#if defined (HAS_LCMS2)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the Little CMS 2 library"),
                      QString::fromLatin1("lcms2"),
                      QString::fromLatin1("%1.%2").arg(LCMS_VERSION / 1000).arg(LCMS_VERSION % 100 / 10),
                      QString::fromLatin1("http://www.littlecms.com/")
                      ));
#endif

#if defined (HAS_ZLIB)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the zlib library"),
                      QString::fromLatin1("zlib"),
                      QString::fromLatin1(ZLIB_VERSION),
                      QString::fromLatin1("http://www.zlib.net/")
                      ));
#endif

#if defined (HAS_QTEXTENDED)
    result.append(formatItem(
                      QString::fromLatin1("This software uses part of the Qt Extended library"),
                      QString::fromLatin1("qt-extended"),
                      QString::fromLatin1("4.4.3"),
                      QString::fromLatin1("https://sourceforge.net/projects/qpe/files/QPE/qtopia/")
                      ));
#endif

#if defined (HAS_QTIMAGEFORMATS)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the additional Image Format plugins for Qt"),
                      QString::fromLatin1("qtimageformats"),
                      QString(),
                      QString::fromLatin1("https://github.com/qt/qtimageformats")
                      ));
#endif

#if defined (HAS_STB)
    result.append(formatItem(
                      QString::fromLatin1("This software uses part of the STB library"),
                      QString::fromLatin1("stb"),
                      QString(),
                      QString::fromLatin1("https://github.com/nothings/stb")
                      ));
#endif

#if defined (HAS_POSHLIB)
    result.append(formatItem(
                      QString::fromLatin1("This software uses the Portable Open Source Harness library"),
                      QString::fromLatin1("poshlib"),
                      QString(),
                      QString::fromLatin1("http://hookatooka.com/poshlib/")
                      ));
#endif

    return result;
}

} // namespace

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new UI(this))
{
    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(close()));

    m_ui->iconLabel->setPixmap(qApp->translate("AboutDialog", ABOUT_PIXMAP_PATH));
    setWindowTitle(qApp->translate("AboutDialog", "About"));
    m_ui->textLabel->setText(QString::fromLatin1(
                                 "<h3>%1 v%2</h3>"
                                 "<a href=\"%3\">%3</a><br>"
                                 "%4: %5<br><br>"
                                 "Copyright &copy; %6<br>"
                                 "%7 &lt;<a href=\"mailto:%8\">%8</a>&gt;"
                                 )
                             .arg(qApp->applicationName())
                             .arg(qApp->applicationVersion())
                             .arg(qApp->translate("AboutDialog", ABOUT_URL_STRING))
                             .arg(qApp->translate("AboutDialog", "License"))
                             .arg(qApp->translate("AboutDialog", ABOUT_LICENSE_STRIG))
                             .arg(qApp->translate("AboutDialog", ABOUT_YEAR_STRING))
                             .arg(qApp->translate("AboutDialog", ABOUT_AUTHOR_STRING))
                             .arg(qApp->translate("AboutDialog", ABOUT_EMAIL_STRING))
                             );
    m_ui->textBrowser->setHtml(getTextBrowserContent());

    adjustSize();
    setFixedSize(minimumSize());
}

AboutDialog::~AboutDialog()
{}

void AboutDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    m_ui->textBrowser->setFixedWidth(width() - m_ui->centralWidget->layoutMarginLeft() - m_ui->centralWidget->layoutMarginRight());
}
