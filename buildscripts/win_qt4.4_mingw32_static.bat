@echo off
set PROJECT=ImageViewer
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\4.4.3\mingw342_32_static
if "x%MINGW_PATH%x" == "xx" set MINGW_PATH=C:\Qt\Tools\mingw342_32
set BUILDDIR=build_win_qt4.4_mingw342_static
set SUFFIX=_qt4.4_mingw32_static
set APP_PATH=src\%PROJECT%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') DO (
    if "%%i" == "PROCESSOR_ARCHITECTURE" if "%%~k" == "ARM64" (
        set ZIP_CMD="%~dp0\..\buildscripts\helpers\arm64\zip.exe"
    )
)

set PATH=%QT_PATH%\bin;%MINGW_PATH%\bin;%MINGW_PATH%\mingw32\bin;%MINGW_PATH%\libexec\gcc\mingw32\3.4.2;%WINDIR%;%WINDIR%\System32
set CPLUS_INCLUDE_PATH=%MINGW_PATH%\include\c++
set C_INCLUDE_PATH=%MINGW_PATH%\include;%MINGW_PATH%\lib\gcc\mingw32\3.4.2\include
set INCLUDE=%QT_PATH%\include;%CPLUS_INCLUDE_PATH%;%C_INCLUDE_PATH%
set LIBRARY_PATH=%MINGW_PATH%\lib;%MINGW_PATH%\lib\gcc\mingw32\3.4.2
set LIB=%QT_PATH%\lib\;%LIBRARY_PATH%
set QTDIR=%QT_PATH%
set QDIR=%QT_PATH%
set QMAKESPEC=win32-g++
set QMAKE_CC=mingw32-gcc.exe
set QMAKE_CXX=mingw32-g++.exe
set QMAKE_LINK=mingw32-g++.exe

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r QMAKE_CC="%QMAKE_CC%" QMAKE_CXX="%QMAKE_CXX%" QMAKE_LINK="%QMAKE_LINK%" CONFIG+="release" CONFIG+="use_static_qgif use_static_qtiff use_static_qjpeg use_static_qico use_static_qmng use_static_qsvg" CONFIG+="exceptions" QMAKE_CXXFLAGS_EXCEPTIONS_ON="-fexceptions" QMAKE_LFLAGS_EXCEPTIONS_ON="" DEFINES+="DISABLE_EXCEPTIONS_IN_THREADS" ..\%PROJECT%.pro
mingw32-make
if not exist %APP_PATH%\release\%PROJECT%.exe (
    if NOT "%CI%" == "true" pause
    exit /b 1
)
strip --strip-all %APP_PATH%\release\%PROJECT%.exe
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

if NOT "%CI%" == "true" pause
