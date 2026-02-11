@echo off
set PROJECT=ImageViewer
if "x%QT_PATH%x" == "xx" set QT_PATH=C:\Qt\5.6.3\tdmgcc1030-sjlj
if "x%MINGW_PATH%x" == "xx" set MINGW_PATH=C:\Qt\Tools\tdmgcc1030-sjlj_32
set BUILDDIR=build_win_qt5.6_tdmgcc1030-sjlj
set SUFFIX=_qt5.6_mingw32
set APP_PATH=src\%PROJECT%
set ZIP_CMD="%~dp0\..\buildscripts\helpers\zip.exe"
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') DO (
    if "%%i" == "PROCESSOR_ARCHITECTURE" if "%%~k" == "ARM64" (
        set ZIP_CMD="%~dp0\..\buildscripts\helpers\arm64\zip.exe"
    )
)

set PATH=%QT_PATH%\bin;%MINGW_PATH%\bin;%WIX%\bin;%WIX%;%WINDIR%;%WINDIR%\System32

cd "%~dp0"
cd ..
rmdir /S /Q %BUILDDIR% 2>nul >nul
mkdir %BUILDDIR%
cd %BUILDDIR%
qmake -r CONFIG+="release" CONFIG+="disable_embed_translations" ..\%PROJECT%.pro
mingw32-make -j%NUMBER_OF_PROCESSORS%
if not exist %APP_PATH%\release\%PROJECT%.exe (
    if NOT "%CI%" == "true" pause
    exit /b 1
)
strip --strip-all %APP_PATH%\release\%PROJECT%.exe
mkdir %PROJECT%%SUFFIX%
copy %APP_PATH%\release\%PROJECT%.exe %PROJECT%%SUFFIX%\%PROJECT%.exe
windeployqt --release --no-compiler-runtime --no-system-d3d-compiler --no-angle --no-opengl-sw %PROJECT%%SUFFIX%
copy ..\src\%PROJECT%\resources\translations\*.qm %PROJECT%%SUFFIX%\translations\
copy ..\src\QtUtils\resources\translations\*.qm %PROJECT%%SUFFIX%\translations\
%ZIP_CMD% -9r ..\%PROJECT%%SUFFIX%.zip %PROJECT%%SUFFIX%
rmdir /S /Q build_msi 2>nul >nul
move %PROJECT%%SUFFIX% build_msi
heat dir build_msi -cg ApplicationFiles -dr INSTALLLOCATION -gg -scom -sfrag -srd -sreg -svb6 -out appfiles.wxs
candle -out appfiles.wixobj appfiles.wxs -arch x86
candle -out common.wixobj ..\src\ImageViewer\resources\platform\windows\common.wxs -arch x86
candle -out main.wixobj ..\src\ImageViewer\resources\platform\windows\wxp.wxs -ext WixUIExtension -ext WixUtilExtension -arch x86
light -out %PROJECT%.msi -b build_msi main.wixobj appfiles.wixobj common.wixobj -ext WixUIExtension -ext WixUtilExtension -dcl:high -sice:ICE57
move %PROJECT%.msi ..\%PROJECT%%SUFFIX%.msi
cd ..

if NOT "%CI%" == "true" pause
