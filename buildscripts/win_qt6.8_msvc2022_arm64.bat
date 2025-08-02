@echo off
set PROJECT=ImageViewer
set ARCH=arm64
set VCVARS_ARCH=arm64
call "%~dp0\..\buildscripts\helpers\find_vcvarsall.bat" 2022
set VCVARS="%VS2022_VCVARSALL%"
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\6.8.3\msvc2022_arm64
set BUILDDIR=build_win_qt6.8_msvc2022_%ARCH%
set SUFFIX=_qt6.8_msvc2022_%ARCH%
set APP_PATH=src\%PROJECT%
set NMAKE_CMD="%~dp0\..\buildscripts\helpers\jom.exe" /J %NUMBER_OF_PROCESSORS%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
set DLLRESOLVER_CMD="%~dp0\..\buildscripts\helpers\dllresolver.exe"
set RESVG_PATH="%~dp0\resvg\aarch64-pc-windows-msvc"
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') DO (
    if "%%i" == "PROCESSOR_ARCHITECTURE" if "%%~k" == "ARM64" (
        set NMAKE_CMD="%~dp0\..\buildscripts\helpers\arm64\jom.exe" /J %NUMBER_OF_PROCESSORS%
        set ZIP_CMD="%~dp0\..\buildscripts\helpers\arm64\zip.exe"
        set DLLRESOLVER_CMD="%~dp0\..\buildscripts\helpers\arm64\dllresolver.exe"
    )
)

call %VCVARS% %VCVARS_ARCH%
set PATH=%QT_PATH%\bin;%WIX%\bin;%WIX%;%PATH%
set CRT_DIR="%VCToolsRedistDir%\arm64\Microsoft.VC143.CRT"
set UCRT_DIR="%UniversalCRTSdkDir%\Redist\%UCRTVersion%\ucrt\DLLs\arm64"

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
