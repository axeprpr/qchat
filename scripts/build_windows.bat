@echo off
REM ============================================================================
REM QChat - Windows Build Script
REM Requirements: Qt 6.5+, CMake 3.21+, Visual Studio 2019/2022 or MinGW
REM ============================================================================

setlocal enabledelayedexpansion

echo.
echo ============================================
echo    QChat Build Script for Windows
echo ============================================
echo.

REM ---- Configuration ----
set BUILD_TYPE=Release
set BUILD_DIR=build-win
set QT_DIR=

REM Try to find Qt automatically
for /d %%d in ("C:\Qt\6.*" "D:\Qt\6.*" "%USERPROFILE%\Qt\6.*") do (
    if exist "%%d\msvc2022_64\bin\qmake.exe" (
        set QT_DIR=%%d\msvc2022_64
        echo Found Qt at: !QT_DIR!
        goto :found_qt
    )
    if exist "%%d\msvc2019_64\bin\qmake.exe" (
        set QT_DIR=%%d\msvc2019_64
        echo Found Qt at: !QT_DIR!
        goto :found_qt
    )
    if exist "%%d\mingw_64\bin\qmake.exe" (
        set QT_DIR=%%d\mingw_64
        echo Found Qt at: !QT_DIR!
        goto :found_qt
    )
)

if "%QT_DIR%"=="" (
    echo [ERROR] Qt 6 not found automatically.
    echo Please set QT_DIR manually, e.g.:
    echo   set QT_DIR=C:\Qt\6.7.2\msvc2022_64
    echo   %~nx0
    exit /b 1
)

:found_qt

REM ---- Check FluentUI ----
if not exist "third_party\FluentUI\CMakeLists.txt" (
    echo.
    echo Downloading FluentUI...
    git clone --depth 1 https://github.com/zhuzichu520/FluentUI.git third_party\FluentUI
    if errorlevel 1 (
        echo [WARNING] FluentUI download failed. Building without FluentUI.
    )
)

REM ---- Build ----
echo.
echo Building QChat (%BUILD_TYPE%)...
echo.

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

cmake -B %BUILD_DIR% -S . ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_PREFIX_PATH="%QT_DIR%" ^
    -G "Ninja"

if errorlevel 1 (
    echo.
    echo [NOTE] Ninja not found, trying Visual Studio generator...
    cmake -B %BUILD_DIR% -S . ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DCMAKE_PREFIX_PATH="%QT_DIR%"
)

if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

REM ---- Deploy ----
echo.
echo Deploying Qt dependencies...
set DEPLOY_DIR=%BUILD_DIR%\deploy

if not exist %DEPLOY_DIR% mkdir %DEPLOY_DIR%
copy %BUILD_DIR%\QChat.exe %DEPLOY_DIR%\ >nul 2>&1
copy %BUILD_DIR%\%BUILD_TYPE%\QChat.exe %DEPLOY_DIR%\ >nul 2>&1

"%QT_DIR%\bin\windeployqt.exe" --qmldir src\qml %DEPLOY_DIR%\QChat.exe

echo.
echo ============================================
echo    Build complete!
echo    Output: %DEPLOY_DIR%\QChat.exe
echo ============================================
echo.

endlocal
