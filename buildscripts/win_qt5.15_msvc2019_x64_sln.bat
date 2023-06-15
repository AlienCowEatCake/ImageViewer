@echo off
set PROJECT=ImageViewer
set ARCH=x64
call "%~dp0\..\buildscripts\helpers\find_vcvarsall.bat" 2019
set VCVARS="%VS2019_VCVARSALL%"
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\5.15.2\msvc2019_64
set BUILDDIR=build_win_qt5.15_msvc2019_%ARCH%_sln

call %VCVARS% %ARCH%
set PATH=%QT_PATH%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" CONFIG+="enable_update_checking" CONFIG+="enable_librsvg enable_resvg enable_magickwand enable_graphicsmagickwand enable_qtextended enable_qtwebkit enable_qtwebengine enable_qmlwebengine" -tp vc ..\%PROJECT%.pro

pause
