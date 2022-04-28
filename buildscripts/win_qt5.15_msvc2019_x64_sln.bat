@echo off
set PROJECT=ImageViewer
set ARCH=x64
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
set QTDIR=C:\Qt\5.15.2\msvc2019_64
set BUILDDIR=build_win_qt5.15_msvc2019_%ARCH%_sln

call %VCVARS% %ARCH%
set PATH=%QTDIR%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" CONFIG+="enable_update_checking" CONFIG+="enable_librsvg enable_resvg enable_magickwand enable_graphicsmagickwand enable_qtextended enable_qtwebkit enable_qtwebengine enable_qmlwebengine" -tp vc ..\%PROJECT%.pro

pause
