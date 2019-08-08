@echo off
set PROJECT=ImageViewer
set ARCH=x64
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
set QTDIR=C:\Qt\5.12.4\msvc2019_64_static
set BUILDDIR=build_win_qt5.12_msvc2019_%ARCH%
set SUFFIX=_qt5.12_msvc2019_%ARCH%
set APP_PATH=src\%PROJECT%
set ZIP_CMD=C:\MinGW\msys\1.0\bin\zip.exe

call %VCVARS% %ARCH%
set PATH=%QTDIR%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" QTPLUGIN.imageformats="qico qsvg qtiff" ..\%PROJECT%.pro
nmake
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

pause
