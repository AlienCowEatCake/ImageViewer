@echo off
set PROJECT=ImageViewer
set ARCH=x86
set VCVARS_ARCH=x64_x86
call "%~dp0\..\buildscripts\helpers\find_vcvarsall.bat" 2026
set VCVARS="%VS2026_VCVARSALL%"
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\6.9.3\msvc2022
set BUILDDIR=build_win_qt6_msvc2026_%ARCH%
set SUFFIX=_qt6_msvc2026_%ARCH%
set APP_PATH=src\%PROJECT%
set NMAKE_CMD="%~dp0\..\buildscripts\helpers\jom.exe" /J %NUMBER_OF_PROCESSORS%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
set DLLRESOLVER_CMD="%~dp0\..\buildscripts\helpers\dllresolver.exe"
set RESVG_PATH="%~dp0\resvg\resvg-0.45.1\i686-pc-windows-msvc"
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') DO (
    if "%%i" == "PROCESSOR_ARCHITECTURE" if "%%~k" == "ARM64" (
        set VCVARS_ARCH=arm64_x86
        set NMAKE_CMD="%~dp0\..\buildscripts\helpers\arm64\jom.exe" /J %NUMBER_OF_PROCESSORS%
        set ZIP_CMD="%~dp0\..\buildscripts\helpers\arm64\zip.exe"
        set DLLRESOLVER_CMD="%~dp0\..\buildscripts\helpers\arm64\dllresolver.exe"
    )
)

call %VCVARS% %VCVARS_ARCH%
set PATH=%QT_PATH%\bin;%WIX%\bin;%WIX%;%PATH%
set CRT_DIR="%VCToolsRedistDir%\x86\Microsoft.VC145.CRT"
set UCRT_DIR="%UniversalCRTSdkDir%\Redist\%UCRTVersion%\ucrt\DLLs\x86"

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" CONFIG+="hide_symbols" CONFIG+="enable_update_checking" CONFIG+="disable_embed_translations" CONFIG+="enable_qtcore5compat" CONFIG+="system_resvg" INCLUDEPATH+=%RESVG_PATH% LIBS+=/LIBPATH:%RESVG_PATH% DEFINES+="WINVER=0x0A00 _WIN32_WINNT=0x0A00" ..\%PROJECT%.pro
%NMAKE_CMD%
if not exist %APP_PATH%\release\%PROJECT%.exe (
    if NOT "%CI%" == "true" pause
    exit /b 1
)
rmdir /S /Q %PROJECT%%SUFFIX% 2>nul >nul
mkdir %PROJECT%%SUFFIX%
copy %APP_PATH%\release\%PROJECT%.exe %PROJECT%%SUFFIX%\%PROJECT%.exe
windeployqt --release --no-compiler-runtime --no-system-d3d-compiler --no-system-dxc-compiler --no-virtualkeyboard --no-opengl-sw %PROJECT%%SUFFIX%
%DLLRESOLVER_CMD% %PROJECT%%SUFFIX% %RESVG_PATH% %UCRT_DIR% %CRT_DIR% %QT_PATH%\bin
copy ..\src\%PROJECT%\resources\translations\*.qm %PROJECT%%SUFFIX%\translations\
copy ..\src\QtUtils\resources\translations\*.qm %PROJECT%%SUFFIX%\translations\
%ZIP_CMD% -9r ..\%PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%
rmdir /S /Q build_msi 2>nul >nul
move %PROJECT%%SUFFIX% build_msi
heat dir build_msi -cg ApplicationFiles -dr INSTALLLOCATION -gg -scom -sfrag -srd -sreg -svb6 -out appfiles.wxs
candle -out appfiles.wixobj appfiles.wxs -arch %ARCH%
candle -out common.wixobj ..\src\ImageViewer\resources\platform\windows\common.wxs -arch %ARCH%
candle -out main.wixobj ..\src\ImageViewer\resources\platform\windows\w10_%ARCH%.wxs -ext WixUIExtension -ext WixUtilExtension -arch %ARCH%
light -out %PROJECT%.msi -b build_msi main.wixobj appfiles.wixobj common.wixobj -ext WixUIExtension -ext WixUtilExtension -dcl:high -sice:ICE57
move %PROJECT%.msi ..\%PROJECT%%SUFFIX%.msi
cd ..

if NOT "%CI%" == "true" pause
