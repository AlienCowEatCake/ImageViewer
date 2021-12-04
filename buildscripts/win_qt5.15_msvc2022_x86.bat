@echo off
set PROJECT=ImageViewer
set ARCH=x86
set VCVARS_ARCH=x64_x86
set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
set QTDIR=C:\Qt\5.15.2\msvc2022_static
set BUILDDIR=build_win_qt5.15_msvc2022_%ARCH%
set SUFFIX=_qt5.15_msvc2022_%ARCH%
set APP_PATH=src\%PROJECT%
set ZIP_CMD=buildscripts\helpers\zip.exe
set WIXPY_CMD="C:\Program Files\WiX.Py-0.1\wix.py.exe"

call %VCVARS% %VCVARS_ARCH%
set PATH=%QTDIR%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" QTPLUGIN.imageformats="qico qsvg qtiff" CONFIG+="enable_update_checking" ..\%PROJECT%.pro
nmake
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
mkdir build_msi
copy %APP_PATH%\release\%PROJECT%.exe build_msi\%PROJECT%.exe
%WIXPY_CMD% ..\src\ImageViewer\resources\platform\windows\wixpy_%ARCH%.json
copy %PROJECT%.msi ..\%PROJECT%%SUFFIX%.msi
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

pause
