@echo off
set PROJECT=ImageViewer
set ARCH=x86
set VCVARS_ARCH=x64_x86
set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
set CRT_DIR="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.30.30704\x86\Microsoft.VC143.CRT"
set UCRT_DIR="C:\Program Files (x86)\Windows Kits\10\Redist\10.0.22000.0\ucrt\DLLs\x86"
set OPENSSL_DIR=C:\Qt\Tools\openssl-1.1.1m\openssl-1.1\x86\bin
set QTDIR=C:\Qt\5.15.2\msvc2019
set BUILDDIR=build_win_qt5.15_msvc2022_%ARCH%
set SUFFIX=_qt5.15_msvc2022_%ARCH%
set APP_PATH=src\%PROJECT%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
set WIXPY_CMD="C:\Program Files\WiX.Py-0.1\wix.py.exe"

call %VCVARS% %VCVARS_ARCH%
set PATH=%QTDIR%\bin;%PATH%

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" CONFIG+="enable_qtwebkit enable_update_checking" ..\%PROJECT%.pro
nmake
mkdir %PROJECT%%SUFFIX%
copy %APP_PATH%\release\%PROJECT%.exe %PROJECT%%SUFFIX%\%PROJECT%.exe
windeployqt --release --no-compiler-runtime --no-system-d3d-compiler --no-virtualkeyboard --no-angle --no-opengl-sw --translations en,ru %PROJECT%%SUFFIX%
copy %QTDIR%\bin\libxml2.dll %PROJECT%%SUFFIX%\
copy %QTDIR%\bin\libxslt.dll %PROJECT%%SUFFIX%\
copy %OPENSSL_DIR%\*.dll %PROJECT%%SUFFIX%\
copy %CRT_DIR%\*.dll %PROJECT%%SUFFIX%\
copy %UCRT_DIR%\*.dll %PROJECT%%SUFFIX%\
rmdir /S /Q %PROJECT%%SUFFIX%\position 2>nul >nul
rmdir /S /Q %PROJECT%%SUFFIX%\sensorgestures 2>nul >nul
rmdir /S /Q %PROJECT%%SUFFIX%\sensors 2>nul >nul
%ZIP_CMD% -9r ..\%PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%
move %PROJECT%%SUFFIX% build_msi
%WIXPY_CMD% ..\src\ImageViewer\resources\platform\windows\wixpy_%ARCH%.json
move %PROJECT%.msi ..\%PROJECT%%SUFFIX%.msi
cd ..

pause
