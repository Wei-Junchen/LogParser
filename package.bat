@echo off
REM LogParser NSIS Packaging Script
REM This script creates a Windows installer using NSIS

setlocal EnableDelayedExpansion

echo ============================================
echo   LogParser NSIS Installer Builder
echo ============================================
echo.

set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build-mingw"
set "DEPLOY_DIR=%BUILD_DIR%\deploy"
set "QT_DIR=C:\Qt\6.5.3\mingw_64"
set "MINGW_DIR=C:\Qt\Tools\mingw1120_64"
set "NSIS=C:\Program Files (x86)\NSIS\makensis.exe"

REM Check if NSIS is installed
if not exist "%NSIS%" (
    echo [ERROR] NSIS not found at %NSIS%
    echo Please install NSIS from https://nsis.sourceforge.io/
    goto :error
)

REM Check if build exists
if not exist "%BUILD_DIR%\LogParser.exe" (
    echo [ERROR] LogParser.exe not found in %BUILD_DIR%
    echo Please build the project first!
    goto :error
)

echo [1/4] Preparing deploy directory...
if exist "%DEPLOY_DIR%" rmdir /s /q "%DEPLOY_DIR%"
mkdir "%DEPLOY_DIR%"

REM Copy executable
echo       Copying LogParser.exe...
copy /Y "%BUILD_DIR%\LogParser.exe" "%DEPLOY_DIR%\" >nul

REM Copy Qt DLLs
echo       Copying Qt DLLs...
copy /Y "%QT_DIR%\bin\Qt6Core.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%QT_DIR%\bin\Qt6Gui.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%QT_DIR%\bin\Qt6Widgets.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%QT_DIR%\bin\Qt6Charts.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%QT_DIR%\bin\Qt6Qml.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%QT_DIR%\bin\Qt6Network.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%QT_DIR%\bin\Qt6OpenGL.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%QT_DIR%\bin\Qt6OpenGLWidgets.dll" "%DEPLOY_DIR%\" >nul

REM Copy MinGW runtime DLLs
echo       Copying MinGW runtime DLLs...
copy /Y "%MINGW_DIR%\bin\libgcc_s_seh-1.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%MINGW_DIR%\bin\libstdc++-6.dll" "%DEPLOY_DIR%\" >nul
copy /Y "%MINGW_DIR%\bin\libwinpthread-1.dll" "%DEPLOY_DIR%\" >nul

REM Copy Qt plugins
echo       Copying Qt plugins...
mkdir "%DEPLOY_DIR%\platforms" 2>nul
copy /Y "%QT_DIR%\plugins\platforms\qwindows.dll" "%DEPLOY_DIR%\platforms\" >nul

mkdir "%DEPLOY_DIR%\styles" 2>nul
copy /Y "%QT_DIR%\plugins\styles\qwindowsvistastyle.dll" "%DEPLOY_DIR%\styles\" >nul 2>&1

echo       OK Deploy directory ready

echo.
echo [2/4] Verifying files...
set FILE_COUNT=0
for /r "%DEPLOY_DIR%" %%F in (*) do set /a FILE_COUNT+=1
echo       Total files: !FILE_COUNT!

if !FILE_COUNT! LSS 10 (
    echo [WARNING] Expected more files in deploy directory
)

echo.
echo [3/4] Building NSIS installer...
cd /d "%PROJECT_DIR%"
"%NSIS%" /V2 installer.nsi

if errorlevel 1 (
    echo [ERROR] NSIS build failed
    goto :error
)

echo.
echo [4/4] Verifying installer...
if exist "LogParser_Setup.exe" (
    for %%I in (LogParser_Setup.exe) do (
        echo       Installer size: %%~zI bytes
        echo       Location: %PROJECT_DIR%LogParser_Setup.exe
    )
) else (
    echo [ERROR] Installer not found
    goto :error
)

echo.
echo ============================================
echo   PACKAGING SUCCESSFUL!
echo ============================================
echo.
echo Installer created: LogParser_Setup.exe
echo.
goto :end

:error
echo.
echo ============================================
echo   PACKAGING FAILED
echo ============================================
echo.
pause
exit /b 1

:end
endlocal
exit /b 0
