@echo off
setlocal EnableExtensions EnableDelayedExpansion
title YOLO Mouse - Project Dashboard
cd /d "%~dp0"

set "APP_NAME=YOLO Mouse"
set "BUILD_DIR=%CD%\build"
set "LOG_DIR=%CD%\logs"
set "LOG_FILE=%LOG_DIR%\build-errors.log"
set "EXE_FILE=%BUILD_DIR%\Release\yolo-mouse.exe"
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>&1

:menu
cls
call :header
echo.
echo    [1] First-time setup and build
echo    [2] Run YOLO Mouse
echo    [3] Build latest changes
echo    [4] Open project folder
echo    [5] Open build errors and logs
echo    [6] Check installed requirements
echo    [7] Clean and rebuild everything
echo    [8] Help and project information
echo    [9] Exit dashboard
echo.
set "choice="
set /p "choice=    Choose an option [1-9]: "
if "%choice%"=="1" goto setup
if "%choice%"=="2" goto run
if "%choice%"=="3" goto build
if "%choice%"=="4" goto folder
if "%choice%"=="5" goto logs
if "%choice%"=="6" goto requirements
if "%choice%"=="7" goto rebuild
if "%choice%"=="8" goto help
if "%choice%"=="9" goto goodbye
call :message "That option is not valid. Enter a number from 1 to 9."
goto menu

:setup
cls
call :header
echo.
echo    FIRST-TIME SETUP
echo    This checks your tools, configures CMake, and builds the app.
echo.
call :find_tools
if errorlevel 1 goto failed
call :configure
if errorlevel 1 goto failed
call :compile
if errorlevel 1 goto failed
echo.
echo    [SUCCESS] Setup finished. YOLO Mouse is ready.
choice /c YN /n /m "    Run it now? [Y/N]: "
if errorlevel 2 goto pause_menu
goto run

:build
cls
call :header
echo.
echo    BUILD LATEST CHANGES
echo.
call :find_tools
if errorlevel 1 goto failed
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    call :configure
    if errorlevel 1 goto failed
)
call :compile
if errorlevel 1 goto failed
echo.
echo    [SUCCESS] Build complete.
goto pause_menu

:run
cls
call :header
echo.
if not exist "%EXE_FILE%" (
    echo    The program has not been built yet.
    choice /c YN /n /m "    Run automatic setup now? [Y/N]: "
    if errorlevel 2 goto pause_menu
    goto setup
)
echo    Starting YOLO Mouse...
start "YOLO Mouse" "%EXE_FILE%"
echo    [SUCCESS] The app was opened.
goto pause_menu

:folder
start "" explorer.exe "%CD%"
call :message "Project folder opened."
goto menu

:logs
if not exist "%LOG_FILE%" (
    >"%LOG_FILE%" echo No build errors have been recorded yet.
)
start "Build errors" notepad.exe "%LOG_FILE%"
call :message "The build log was opened in Notepad."
goto menu

:requirements
cls
call :header
echo.
echo    REQUIREMENTS CHECK
echo.
where cmake >nul 2>&1
if errorlevel 1 (echo    [MISSING] CMake) else (for /f "delims=" %%V in ('cmake --version 2^>nul ^| findstr /b /c:"cmake version"') do echo    [FOUND]   %%V)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_PATH=%%I"
)
if defined VS_PATH (echo    [FOUND]   Visual Studio C++ tools at !VS_PATH!) else (echo    [MISSING] Visual Studio C++ build tools)
if exist "%EXE_FILE%" (echo    [READY]   Compiled application found) else (echo    [INFO]    Application needs to be built)
echo.
echo    Missing tools: install Visual Studio with "Desktop development with C++".
goto pause_menu

:rebuild
cls
call :header
echo.
echo    CLEAN AND REBUILD
echo    This removes generated build files, not your source code.
echo.
choice /c YN /n /m "    Continue? [Y/N]: "
if errorlevel 2 goto menu
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
goto setup

:help
cls
call :header
echo.
echo    ABOUT THIS DASHBOARD
echo.
echo    1  Installs nothing silently; it checks required tools and builds.
echo    2  Opens the compiled GUI. If missing, it offers automatic setup.
echo    3  Compiles source changes without deleting the build folder.
echo    4  Opens this project in Windows File Explorer.
echo    5  Opens the most recent configuration or compiler output.
echo    6  Reports whether CMake, Visual Studio, and the app are ready.
echo    7  Deletes generated build files and creates a fresh build.
echo.
echo    In the app, press Esc at any time to stop cursor movement safely.
echo    Project: %CD%
goto pause_menu

:find_tools
where cmake >nul 2>&1
if errorlevel 1 (
    >"%LOG_FILE%" echo ERROR: CMake was not found in PATH.
    echo    [ERROR] CMake was not found.
    echo    Install Visual Studio with Desktop development with C++ and CMake.
    exit /b 1
)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VS_PATH="
if exist "%VSWHERE%" for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_PATH=%%I"
if not defined VS_PATH (
    >"%LOG_FILE%" echo ERROR: Visual Studio C++ build tools were not found.
    echo    [ERROR] Visual Studio C++ build tools were not found.
    exit /b 1
)
set "VS_DEV_CMD=!VS_PATH!\Common7\Tools\VsDevCmd.bat"
if not exist "!VS_DEV_CMD!" (
    >"%LOG_FILE%" echo ERROR: VsDevCmd.bat was not found at !VS_DEV_CMD!.
    echo    [ERROR] The Visual Studio developer environment is incomplete.
    exit /b 1
)
echo    [OK] CMake and Visual Studio C++ tools found.
exit /b 0

:configure
echo    Configuring the project...
call "!VS_DEV_CMD!" -arch=x64 -host_arch=x64 >"%LOG_FILE%" 2>&1
cmake -S "%CD%" -B "%BUILD_DIR%" -A x64 >>"%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo    [ERROR] Configuration failed. Use option 5 to read the log.
    exit /b 1
)
echo    [OK] Project configured.
exit /b 0

:compile
echo    Building the Release version...
cmake --build "%BUILD_DIR%" --config Release >>"%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo    [ERROR] Build failed. Use option 5 to read the log.
    exit /b 1
)
if not exist "%EXE_FILE%" (
    echo    [ERROR] Build finished but the application was not found.
    exit /b 1
)
echo    [OK] Application built successfully.
exit /b 0

:failed
echo.
echo    Setup could not finish. Choose option 5 to inspect the error log.
goto pause_menu

:pause_menu
echo.
pause
goto menu

:message
cls
call :header
echo.
echo    %~1
timeout /t 2 /nobreak >nul
exit /b 0

:header
echo    ============================================================
echo                     YOLO MOUSE DASHBOARD
echo    ============================================================
echo       Setup  -  Build  -  Run  -  Diagnose  -  Project tools
exit /b 0

:goodbye
cls
call :header
echo.
echo    Dashboard closed. Your project files were not changed.
timeout /t 1 /nobreak >nul
endlocal
exit /b 0
