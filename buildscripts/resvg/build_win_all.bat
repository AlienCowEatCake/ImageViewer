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

cd "%~dp0"

set "RUSTUP_HOME=%CD%\RUSTUP_HOME"
set "CARGO_HOME=%CD%\CARGO_HOME"
set "PATH=%CARGO_HOME%\bin;%PATH%"

call "%~dp0\..\helpers\find_vcvarsall.bat" 2022
set "VCVARS=%VS2022_VCVARSALL%"

curl -LO "https://static.rust-lang.org/rustup/dist/%RESVG_HOST%/rustup-init.exe"
setlocal
call "%VCVARS%" %VCVARS_HOST%
rustup-init.exe --default-host "%RESVG_HOST%" --target "x86_64-pc-windows-msvc" --target "i686-pc-windows-msvc" --target "aarch64-pc-windows-msvc" --default-toolchain "%RUST_VERSION%" --profile default --no-modify-path -y
endlocal

curl -Lo "resvg-%RESVG_VERSION%.tar.gz" "https://github.com/linebender/resvg/archive/refs/tags/v%RESVG_VERSION%.tar.gz"
tar -xvpf "resvg-%RESVG_VERSION%.tar.gz"

pushd "resvg-%RESVG_VERSION%\crates\c-api"

setlocal
set "RESVG_TARGET=x86_64-pc-windows-msvc"
set "VCVARS_ARCH=x64"
if /i "%VCVARS_HOST%" NEQ "%VCVARS_ARCH%" set "VCVARS_ARCH=%VCVARS_HOST%_%VCVARS_ARCH%"
call "%VCVARS%" %VCVARS_ARCH%
cargo build --release --target "%RESVG_TARGET%"
endlocal

setlocal
set "RESVG_TARGET=i686-pc-windows-msvc"
set "VCVARS_ARCH=x86"
if /i "%VCVARS_HOST%" NEQ "%VCVARS_ARCH%" set "VCVARS_ARCH=%VCVARS_HOST%_%VCVARS_ARCH%"
call "%VCVARS%" %VCVARS_ARCH%
cargo build --release --target "%RESVG_TARGET%"
endlocal

setlocal
set "RESVG_TARGET=aarch64-pc-windows-msvc"
set "VCVARS_ARCH=arm64"
if /i "%VCVARS_HOST%" NEQ "%VCVARS_ARCH%" set "VCVARS_ARCH=%VCVARS_HOST%_%VCVARS_ARCH%"
call "%VCVARS%" %VCVARS_ARCH%
cargo build --release --target "%RESVG_TARGET%"
endlocal

popd

for %%i in (x86_64-pc-windows-msvc i686-pc-windows-msvc aarch64-pc-windows-msvc) do (
    rmdir /S /Q "%%i" 2>nul >nul
    mkdir "%%i"
    copy "resvg-%RESVG_VERSION%\crates\c-api\resvg.h" "%%i\resvg.h"
    copy "resvg-%RESVG_VERSION%\target\%%i\release\resvg.dll" "%%i\resvg.dll"
    copy "resvg-%RESVG_VERSION%\target\%%i\release\resvg.dll.lib" "%%i\resvg.lib"
)

rmdir /S /Q "%RUSTUP_HOME%" 2>nul >nul
rmdir /S /Q "%CARGO_HOME%" 2>nul >nul
rmdir /S /Q "resvg-%RESVG_VERSION%" 2>nul >nul
del /Q /F "rustup-init.exe" "resvg-%RESVG_VERSION%.tar.gz"
