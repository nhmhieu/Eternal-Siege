@echo off
setlocal

set "PATH=C:\msys64\ucrt64\bin;%PATH%"

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] CMake was not found in PATH.
    echo Please install MSYS2 UCRT64 toolchain or CMake.
    pause
    exit /b 1
)

cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\msys64\ucrt64
if errorlevel 1 exit /b 1

cmake --build build --parallel 4
if errorlevel 1 exit /b 1

echo.
echo Build completed successfully.
echo Run RUN_GAME.bat to start Eternal Siege.
pause
