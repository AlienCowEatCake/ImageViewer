@echo off
set PROJECT=ImageViewer
set ARCH=x64
set VCVARS_ARCH=x64
set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
set OPENSSL_DIR=C:\Qt\Tools\openssl-1.1.1q\openssl-1.1\x64\bin
set QTDIR=C:\Qt\5.15.2\msvc2019_64
set BUILDDIR=build_win_qt5.15_msvc2022_%ARCH%
set SUFFIX=_qt5.15_msvc2022_%ARCH%
set APP_PATH=src\%PROJECT%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
set WEBVIEW2LOADER_DLL="%~dp0\..\src\ThirdParty\MSEdgeWebView2\microsoft.web.webview2.1.0.1293.44\build\native\%ARCH%\WebView2Loader.dll"

call %VCVARS% %VCVARS_ARCH%
set PATH=%QTDIR%\bin;%WIX%\bin;%PATH%
set CRT_DIR="%VCToolsRedistDir%\x64\Microsoft.VC143.CRT"
set UCRT_DIR="%UniversalCRTSdkDir%\Redist\%UCRTVersion%\ucrt\DLLs\x64"

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
copy %WEBVIEW2LOADER_DLL% %PROJECT%%SUFFIX%\
copy %QTDIR%\bin\libxml2.dll %PROJECT%%SUFFIX%\
copy %QTDIR%\bin\libxslt.dll %PROJECT%%SUFFIX%\
copy %OPENSSL_DIR%\*.dll %PROJECT%%SUFFIX%\
copy %CRT_DIR%\*.dll %PROJECT%%SUFFIX%\
copy %UCRT_DIR%\*.dll %PROJECT%%SUFFIX%\
rmdir /S /Q %PROJECT%%SUFFIX%\position 2>nul >nul
rmdir /S /Q %PROJECT%%SUFFIX%\sensorgestures 2>nul >nul
rmdir /S /Q %PROJECT%%SUFFIX%\sensors 2>nul >nul
%ZIP_CMD% -9r ..\%PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%
rmdir /S /Q build_msi 2>nul >nul
move %PROJECT%%SUFFIX% build_msi
heat dir build_msi -cg ApplicationFiles -dr INSTALLLOCATION -gg -scom -sfrag -srd -sreg -svb6 -out appfiles.wxs
candle -out appfiles.wixobj appfiles.wxs -arch %ARCH%
candle -out common.wixobj ..\src\ImageViewer\resources\platform\windows\common.wxs -arch %ARCH%
candle -out main.wixobj ..\src\ImageViewer\resources\platform\windows\w7_%ARCH%.wxs -ext WixUIExtension -ext WixUtilExtension -arch %ARCH%
light -out %PROJECT%.msi -b build_msi main.wixobj appfiles.wixobj common.wixobj -ext WixUIExtension -ext WixUtilExtension -dcl:high
move %PROJECT%.msi ..\%PROJECT%%SUFFIX%.msi
cd ..

pause
