@echo off
set PROJECT=ImageViewer
set ARCH=x64
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
set QTDIR=C:\Qt\5.12.5\msvc2019_64_static
set BUILDDIR=build_win_qt5.12_msvc2019_%ARCH%_sln

call %VCVARS% %ARCH%
set PATH=%QTDIR%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" -tp vc ..\%PROJECT%.pro

pause
