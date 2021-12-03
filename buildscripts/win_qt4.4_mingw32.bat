@echo off
set PROJECT=ImageViewer
set QTDIR=C:\Qt\4.4.3\mingw342_32_static
set MINGWDIR=C:\Qt\Tools\mingw342_32
set BUILDDIR=build_win_qt4.4_mingw342
set SUFFIX=_qt4.4_mingw32
set APP_PATH=src\%PROJECT%
set ZIP_CMD=buildscripts\helpers\zip.exe

set PATH=%QTDIR%\bin;%MINGWDIR%\bin;%MINGWDIR%\libexec\gcc\mingw32\3.4.2;%WINDIR%;%WINDIR%\System32
set CPLUS_INCLUDE_PATH=%MINGWDIR%\include\c++
set C_INCLUDE_PATH=%MINGWDIR%\include;%MINGWDIR%\lib\gcc\mingw32\3.4.2\include
set INCLUDE=%QTDIR%\include;%CPLUS_INCLUDE_PATH%;%C_INCLUDE_PATH%
set LIBRARY_PATH=%MINGWDIR%\lib;%MINGWDIR%\lib\gcc\mingw32\3.4.2
set LIB=%QTDIR%\lib\;%LIBRARY_PATH%
set QDIR=%QTDIR%
set QMAKESPEC=win32-g++

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" CONFIG+="use_static_qgif use_static_qtiff use_static_qjpeg use_static_qico use_static_qmng use_static_qsvg" CONFIG+="exceptions" QMAKE_CXXFLAGS_EXCEPTIONS_ON="-fexceptions" QMAKE_LFLAGS_EXCEPTIONS_ON="" DEFINES+="DISABLE_EXCEPTIONS_IN_THREADS" ..\%PROJECT%.pro
mingw32-make
strip --strip-all %APP_PATH%\release\%PROJECT%.exe
copy %APP_PATH%\release\%PROJECT%.exe ..\%PROJECT%%SUFFIX%.exe
cd ..
%ZIP_CMD% -9r %PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%.exe

pause
