@echo off

set "RESVG_VERSION=0.45.1"

rem Windows 7 support is dropped since 1.76.0 It has been moved to
rem {x86_64,i686}-win7-windows-msvc targes. See changelog here:
rem https://releases.rs/docs/1.76.0/
set "RUST_VERSION=1.73.0"

for /F "tokens=1,2*" %%i in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') DO (
    if "%%i" == "PROCESSOR_ARCHITECTURE" (
        set PROCESSOR_ARCHITECTURE=%%~k
    )
)
if /i "%PROCESSOR_ARCHITECTURE%" == "ARM64" (
    set "RESVG_HOST=aarch64-pc-windows-msvc"
    set "VCVARS_HOST=arm64"
) else (
    set "RESVG_HOST=x86_64-pc-windows-msvc"
    set "VCVARS_HOST=x64"
)

if "%1" == "x86_64-pc-windows-msvc"     goto :x86_64
if "%1" == "i686-pc-windows-msvc"       goto :i686
if "%1" == "aarch64-pc-windows-msvc"    goto :aarch64

:x86_64
set "RESVG_TARGET=x86_64-pc-windows-msvc"
set "VCVARS_ARCH=x64"
goto :build

:i686
set "RESVG_TARGET=i686-pc-windows-msvc"
set "VCVARS_ARCH=x86"
goto :build

:aarch64
set "RESVG_TARGET=aarch64-pc-windows-msvc"
set "VCVARS_ARCH=arm64"
goto :build

:build
call "%~dp0\..\helpers\find_vcvarsall.bat" 2022
set "VCVARS=%VS2022_VCVARSALL%"

set "RUSTUP_HOME=%CD%\RUSTUP_HOME"
set "CARGO_HOME=%CD%\CARGO_HOME"
set "PATH=%CARGO_HOME%\bin;%PATH%"

if /i "%VCVARS_HOST%" NEQ "%VCVARS_ARCH%" set "VCVARS_ARCH=%VCVARS_HOST%_%VCVARS_ARCH%"
call "%VCVARS%" %VCVARS_ARCH%
cd "%~dp0"

curl -LO "https://static.rust-lang.org/rustup/dist/%RESVG_HOST%/rustup-init.exe"
rustup-init.exe --default-host "%RESVG_HOST%" --target "%RESVG_TARGET%" --default-toolchain "%RUST_VERSION%" --profile default --no-modify-path -y

curl -LO "https://www.7-zip.org/a/7zr.exe"
curl -LO "https://github.com/linebender/resvg/releases/download/v%RESVG_VERSION%/resvg-%RESVG_VERSION%.tar.xz"
7zr x "resvg-%RESVG_VERSION%.tar.xz"
tar -xvpf "resvg-%RESVG_VERSION%.tar"
set "OLD_DIR=%CD%"
cd "resvg-%RESVG_VERSION%\crates\c-api"
cargo build --release --target "%RESVG_TARGET%"
cd "%OLD_DIR%"

rmdir /S /Q %RESVG_TARGET% 2>nul >nul
mkdir "%RESVG_TARGET%"
copy "resvg-%RESVG_VERSION%\crates\c-api\resvg.h" "%RESVG_TARGET%\resvg.h"
copy "resvg-%RESVG_VERSION%\target\%RESVG_TARGET%\release\resvg.dll" "%RESVG_TARGET%\resvg.dll"
copy "resvg-%RESVG_VERSION%\target\%RESVG_TARGET%\release\resvg.dll.lib" "%RESVG_TARGET%\resvg.lib"

rmdir /S /Q "%RUSTUP_HOME%" 2>nul >nul
rmdir /S /Q "%CARGO_HOME%" 2>nul >nul
rmdir /S /Q "resvg-%RESVG_VERSION%" 2>nul >nul
del /Q /F "rustup-init.exe" "7zr.exe" "resvg-%RESVG_VERSION%.tar.xz" "resvg-%RESVG_VERSION%.tar"
