@echo off
set PROJECT=ImageViewer
set ARCH=x64
call "%~dp0\..\buildscripts\helpers\find_vcvarsall.bat" 2015
set VCVARS="%VS2015_VCVARSALL%"
set QTDIR=C:\Qt\4.8.7\msvc2015_64_static
set BUILDDIR=build_win_qt4.8_msvc2015_%ARCH%
set SUFFIX=_qt4.8_msvc2015_%ARCH%
set APP_PATH=src\%PROJECT%
set NMAKE_CMD="%~dp0\..\buildscripts\helpers\jom.exe" /J %NUMBER_OF_PROCESSORS%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"

call %VCVARS% %ARCH%
set PATH=%QTDIR%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" CONFIG+="disable_msedgewebview2" ..\%PROJECT%.pro
%NMAKE_CMD%
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

if NOT "%CI%" == "true" pause
