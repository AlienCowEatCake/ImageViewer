/*
   Copyright (C) 2017-2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

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

#include "InfoUtils.h"

#include <cstring>

#include <QString>
#include <QSysInfo>

#include "Global.h"

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
#elif defined (Q_OS_HAIKU)
#include <AppFileInfo.h>
#include <FindDirectory.h>
#include <Path.h>
#elif !defined (Q_OS_MAC)
#include <sys/utsname.h>
#endif

namespace {

QString compilerDescriptionInt()
{
#if defined(__clang__)
    QString prefix;
#if defined (__apple_build_version__)
    prefix = QString::fromLatin1("Apple ");
#endif
    return QString::fromLatin1("%1Clang %2.%3.%4").arg(prefix).arg(__clang_major__).arg(__clang_minor__).arg(__clang_patchlevel__);
#elif defined (__GNUC__)
    QString prefix;
#if defined (__MINGW32__)
    prefix = QString::fromLatin1("MinGW ");
#elif defined (__CYGWIN__)
    prefix = QString::fromLatin1("Cygwin ");
#endif
    return QString::fromLatin1("%1GCC %2.%3.%4").arg(prefix).arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    switch (_MSC_VER)
    {
    // https://dev.to/yumetodo/list-of-mscver-and-mscfullver-8nd
    // https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering
    case 1100: return QString::fromLatin1("MSVC++ 5.0 (Visual Studio 97 version 5.0)");
    case 1200: return QString::fromLatin1("MSVC++ 6.0 (Visual Studio 6.0)");
    case 1300: return QString::fromLatin1("MSVC++ 7.0 (Visual Studio .NET 2002 version 7.0)");
    case 1310: return QString::fromLatin1("MSVC++ 7.1 (Visual Studio .NET 2003 version 7.1)");
    case 1400: return QString::fromLatin1("MSVC++ 8.0 (Visual Studio 2005 version 8.0)");
    case 1500: return QString::fromLatin1("MSVC++ 9.0 (Visual Studio 2008 version 9.0)");
    case 1600: return QString::fromLatin1("MSVC++ 10.0 (Visual Studio 2010 version 10.0)");
    case 1700: return QString::fromLatin1("MSVC++ 11.0 (Visual Studio 2012 version 11.0)");
    case 1800: return QString::fromLatin1("MSVC++ 12.0 (Visual Studio 2013 version 12.0)");
    case 1900: return QString::fromLatin1("MSVC++ 14.0 (Visual Studio 2015 version 14.0)");
    case 1910: return QString::fromLatin1("MSVC++ 14.1 (Visual Studio 2017 version 15.0)");
    case 1911: return QString::fromLatin1("MSVC++ 14.11 (Visual Studio 2017 version 15.3)");
    case 1912: return QString::fromLatin1("MSVC++ 14.12 (Visual Studio 2017 version 15.5)");
    case 1913: return QString::fromLatin1("MSVC++ 14.13 (Visual Studio 2017 version 15.6)");
    case 1914: return QString::fromLatin1("MSVC++ 14.14 (Visual Studio 2017 version 15.7)");
    case 1915: return QString::fromLatin1("MSVC++ 14.15 (Visual Studio 2017 version 15.8)");
    case 1916: return QString::fromLatin1("MSVC++ 14.16 (Visual Studio 2017 version 15.9)");
    case 1920: return QString::fromLatin1("MSVC++ 14.20 (Visual Studio 2019 version 16.0)");
    case 1921: return QString::fromLatin1("MSVC++ 14.21 (Visual Studio 2019 version 16.1)");
    case 1922: return QString::fromLatin1("MSVC++ 14.22 (Visual Studio 2019 version 16.2)");
    case 1923: return QString::fromLatin1("MSVC++ 14.23 (Visual Studio 2019 version 16.3)");
    case 1924: return QString::fromLatin1("MSVC++ 14.24 (Visual Studio 2019 version 16.4)");
    default:   return QString::fromLatin1("MSVC++ (_MSC_VER=%1)").arg(_MSC_VER);
    }
#elif defined (Q_CC_SYM)
    return QString::fromLatin1("Digital Mars C/C++");
#elif defined (Q_CC_INTEL)
    return QString::fromLatin1("Intel C++");
#elif defined (Q_CC_MSVC)
    return QString::fromLatin1("Microsoft Visual C/C++");
#elif defined (Q_CC_BOR)
    return QString::fromLatin1("Borland/Turbo C++");
#elif defined (Q_CC_WAT)
    return QString::fromLatin1("Watcom C++");
#elif defined (Q_CC_GNU)
    return QString::fromLatin1("GNU C++");
#elif defined (Q_CC_COMEAU)
    return QString::fromLatin1("Comeau C++");
#elif defined (Q_CC_EDG)
    return QString::fromLatin1("Edison Design Group C++");
#elif defined (Q_CC_OC)
    return QString::fromLatin1("CenterLine C++");
#elif defined (Q_CC_SUN)
    return QString::fromLatin1("Forte Developer, or Sun Studio C++");
#elif defined (Q_CC_MIPS)
    return QString::fromLatin1("MIPSpro C++");
#elif defined (Q_CC_DEC)
    return QString::fromLatin1("DEC C++");
#elif defined (Q_CC_HPACC)
    return QString::fromLatin1("HP aC++");
#elif defined (Q_CC_USLC)
    return QString::fromLatin1("SCO OUDK and UDK");
#elif defined (Q_CC_CDS)
    return QString::fromLatin1("Reliant C++");
#elif defined (Q_CC_KAI)
    return QString::fromLatin1("KAI C++");
#elif defined (Q_CC_HIGHC)
    return QString::fromLatin1("MetaWare High C/C++");
#elif defined (Q_CC_PGI)
    return QString::fromLatin1("Portland Group C++");
#elif defined (Q_CC_GHS)
    return QString::fromLatin1("Green Hills Optimizing C++ Compilers");
#elif defined (Q_CC_RVCT)
    return QString::fromLatin1("ARM Realview Compiler Suite");
#elif defined (Q_CC_CLANG)
    return QString::fromLatin1("C++ front-end for the LLVM compiler");
#else
    return QString::fromLatin1("Unknown");
#endif
}

QString targetDescriptionInt()
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
#elif defined (Q_PROCESSOR_ALPHA)
    return QString::fromLatin1("Alpha");
#elif defined (Q_PROCESSOR_ARM_V5)
    return QString::fromLatin1("ARM V5");
#elif defined (Q_PROCESSOR_ARM_V6)
    return QString::fromLatin1("ARM V6");
#elif defined (Q_PROCESSOR_ARM_V7)
    return QString::fromLatin1("ARM V7");
#elif defined (Q_PROCESSOR_ARM_V8)
    return QString::fromLatin1("ARM V8");
#elif defined (Q_PROCESSOR_ARM_64)
    return QString::fromLatin1("ARM64");
#elif defined (Q_PROCESSOR_ARM)
    return QString::fromLatin1("ARM");
#elif defined (Q_PROCESSOR_AVR32)
    return QString::fromLatin1("AVR32");
#elif defined (Q_PROCESSOR_BLACKFIN)
    return QString::fromLatin1("Blackfin");
#elif defined (Q_PROCESSOR_IA64)
    return QString::fromLatin1("IA-64");
#elif defined (Q_PROCESSOR_MIPS_I)
    return QString::fromLatin1("MIPS I");
#elif defined (Q_PROCESSOR_MIPS_II)
    return QString::fromLatin1("MIPS II");
#elif defined (Q_PROCESSOR_MIPS_III)
    return QString::fromLatin1("MIPS III");
#elif defined (Q_PROCESSOR_MIPS_IV)
    return QString::fromLatin1("MIPS IV");
#elif defined (Q_PROCESSOR_MIPS_V)
    return QString::fromLatin1("MIPS V");
#elif defined (Q_PROCESSOR_MIPS_64)
    return QString::fromLatin1("MIPS64");
#elif defined (Q_PROCESSOR_MIPS)
    return QString::fromLatin1("MIPS");
#elif defined (Q_PROCESSOR_POWER_64)
    return QString::fromLatin1("Power 64");
#elif defined (Q_PROCESSOR_POWER)
    return QString::fromLatin1("Power");
#elif defined (Q_PROCESSOR_S390_X)
    return QString::fromLatin1("S390X (64-bit)");
#elif defined (Q_PROCESSOR_S390)
    return QString::fromLatin1("S390");
#elif defined (Q_PROCESSOR_SH_4A)
    return QString::fromLatin1("SuperH SH-4A");
#elif defined (Q_PROCESSOR_SH)
    return QString::fromLatin1("SuperH");
#elif defined (Q_PROCESSOR_SPARC_V9)
    return QString::fromLatin1("SPARCv9");
#elif defined (Q_PROCESSOR_SPARC_64)
    return QString::fromLatin1("SPARC64");
#elif defined (Q_PROCESSOR_SPARC)
    return QString::fromLatin1("SPARC");
#elif defined (Q_PROCESSOR_X86_64)
    return QString::fromLatin1("x86-64");
#elif defined (Q_PROCESSOR_X86)
    return QString::fromLatin1("x86");
#else
    return QString();
#endif
}

} // namespace

namespace InfoUtils {

#if !defined (Q_OS_MAC)

/// @brief Проверить текущую версию macOS
bool MacVersionGreatOrEqual(const int major, const int minor, const int patch)
{
    Q_UNUSED(major);
    Q_UNUSED(minor);
    Q_UNUSED(patch);
    return false;
}

#endif

#if defined (Q_OS_WIN)

/// @brief Получить человеко-читаемую информацию о системе
QString GetSystemDescription()
{
    // https://stackoverflow.com/questions/2877295/get-os-in-c-win32-for-all-versions-of-win
    HMODULE hNetapi32 = LoadLibraryA("netapi32.dll");
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");

    SYSTEM_INFO sysInfo;
    memset(&sysInfo, 0, sizeof(sysInfo));
    if(hKernel32)
    {
        typedef void(WINAPI *GetNativeSystemInfo_t)(LPSYSTEM_INFO);
        GetNativeSystemInfo_t GetNativeSystemInfo_f = reinterpret_cast<GetNativeSystemInfo_t>(GetProcAddress(hKernel32, "GetNativeSystemInfo"));
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
        GetVersionExA_t GetVersionExA_f = reinterpret_cast<GetVersionExA_t>(GetProcAddress(hKernel32, "GetVersionExA"));
        typedef BOOL(WINAPI *GetVersionA_t)(LPOSVERSIONINFOA);
        GetVersionA_t GetVersionA_f = reinterpret_cast<GetVersionA_t>(GetProcAddress(hKernel32, "GetVersionA"));
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
            VerSetConditionMask_t VerSetConditionMask_f = reinterpret_cast<VerSetConditionMask_t>(GetProcAddress(hKernel32, "VerSetConditionMask"));
            typedef BOOL(WINAPI *VerifyVersionInfoA_t)(LPOSVERSIONINFOEXA, DWORD, DWORDLONG);
            VerifyVersionInfoA_t VerifyVersionInfoA_f = reinterpret_cast<VerifyVersionInfoA_t>(GetProcAddress(hKernel32, "VerifyVersionInfoA"));
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
        NetWkstaGetInfo_t NetWkstaGetInfo_f = reinterpret_cast<NetWkstaGetInfo_t>(GetProcAddress(hNetapi32, "NetWkstaGetInfo"));
        typedef NET_API_STATUS(NET_API_FUNCTION *NetApiBufferFree_t)(LPVOID);
        NetApiBufferFree_t NetApiBufferFree_f = reinterpret_cast<NetApiBufferFree_t>(GetProcAddress(hNetapi32, "NetApiBufferFree"));
        if(NetWkstaGetInfo_f && NetApiBufferFree_f)
        {
            LPBYTE pinfoRawData = Q_NULLPTR;
            if(NERR_Success == NetWkstaGetInfo_f(Q_NULLPTR, 100, &pinfoRawData) && pinfoRawData)
            {
                WKSTA_INFO_100* pworkstationInfo = reinterpret_cast<WKSTA_INFO_100*>(pinfoRawData);
                osMajorVersion = pworkstationInfo->wki100_ver_major;
                osMinorVersion = pworkstationInfo->wki100_ver_minor;
                NetApiBufferFree_f(pinfoRawData);
            }
        }
    }

    FreeLibrary(hNetapi32);
    FreeLibrary(hKernel32);

    QString winVersion;
    if     (osMajorVersion == 10 /*&& osMinorVersion >= 0*/ && osProductType != VER_NT_WORKSTATION)
        winVersion = QString::fromLatin1("Windows Server 2016");
    else if(osMajorVersion == 10 /*&& osMinorVersion >= 0*/ && osProductType == VER_NT_WORKSTATION)
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
}

#elif defined (Q_OS_HAIKU)

/// @brief Получить человеко-читаемую информацию о системе
QString GetSystemDescription()
{
    QString result = QString::fromLatin1("Haiku");

    // https://github.com/haiku/haiku/blob/r1beta1/src/apps/aboutsystem/AboutSystem.cpp#L437
    // the version is stored in the BEOS:APP_VERSION attribute of libbe.so
    BPath path;
    if(find_directory(B_BEOS_LIB_DIRECTORY, &path) == B_OK)
    {
        path.Append("libbe.so");
        BAppFileInfo appFileInfo;
        version_info versionInfo;
        BFile file;
        if(file.SetTo(path.Path(), B_READ_ONLY) == B_OK
                && appFileInfo.SetTo(&file) == B_OK
                && appFileInfo.GetVersionInfo(&versionInfo, B_APP_VERSION_KIND) == B_OK
                && versionInfo.short_info[0] != '\0')
            result += QString::fromLatin1(" ") + QString::fromLocal8Bit(versionInfo.short_info);
    }

    return result;
}

#elif !defined (Q_OS_MAC)

/// @brief Получить человеко-читаемую информацию о системе
QString GetSystemDescription()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    return QSysInfo::prettyProductName();
#else
    utsname buf;
    memset(&buf, 0, sizeof(buf));
    if(uname(&buf))
        return QString();
    return QString::fromLatin1("%1 %2, %3")
            .arg(QString::fromLocal8Bit(buf.sysname))
            .arg(QString::fromLocal8Bit(buf.release))
            .arg(QString::fromLocal8Bit(buf.machine));
#endif
}

#endif

/// @brief Получить человеко-читаемую информацию о компиляторе
QString GetCompilerDescription()
{
    QString description = compilerDescriptionInt();
    const QString target = targetDescriptionInt();
    if(!target.isEmpty())
    {
        const QString byteOrder = QString::fromLatin1(QSysInfo::ByteOrder == QSysInfo::BigEndian ? "BE" : "LE");
        description.append(QString::fromLatin1(", %1 (%2)").arg(target).arg(byteOrder));
    }
    return description;
}

} // namespace InfoUtils

