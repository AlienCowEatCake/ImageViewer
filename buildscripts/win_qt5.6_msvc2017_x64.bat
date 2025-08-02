@echo off
set PROJECT=ImageViewer
set ARCH=x64
set VCVARS_ARCH=x64
call "%~dp0\..\buildscripts\helpers\find_vcvarsall.bat" 2017
set VCVARS="%VS2017_VCVARSALL%"
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\5.6.3\msvc2017_64_static
set BUILDDIR=build_win_qt5.6_msvc2017_%ARCH%
set SUFFIX=_qt5.6_msvc2017_%ARCH%
set APP_PATH=src\%PROJECT%
set NMAKE_CMD="%~dp0\..\buildscripts\helpers\jom.exe" /J %NUMBER_OF_PROCESSORS%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') DO (
    if "%%i" == "PROCESSOR_ARCHITECTURE" if "%%~k" == "ARM64" (
        set NMAKE_CMD="%~dp0\..\buildscripts\helpers\arm64\jom.exe" /J %NUMBER_OF_PROCESSORS%
        set ZIP_CMD="%~dp0\..\buildscripts\helpers\arm64\zip.exe"
    )
)

call %VCVARS% %VCVARS_ARCH%
set PATH=%QT_PATH%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" QTPLUGIN.imageformats="qico qsvg qtiff" ..\%PROJECT%.pro
%NMAKE_CMD%
if not exist %APP_PATH%\release\%PROJECT%.exe (
    if NOT "%CI%" == "true" pause
    exit /b 1
)
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

if NOT "%CI%" == "true" pause
