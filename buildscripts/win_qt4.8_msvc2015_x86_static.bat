@echo off
set PROJECT=ImageViewer
set ARCH=x86
call "%~dp0\..\buildscripts\helpers\find_vcvarsall.bat" 2015
set VCVARS="%VS2015_VCVARSALL%"
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\4.8.7\msvc2015_static
set BUILDDIR=build_win_qt4.8_msvc2015_%ARCH%_static
set SUFFIX=_qt4.8_msvc2015_%ARCH%_static
set APP_PATH=src\%PROJECT%
set NMAKE_CMD="%~dp0\..\buildscripts\helpers\jom.exe" /J %NUMBER_OF_PROCESSORS%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') DO (
    if "%%i" == "PROCESSOR_ARCHITECTURE" if "%%~k" == "ARM64" (
        set NMAKE_CMD="%~dp0\..\buildscripts\helpers\arm64\jom.exe" /J %NUMBER_OF_PROCESSORS%
        set ZIP_CMD="%~dp0\..\buildscripts\helpers\arm64\zip.exe"
    )
)

call %VCVARS% %ARCH%
set PATH=%QT_PATH%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" ..\%PROJECT%.pro
%NMAKE_CMD%
if not exist %APP_PATH%\release\%PROJECT%.exe (
    if NOT "%CI%" == "true" pause
    exit /b 1
)
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

if NOT "%CI%" == "true" pause
