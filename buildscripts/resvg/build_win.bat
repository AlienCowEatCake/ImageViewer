@echo off

set "RESVG_VERSION=0.42.0"

if "%1" == "x86_64-pc-windows-msvc"     goto :x86_64
if "%1" == "i686-pc-windows-msvc"       goto :i686
if "%1" == "aarch64-pc-windows-msvc"    goto :aarch64

:x86_64
set "RESVG_TARGET=x86_64-pc-windows-msvc"
set "VCVARS_ARCH=x64"
goto :build

:i686
set "RESVG_TARGET=i686-pc-windows-msvc"
set "VCVARS_ARCH=x64_x86"
goto :build

:aarch64
set "RESVG_TARGET=aarch64-pc-windows-msvc"
set "VCVARS_ARCH=x64_arm64"
goto :build

:build
call "%~dp0\..\helpers\find_vcvarsall.bat" 2022
set "VCVARS=%VS2022_VCVARSALL%"

set "RUSTUP_HOME=%CD%\RUSTUP_HOME"
set "CARGO_HOME=%CD%\CARGO_HOME"
set "PATH=%CARGO_HOME%\bin;%PATH%"

call "%VCVARS%" %VCVARS_ARCH%
cd "%~dp0"

curl -LO "https://static.rust-lang.org/rustup/dist/x86_64-pc-windows-msvc/rustup-init.exe"
rustup-init.exe --default-host x86_64-pc-windows-msvc --target "%RESVG_TARGET%" --default-toolchain stable --profile default --no-modify-path -y

curl -LO "https://www.7-zip.org/a/7zr.exe"
curl -LO "https://github.com/RazrFalcon/resvg/releases/download/v%RESVG_VERSION%/resvg-%RESVG_VERSION%.tar.xz"
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
