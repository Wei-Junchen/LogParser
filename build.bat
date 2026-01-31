@echo off
setlocal EnableDelayedExpansion

set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build-release"
set "DEPLOY_DIR=%BUILD_DIR%\deploy"
set "QT_DIR=C:\Users\WJC\anaconda3\Library"
set "QT_BIN=%QT_DIR%\bin"

set "BUILD_INSTALLER=0"
set "CLEAN_ONLY=0"
if /i "%~1"=="installer" set "BUILD_INSTALLER=1"
if /i "%~1"=="clean" set "CLEAN_ONLY=1"

echo.
echo ============================================
echo   LogParser Windows Build System
echo ============================================
echo.

if "%CLEAN_ONLY%"=="1" (
    echo [Clean] Removing build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    if exist "%PROJECT_DIR%LogParser_Setup.exe" del /q "%PROJECT_DIR%LogParser_Setup.exe"
    echo [Clean] Done.
    goto :end_success
)

echo [Check] Verifying build environment...
if not exist "%QT_BIN%\Qt5Core_conda.dll" (
    if not exist "%QT_BIN%\Qt5Core.dll" (
        echo [ERROR] Qt not found at %QT_DIR%
        goto :error
    )
)
echo         Qt: %QT_DIR%

where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found in PATH
    goto :error
)
for /f "tokens=3" %%v in ('cmake --version 2^>nul ^| findstr /i "version"') do echo         CMake: %%v

set "VCVARS_PATH="
for %%v in (2022 2019 2017) do (
    for %%e in (Enterprise Professional Community BuildTools) do (
        if exist "C:\Program Files\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat" (
            set "VCVARS_PATH=C:\Program Files\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat"
            echo         MSVC: VS %%v %%e
            goto :found_vs
        )
        if exist "C:\Program Files (x86)\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat" (
            set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\%%v\%%e\VC\Auxiliary\Build\vcvars64.bat"
            echo         MSVC: VS %%v %%e
            goto :found_vs
        )
    )
)
echo [ERROR] Visual Studio not found
goto :error

:found_vs
echo.
echo [1/6] Initializing MSVC environment...
call "%VCVARS_PATH%" >nul
if errorlevel 1 goto :error

echo [2/6] Preparing build directory...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

echo [3/6] Running CMake configuration...
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%"
if errorlevel 1 goto :error

echo [4/6] Building project...
nmake
if errorlevel 1 goto :error

echo [5/6] Deploying application...
if not exist "%DEPLOY_DIR%" mkdir "%DEPLOY_DIR%"
copy /Y "LogParser.exe" "%DEPLOY_DIR%\" >nul

echo       Running windeployqt...
"%QT_BIN%\windeployqt.exe" --release --no-translations --no-system-d3d-compiler "%DEPLOY_DIR%\LogParser.exe" >nul

echo       Copying additional dependencies...
copy /Y "%QT_BIN%\zlib.dll" "%DEPLOY_DIR%\" >nul 2>&1
copy /Y "%QT_BIN%\zstd.dll" "%DEPLOY_DIR%\" >nul 2>&1
if not exist "%DEPLOY_DIR%\icuuc73.dll" (
    copy /Y "%QT_BIN%\icudt73.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy /Y "%QT_BIN%\icuin73.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy /Y "%QT_BIN%\icuuc73.dll" "%DEPLOY_DIR%\" >nul 2>&1
)
copy /Y "%QT_BIN%\msvcp140.dll" "%DEPLOY_DIR%\" >nul 2>&1
copy /Y "%QT_BIN%\msvcp140_1.dll" "%DEPLOY_DIR%\" >nul 2>&1
copy /Y "%QT_BIN%\msvcp140_2.dll" "%DEPLOY_DIR%\" >nul 2>&1
copy /Y "%QT_BIN%\vcruntime140.dll" "%DEPLOY_DIR%\" >nul 2>&1
copy /Y "%QT_BIN%\vcruntime140_1.dll" "%DEPLOY_DIR%\" >nul 2>&1
copy /Y "%QT_BIN%\concrt140.dll" "%DEPLOY_DIR%\" >nul 2>&1
if exist "%QT_BIN%\libssl-3-x64.dll" (
    copy /Y "%QT_BIN%\libssl-3-x64.dll" "%DEPLOY_DIR%\" >nul 2>&1
    copy /Y "%QT_BIN%\libcrypto-3-x64.dll" "%DEPLOY_DIR%\" >nul 2>&1
)

echo       Verifying deployment...
set "MISSING=0"
for %%f in (LogParser.exe zlib.dll zstd.dll vcruntime140.dll) do (
    if not exist "%DEPLOY_DIR%\%%f" (
        echo       [MISSING] %%f
        set "MISSING=1"
    )
)
if "%MISSING%"=="0" echo       All dependencies verified.

cd /d "%PROJECT_DIR%"
if "%BUILD_INSTALLER%"=="1" (
    echo [6/6] Creating installer...
    if exist "C:\Program Files (x86)\NSIS\makensis.exe" (
        "C:\Program Files (x86)\NSIS\makensis.exe" /V2 installer.nsi
        if errorlevel 1 goto :error
        echo       Installer created: LogParser_Setup.exe
    ) else (
        echo [WARNING] NSIS not found
    )
) else (
    echo [6/6] Skipping installer
)

:end_success
echo.
echo ============================================
echo   BUILD SUCCESSFUL
echo ============================================
echo   Executable: %DEPLOY_DIR%\LogParser.exe
if "%BUILD_INSTALLER%"=="1" if exist "%PROJECT_DIR%LogParser_Setup.exe" echo   Installer:  %PROJECT_DIR%LogParser_Setup.exe
echo ============================================
endlocal
exit /b 0

:error
echo.
echo ============================================
echo   BUILD FAILED
echo ============================================
endlocal
pause
exit /b 1