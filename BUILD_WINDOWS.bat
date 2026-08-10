@echo off
setlocal

where cmake >nul 2>nul
if errorlevel 1 (
  echo [ERROR] CMake was not found in PATH.
  echo Install CMake or open this folder with Visual Studio 2022.
  pause
  exit /b 1
)

if not defined SFML_DIR set "SFML_DIR=C:/SFML3/lib/cmake/SFML"
if not exist "%SFML_DIR%/SFMLConfig.cmake" (
  echo [ERROR] SFMLConfig.cmake was not found at:
  echo %SFML_DIR%
  echo Set SFML_DIR to the folder containing SFMLConfig.cmake, then run again.
  pause
  exit /b 1
)

cmake -S . -B build -DSFML_DIR="%SFML_DIR%"
if errorlevel 1 exit /b 1

cmake --build build --config Debug
if errorlevel 1 exit /b 1

echo.
echo Build completed. Look for my_game.exe inside build.
pause
