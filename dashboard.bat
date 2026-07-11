@echo off
setlocal EnableExtensions EnableDelayedExpansion
title YOLO Mouse - Project Dashboard
cd /d "%~dp0"

set "APP_NAME=YOLO Mouse"
set "BUILD_DIR=%CD%\build"
set "LOG_DIR=%CD%\logs"
set "LOG_FILE=%LOG_DIR%\build-errors.log"
set "EXE_FILE=%BUILD_DIR%\Release\yolo-mouse.exe"
set "CONFIG_FILE=%CD%\yolo-mouse.ini"
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%" >nul 2>&1
call :load_custom_settings

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
    echo    [10] Customise app settings and colour
echo.
set "choice="
set /p "choice=    Choose an option [1-10]: "
if "%choice%"=="1" goto setup
if "%choice%"=="2" goto run
if "%choice%"=="3" goto build
if "%choice%"=="4" goto folder
if "%choice%"=="5" goto logs
if "%choice%"=="6" goto requirements
if "%choice%"=="7" goto rebuild
if "%choice%"=="8" goto help
if "%choice%"=="9" goto goodbye
if "%choice%"=="10" goto customise
call :message "That option is not valid. Enter a number from 1 to 10."
goto menu

:customise
cls
call :header
echo.
echo    CUSTOMISATION CENTRE
echo.
echo    [1] Duration ................ !CFG_DURATION! seconds
echo    [2] Pattern size ............ !CFG_RADIUS! pixels
echo    [3] Motion speed ............ !CFG_SPEED! tenths (10 = 1.0x)
echo    [4] Vertical scale .......... !CFG_HEIGHT! percent
echo    [5] Start countdown ......... !CFG_COUNTDOWN! seconds
echo    [6] Motion pattern .......... !CFG_PATTERN! (0=8, 1=circle, 2=horizontal, 3=vertical)
echo    [7] Return cursor ........... !CFG_RETURN! (1=yes, 0=no)
echo    [8] Always on top ........... !CFG_TOPMOST! (1=yes, 0=no)
echo    [9] Accent colour ........... RGB !CFG_RED!, !CFG_GREEN!, !CFG_BLUE!
echo    [10] Save settings and return
echo    [11] Restore factory defaults
echo.
set "custom_choice="
set /p "custom_choice=    Choose a setting [1-11]: "
if "!custom_choice!"=="1" call :ask_value CFG_DURATION "Duration in seconds [5-300]" 5 300
if "!custom_choice!"=="2" call :ask_value CFG_RADIUS "Pattern size in pixels [20-800]" 20 800
if "!custom_choice!"=="3" call :ask_value CFG_SPEED "Speed in tenths [1-100, 10 = 1.0x]" 1 100
if "!custom_choice!"=="4" call :ask_value CFG_HEIGHT "Vertical scale percent [10-100]" 10 100
if "!custom_choice!"=="5" call :ask_value CFG_COUNTDOWN "Countdown seconds [0-10]" 0 10
if "!custom_choice!"=="6" call :ask_value CFG_PATTERN "Pattern [0-3]" 0 3
if "!custom_choice!"=="7" call :ask_value CFG_RETURN "Return cursor [1=yes, 0=no]" 0 1
if "!custom_choice!"=="8" call :ask_value CFG_TOPMOST "Always on top [1=yes, 0=no]" 0 1
if "!custom_choice!"=="9" goto colour
if "!custom_choice!"=="10" goto save_custom
if "!custom_choice!"=="11" goto factory_defaults
goto customise

:colour
cls
call :header
echo.
echo    ACCENT COLOUR
echo.
echo    [1] Indigo   [2] Blue     [3] Green
echo    [4] Orange   [5] Pink     [6] Custom RGB
echo.
set /p "colour_choice=    Choose a colour [1-6]: "
if "!colour_choice!"=="1" (set "CFG_RED=99"& set "CFG_GREEN=102"& set "CFG_BLUE=241")
if "!colour_choice!"=="2" (set "CFG_RED=37"& set "CFG_GREEN=150"& set "CFG_BLUE=255")
if "!colour_choice!"=="3" (set "CFG_RED=34"& set "CFG_GREEN=197"& set "CFG_BLUE=94")
if "!colour_choice!"=="4" (set "CFG_RED=249"& set "CFG_GREEN=115"& set "CFG_BLUE=22")
if "!colour_choice!"=="5" (set "CFG_RED=236"& set "CFG_GREEN=72"& set "CFG_BLUE=153")
if "!colour_choice!"=="6" (
    call :ask_value CFG_RED "Red [0-255]" 0 255
    call :ask_value CFG_GREEN "Green [0-255]" 0 255
    call :ask_value CFG_BLUE "Blue [0-255]" 0 255
)
goto customise

:save_custom
call :save_custom_settings
call :message "Custom settings saved. They load next time the app opens."
goto menu

:factory_defaults
set "CFG_DURATION=15"
set "CFG_RADIUS=180"
set "CFG_SPEED=10"
set "CFG_HEIGHT=50"
set "CFG_COUNTDOWN=3"
set "CFG_PATTERN=0"
set "CFG_RETURN=1"
set "CFG_TOPMOST=0"
set "CFG_RED=99"
set "CFG_GREEN=102"
set "CFG_BLUE=241"
goto customise

:ask_value
set "new_value="
set /p "new_value=    %~2: "
for /f "delims=0123456789" %%A in ("!new_value!") do set "new_value="
if not defined new_value (
    call :message "Please enter numbers only."
    exit /b 1
)
if !new_value! LSS %3 set "new_value=%3"
if !new_value! GTR %4 set "new_value=%4"
set "%~1=!new_value!"
exit /b 0

:load_custom_settings
set "CFG_DURATION=15"
set "CFG_RADIUS=180"
set "CFG_SPEED=10"
set "CFG_HEIGHT=50"
set "CFG_COUNTDOWN=3"
set "CFG_PATTERN=0"
set "CFG_RETURN=1"
set "CFG_TOPMOST=0"
set "CFG_RED=99"
set "CFG_GREEN=102"
set "CFG_BLUE=241"
if not exist "%CONFIG_FILE%" exit /b 0
for /f "usebackq tokens=1,2 delims==" %%A in ("%CONFIG_FILE%") do (
    if /i "%%A"=="Duration" set "CFG_DURATION=%%B"
    if /i "%%A"=="Radius" set "CFG_RADIUS=%%B"
    if /i "%%A"=="SpeedTenths" set "CFG_SPEED=%%B"
    if /i "%%A"=="HeightPercent" set "CFG_HEIGHT=%%B"
    if /i "%%A"=="Countdown" set "CFG_COUNTDOWN=%%B"
    if /i "%%A"=="Pattern" set "CFG_PATTERN=%%B"
    if /i "%%A"=="ReturnCursor" set "CFG_RETURN=%%B"
    if /i "%%A"=="AlwaysOnTop" set "CFG_TOPMOST=%%B"
    if /i "%%A"=="AccentRed" set "CFG_RED=%%B"
    if /i "%%A"=="AccentGreen" set "CFG_GREEN=%%B"
    if /i "%%A"=="AccentBlue" set "CFG_BLUE=%%B"
)
exit /b 0

:save_custom_settings
>"%CONFIG_FILE%" echo [Settings]
>>"%CONFIG_FILE%" echo Duration=!CFG_DURATION!
>>"%CONFIG_FILE%" echo Radius=!CFG_RADIUS!
>>"%CONFIG_FILE%" echo SpeedTenths=!CFG_SPEED!
>>"%CONFIG_FILE%" echo HeightPercent=!CFG_HEIGHT!
>>"%CONFIG_FILE%" echo Countdown=!CFG_COUNTDOWN!
>>"%CONFIG_FILE%" echo Pattern=!CFG_PATTERN!
>>"%CONFIG_FILE%" echo ReturnCursor=!CFG_RETURN!
>>"%CONFIG_FILE%" echo AlwaysOnTop=!CFG_TOPMOST!
>>"%CONFIG_FILE%" echo AccentRed=!CFG_RED!
>>"%CONFIG_FILE%" echo AccentGreen=!CFG_GREEN!
>>"%CONFIG_FILE%" echo AccentBlue=!CFG_BLUE!
exit /b 0

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
