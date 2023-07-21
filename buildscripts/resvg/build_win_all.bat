@echo off

set "OLD_PATH=%PATH%"
call "%~dp0\build_win.bat" x86_64-pc-windows-msvc
set "PATH=%OLD_PATH%"
call "%~dp0\build_win.bat" i686-pc-windows-msvc
set "PATH=%OLD_PATH%"
call "%~dp0\build_win.bat" aarch64-pc-windows-msvc
set "PATH=%OLD_PATH%"
