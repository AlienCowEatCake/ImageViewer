@echo off
set PROJECT=ImageViewer
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\4.8.7\tdmgcc510-sjlj
if "x%MINGW_PATH%x" == "xx" set MINGW_PATH=C:\Qt\Tools\tdmgcc510-sjlj_32
set BUILDDIR=build_win_qt4.8_tdmgcc510-sjlj
set SUFFIX=_qt4.8_mingw32
set APP_PATH=src\%PROJECT%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"

set PATH=%QT_PATH%\bin;%MINGW_PATH%\bin;%WIX%\bin;%WIX%;%WINDIR%;%WINDIR%\System32

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" CONFIG+="enable_qtwebkit disable_wic" CONFIG+="enable_cxx11" QMAKE_CXXFLAGS+="-std=gnu++1y" ..\%PROJECT%.pro
mingw32-make
strip --strip-all %APP_PATH%\release\%PROJECT%.exe
mkdir %PROJECT%%SUFFIX%
copy %APP_PATH%\release\%PROJECT%.exe %PROJECT%%SUFFIX%\%PROJECT%.exe
for %%x in (Core, Gui, Network, Svg, WebKit) do (
    copy %QT_PATH%\bin\Qt%%x4.dll %PROJECT%%SUFFIX%\
)
for %%x in (accessible, bearer, codecs, graphicssystems, iconengines, imageformats) do (
    mkdir %PROJECT%%SUFFIX%\plugins\%%x
    copy %QT_PATH%\plugins\%%x\*.dll %PROJECT%%SUFFIX%\plugins\%%x\
    del /S /Q %PROJECT%%SUFFIX%\plugins\%%x\*d4.dll
)
mkdir %PROJECT%%SUFFIX%\translations
for %%x in (en ru) do (
    copy %QT_PATH%\translations\qt_%%x.qm %PROJECT%%SUFFIX%\translations\
)
(
    echo [Paths]
    echo Translations = translations
    echo Plugins = plugins
)> %PROJECT%%SUFFIX%\qt.conf
%ZIP_CMD% -9r ..\%PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%
rmdir /S /Q build_msi 2>nul >nul
move %PROJECT%%SUFFIX% build_msi
heat dir build_msi -cg ApplicationFiles -dr INSTALLLOCATION -gg -scom -sfrag -srd -sreg -svb6 -out appfiles.wxs
candle -out appfiles.wixobj appfiles.wxs -arch x86
candle -out common.wixobj ..\src\ImageViewer\resources\platform\windows\common.wxs -arch x86
candle -out main.wixobj ..\src\ImageViewer\resources\platform\windows\w2k.wxs -ext WixUIExtension -arch x86
light -out %PROJECT%.msi -b build_msi main.wixobj appfiles.wixobj common.wixobj -ext WixUIExtension -dcl:high
move %PROJECT%.msi ..\%PROJECT%%SUFFIX%.msi
cd ..

if NOT "%CI%" == "true" pause
