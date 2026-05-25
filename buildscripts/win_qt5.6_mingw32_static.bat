@echo off
set PROJECT=ImageViewer
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\5.6.3\tdmgcc1030-sjlj_static
if "x%MINGW_PATH%x" == "xx" set MINGW_PATH=C:\Qt\Tools\tdmgcc1030-sjlj_32
set BUILDDIR=build_win_qt5.6_tdmgcc1030-sjlj_static
set SUFFIX=_qt5.6_mingw32_static
set APP_PATH=src\%PROJECT%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') DO (
    if "%%i" == "PROCESSOR_ARCHITECTURE" if "%%~k" == "ARM64" (
        set ZIP_CMD="%~dp0\..\buildscripts\helpers\arm64\zip.exe"
    )
)

set PATH=%QT_PATH%\bin;%MINGW_PATH%\bin;%WINDIR%;%WINDIR%\System32

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" QTPLUGIN.imageformats="qico qsvg qtiff" ..\%PROJECT%.pro
mingw32-make -j%NUMBER_OF_PROCESSORS%
if not exist %APP_PATH%\release\%PROJECT%.exe (
    if NOT "%CI%" == "true" pause
    exit /b 1
)
strip --strip-all %APP_PATH%\release\%PROJECT%.exe
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

if NOT "%CI%" == "true" pause
