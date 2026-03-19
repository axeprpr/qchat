@echo off
REM ============================================================================
REM QChat - Windows Build Script
REM Requirements: Qt 6.5+, CMake 3.21+, Visual Studio 2019/2022 or MinGW
REM ============================================================================

setlocal enabledelayedexpansion

REM ---- Navigate to project root (parent of scripts/) ----
pushd "%~dp0.."

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
    popd
    exit /b 1
)

:found_qt

REM ---- Add Qt tools to PATH ----
set "PATH=%QT_DIR%\bin;%QT_DIR%\..\..\Tools\mingw1310_64\bin;%QT_DIR%\..\..\Tools\mingw1120_64\bin;%QT_DIR%\..\..\Tools\Ninja;%PATH%"

REM ---- Check required QML compatibility module ----
set QT5COMPAT_QML_DIR=%QT_DIR%\qml\Qt5Compat\GraphicalEffects
if not exist "%QT5COMPAT_QML_DIR%\qmldir" (
    echo [ERROR] Missing Qt QML module: Qt5Compat.GraphicalEffects
    echo Install the "Qt 5 Compatibility Module" for this Qt kit in the Qt Maintenance Tool, then rebuild.
    echo Expected path:
    echo   %QT5COMPAT_QML_DIR%
    popd
    exit /b 1
)

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
    echo [NOTE] Ninja not found, trying MinGW Makefiles...
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
    mkdir %BUILD_DIR%
    cmake -B %BUILD_DIR% -S . ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DCMAKE_PREFIX_PATH="%QT_DIR%" ^
        -G "MinGW Makefiles"
)

if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    popd
    exit /b 1
)

cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo [ERROR] Build failed.
    popd
    exit /b 1
)

REM ---- Deploy ----
echo.
echo Deploying Qt dependencies...
set DEPLOY_DIR=%BUILD_DIR%\deploy

if not exist %DEPLOY_DIR% mkdir %DEPLOY_DIR%

REM Find the built exe (Ninja puts it directly in build dir, MSVC in Release/ subfolder)
set EXE_PATH=
if exist "%BUILD_DIR%\QChat.exe" set EXE_PATH=%BUILD_DIR%\QChat.exe
if exist "%BUILD_DIR%\%BUILD_TYPE%\QChat.exe" set EXE_PATH=%BUILD_DIR%\%BUILD_TYPE%\QChat.exe

if "%EXE_PATH%"=="" (
    echo [ERROR] QChat.exe not found in build directory.
    popd
    exit /b 1
)

echo Found executable: %EXE_PATH%
copy "%EXE_PATH%" "%DEPLOY_DIR%\" >nul

REM Use windeployqt6 to copy all required Qt DLLs, QML modules, and plugins
echo Running windeployqt...
"%QT_DIR%\bin\windeployqt.exe" --qmldir src\qml --no-translations "%DEPLOY_DIR%\QChat.exe"

if not errorlevel 1 if exist "third_party\FluentUI\src\Qt6\imports" (
    echo Scanning FluentUI QML imports...
    "%QT_DIR%\bin\windeployqt.exe" --qmldir third_party\FluentUI\src\Qt6\imports --no-translations "%DEPLOY_DIR%\QChat.exe"
)

if errorlevel 1 (
    echo [WARNING] windeployqt failed. Trying to copy DLLs manually...
    copy "%QT_DIR%\bin\Qt6Core.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6Gui.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6Quick.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6QuickControls2.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6Network.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6Svg.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6Widgets.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6Qml.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6QmlModels.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6OpenGL.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6PrintSupport.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy "%QT_DIR%\bin\Qt6QmlWorkerScript.dll" "%DEPLOY_DIR%\" >nul 2>&1

    REM Copy MinGW runtime DLLs
    for %%f in (libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll) do (
        where %%f >nul 2>&1 && copy "%%~$PATH:f" "%DEPLOY_DIR%\" >nul 2>&1
    )
)

if not exist "%DEPLOY_DIR%\qml\Qt5Compat\GraphicalEffects\qmldir" (
    echo Copying Qt5Compat.GraphicalEffects QML module...
    xcopy "%QT_DIR%\qml\Qt5Compat\GraphicalEffects" "%DEPLOY_DIR%\qml\Qt5Compat\GraphicalEffects\" /E /I /Y >nul
)

if not exist "%DEPLOY_DIR%\qml\Qt5Compat\GraphicalEffects\qmldir" (
    echo [ERROR] Deployment is incomplete: Qt5Compat.GraphicalEffects was not copied.
    popd
    exit /b 1
)

echo.
echo ============================================
echo    Build complete!
echo    Output: %DEPLOY_DIR%\QChat.exe
echo.
echo    Run it directly from the deploy folder:
echo      cd %DEPLOY_DIR%
echo      QChat.exe
echo ============================================
echo.

popd
endlocal
