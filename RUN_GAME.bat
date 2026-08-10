@echo off
setlocal
set "PATH=C:\msys64\ucrt64\bin;%PATH%"
cd /d "%~dp0build"
if not exist "my_game.exe" (
    echo [ERROR] my_game.exe not found in build directory!
    echo Run cmake build first.
    pause
    exit /b 1
)
start my_game.exe
