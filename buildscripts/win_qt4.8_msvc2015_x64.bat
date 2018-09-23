@echo off
set PROJECT=ImageViewer
set ARCH=x64
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
set QTDIR=C:\Qt\4.8.7\msvc2015_64_static
set BUILDDIR=build_win_qt4.8_msvc2015_%ARCH%
set SUFFIX=_qt4.8_msvc2015_%ARCH%
set APP_PATH=src\%PROJECT%
set ZIP_CMD=C:\MinGW\msys\1.0\bin\zip.exe

call %VCVARS% %ARCH%
set PATH=%QTDIR%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" ..\%PROJECT%.pro
nmake
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

pause
