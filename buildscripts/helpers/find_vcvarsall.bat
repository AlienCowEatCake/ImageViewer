@echo off

if "%1" == "2026" goto :vcvarsall_find_2026
if "%1" == "2022" goto :vcvarsall_find_2022
if "%1" == "2019" goto :vcvarsall_find_2019
if "%1" == "2017" goto :vcvarsall_find_2017
if "%1" == "2015" goto :vcvarsall_find_2015
if "%1" == "2013" goto :vcvarsall_find_2013
if "%1" == "2012" goto :vcvarsall_find_2012
if "%1" == "2010" goto :vcvarsall_find_2010
if "%1" == "2008" goto :vcvarsall_find_2008
if "%1" == "2005" goto :vcvarsall_find_2005

echo "Unknown VS version %1"
exit /b 1

rem If environment variable is not set we can find VS in default install directories.
rem Command "ver" is used for reset ERRORLEVEL
rem https://stackoverflow.com/questions/1113727/what-is-the-easiest-way-to-reset-errorlevel-to-zero

rem ================================================================================================
:vcvarsall_find_2026
ver >NUL
set VS2026_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

set "VS2026_VCVARSALL="
pushd "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\" >NUL 2>&1 || goto :vswhere_fail_2026
for /F "delims=" %%i in ('vswhere -property resolvedInstallationPath -version "[18.0,19.0)" -latest') do (
    set "VS2026_VCVARSALL=%%i\VC\Auxiliary\Build"
    popd >NUL 2>&1
    goto :vswhere_found_2026
)
popd >NUL 2>&1
goto :vswhere_fail_2026
:vswhere_found_2026
ver >NUL
dir "%VS2026_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2026_VCVARSALL=%VS2026_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)
:vswhere_fail_2026

ver >NUL
set "VS2026_VCVARSALL=%ProgramFiles%\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build"
dir "%VS2026_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2026_VCVARSALL=%VS2026_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2026_VCVARSALL=%ProgramFiles%\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build"
dir "%VS2026_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2026_VCVARSALL=%VS2026_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2026_VCVARSALL=%ProgramFiles%\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build"
dir "%VS2026_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2026_VCVARSALL=%VS2026_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2026_VCVARSALL=%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build"
dir "%VS2026_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2026_VCVARSALL=%VS2026_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2026_VCVARSALL is not set"
set "VS2026_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2022
ver >NUL
set VS2022_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

set "VS2022_VCVARSALL="
pushd "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\" >NUL 2>&1 || goto :vswhere_fail_2022
for /F "delims=" %%i in ('vswhere -property resolvedInstallationPath -version "[17.0,18.0)" -latest') do (
    set "VS2022_VCVARSALL=%%i\VC\Auxiliary\Build"
    popd >NUL 2>&1
    goto :vswhere_found_2022
)
popd >NUL 2>&1
goto :vswhere_fail_2022
:vswhere_found_2022
ver >NUL
dir "%VS2022_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2022_VCVARSALL=%VS2022_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)
:vswhere_fail_2022

ver >NUL
set "VS2022_VCVARSALL=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build"
dir "%VS2022_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2022_VCVARSALL=%VS2022_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2022_VCVARSALL=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build"
dir "%VS2022_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2022_VCVARSALL=%VS2022_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2022_VCVARSALL=%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build"
dir "%VS2022_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2022_VCVARSALL=%VS2022_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2022_VCVARSALL=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
dir "%VS2022_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2022_VCVARSALL=%VS2022_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2022_VCVARSALL is not set"
set "VS2022_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2019
ver >NUL
set VS2019_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

set "VS2019_VCVARSALL="
pushd "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\" >NUL 2>&1 || goto :vswhere_fail_2019
for /F "delims=" %%i in ('vswhere -property resolvedInstallationPath -version "[16.0,17.0)" -latest') do (
    set "VS2019_VCVARSALL=%%i\VC\Auxiliary\Build"
    popd >NUL 2>&1
    goto :vswhere_found_2019
)
popd >NUL 2>&1
goto :vswhere_fail_2019
:vswhere_found_2019
ver >NUL
dir "%VS2019_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2019_VCVARSALL=%VS2019_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)
:vswhere_fail_2019

ver >NUL
set "VS2019_VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build"
dir "%VS2019_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2019_VCVARSALL=%VS2019_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2019_VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build"
dir "%VS2019_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2019_VCVARSALL=%VS2019_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2019_VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build"
dir "%VS2019_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2019_VCVARSALL=%VS2019_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2019_VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
dir "%VS2019_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2019_VCVARSALL=%VS2019_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2019_VCVARSALL is not set"
set "VS2019_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2017
ver >NUL
set VS2017_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

set "VS2017_VCVARSALL="
pushd "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\" >NUL 2>&1 || goto :vswhere_fail_2017
for /F "delims=" %%i in ('vswhere -property resolvedInstallationPath -version "[15.0,16.0)" -latest') do (
    set "VS2017_VCVARSALL=%%i\VC\Auxiliary\Build"
    popd >NUL 2>&1
    goto :vswhere_found_2017
)
popd >NUL 2>&1
goto :vswhere_fail_2017
:vswhere_found_2017
ver >NUL
dir "%VS2017_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2017_VCVARSALL=%VS2017_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)
:vswhere_fail_2017

ver >NUL
set "VS2017_VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build"
dir "%VS2017_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2017_VCVARSALL=%VS2017_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2017_VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build"
dir "%VS2017_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2017_VCVARSALL=%VS2017_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2017_VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build"
dir "%VS2017_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2017_VCVARSALL=%VS2017_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

ver >NUL
set "VS2017_VCVARSALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build"
dir "%VS2017_VCVARSALL%" >NUL 2>&1
if %ERRORLEVEL% EQU 0 (
    set "VS2017_VCVARSALL=%VS2017_VCVARSALL%\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2017_VCVARSALL is not set"
set "VS2017_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2015
ver >NUL
set VS2015_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

if NOT "%VS140COMNTOOLS%" == "" (
    set "VS2015_VCVARSALL=%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2015_VCVARSALL is not set"
set "VS2015_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2013
ver >NUL
set VS2013_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

if NOT "%VS120COMNTOOLS%" == "" (
    set "VS2013_VCVARSALL=%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2013_VCVARSALL is not set"
set "VS2013_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2012
ver >NUL
set VS2012_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

if NOT "%VS110COMNTOOLS%" == "" (
    set "VS2012_VCVARSALL=%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2012_VCVARSALL is not set"
set "VS2012_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2010
ver >NUL
set VS2010_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

if NOT "%VS100COMNTOOLS%" == "" (
    set "VS2010_VCVARSALL=%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2010_VCVARSALL is not set"
set "VS2010_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2008
ver >NUL
set VS2008_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

if NOT "%VS90COMNTOOLS%" == "" (
    set "VS2008_VCVARSALL=%VS90COMNTOOLS%\..\..\VC\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2008_VCVARSALL is not set"
set "VS2008_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_find_2005
ver >NUL
set VS2005_VCVARSALL >NUL 2>&1
if %ERRORLEVEL% EQU 0 goto :vcvarsall_found

if NOT "%VS80COMNTOOLS%" == "" (
    set "VS2005_VCVARSALL=%VS80COMNTOOLS%\..\..\VC\vcvarsall.bat"
    goto :vcvarsall_found
)

echo "Environment variable VS2005_VCVARSALL is not set"
set "VS2005_VCVARSALL="
exit /b 1


rem ================================================================================================
:vcvarsall_found
ver >NUL
