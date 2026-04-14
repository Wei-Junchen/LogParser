# LogParser NSIS Packaging Script (PowerShell)
# Creates a Windows installer using NSIS

param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "  LogParser NSIS Installer Builder" -ForegroundColor Cyan
Write-Host "============================================`n" -ForegroundColor Cyan

$projectDir = $PSScriptRoot
$buildDir = Join-Path $projectDir "build-mingw"
$deployDir = Join-Path $buildDir "deploy"
$qtDir = "C:\Qt\6.5.3\mingw_64"
$mingwDir = "C:\Qt\Tools\mingw1120_64"
$nsis = "C:\Program Files (x86)\NSIS\makensis.exe"
$windeployqt = Join-Path $qtDir "bin\windeployqt.exe"

# Clean only
if ($Clean) {
    Write-Host "[Clean] Removing old files..." -ForegroundColor Yellow
    if (Test-Path $deployDir) { Remove-Item $deployDir -Recurse -Force }
    if (Test-Path "LogParser_Setup.exe") { Remove-Item "LogParser_Setup.exe" -Force }
    Write-Host "[Clean] Done`n" -ForegroundColor Green
    exit 0
}

# Check NSIS
if (-not (Test-Path $nsis)) {
    Write-Host "[ERROR] NSIS not found at $nsis" -ForegroundColor Red
    Write-Host "Please install NSIS from https://nsis.sourceforge.io/" -ForegroundColor Yellow
    exit 1
}

# Check windeployqt
if (-not (Test-Path $windeployqt)) {
    Write-Host "[ERROR] windeployqt not found at $windeployqt" -ForegroundColor Red
    Write-Host "Please check your Qt installation path in package.ps1" -ForegroundColor Yellow
    exit 1
}

# Check and build if needed
$exePath = Join-Path $buildDir "LogParser.exe"
if (-not (Test-Path $exePath)) {
    Write-Host "[WARNING] LogParser.exe not found, building now..." -ForegroundColor Yellow
    
    # Check if CMake is available
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmake) {
        Write-Host "[ERROR] CMake not found in PATH" -ForegroundColor Red
        exit 1
    }
    
    # Setup build environment
    $env:PATH = "$mingwDir\bin;$qtDir\bin;" + [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
    
    Write-Host "[Build] Configuring CMake..." -ForegroundColor Yellow
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
    }
    
    Push-Location $buildDir
    try {
        cmake .. -G "MinGW Makefiles" `
            -DCMAKE_BUILD_TYPE=Release `
            -DCMAKE_PREFIX_PATH="$qtDir" `
            -DCMAKE_C_COMPILER="$mingwDir\bin\gcc.exe" `
            -DCMAKE_CXX_COMPILER="$mingwDir\bin\g++.exe" `
            -DCMAKE_MAKE_PROGRAM="$mingwDir\bin\mingw32-make.exe"
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed"
        }
        
        Write-Host "[Build] Compiling..." -ForegroundColor Yellow
        cmake --build . --config Release -j
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        Write-Host "[Build] Build complete" -ForegroundColor Green
    } catch {
        Write-Host "[ERROR] Build failed: $_" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    Pop-Location
}

if (-not (Test-Path $exePath)) {
    Write-Host "[ERROR] LogParser.exe still not found after build" -ForegroundColor Red
    exit 1
}

Write-Host "[1/4] Preparing deploy directory..." -ForegroundColor Yellow

if (Test-Path $deployDir) {
    Remove-Item $deployDir -Recurse -Force
}
New-Item -ItemType Directory -Path $deployDir | Out-Null

# Copy executable
Write-Host "      Copying LogParser.exe..." -ForegroundColor Gray
Copy-Item $exePath $deployDir

# Use windeployqt to collect Qt runtime and plugins
Write-Host "      Running windeployqt..." -ForegroundColor Gray
& $windeployqt --release --no-translations "$deployDir\LogParser.exe"
if ($LASTEXITCODE -ne 0) {
    Write-Host "      [WARNING] windeployqt returned code $LASTEXITCODE, continue with manual fallback" -ForegroundColor Yellow
}

# Manual fallback for complete Qt DLLs set
Write-Host "      Copying fallback Qt DLLs..." -ForegroundColor Gray
$qtDlls = @(
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Widgets.dll",
    "Qt6Charts.dll",
    "Qt6Qml.dll",
    "Qt6QmlModels.dll",
    "Qt6OpenGL.dll",
    "Qt6OpenGLWidgets.dll",
    "Qt6Network.dll",
    "Qt6Sql.dll",
    "Qt6Concurrent.dll"
)
foreach ($dll in $qtDlls) {
    $source = Join-Path $qtDir "bin\$dll"
    if ((Test-Path $source) -and -not (Test-Path (Join-Path $deployDir $dll))) {
        Copy-Item $source $deployDir -Force
    }
}

# Copy MinGW runtime DLLs
Write-Host "      Copying MinGW runtime DLLs..." -ForegroundColor Gray
$mingwDlls = @(
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll"
)

foreach ($dll in $mingwDlls) {
    $source = Join-Path $mingwDir "bin\$dll"
    if (Test-Path $source) {
        Copy-Item $source $deployDir
    }
}

# Ensure platform plugin exists (fallback)
Write-Host "      Ensuring Qt platform plugin..." -ForegroundColor Gray
$platformsDir = Join-Path $deployDir "platforms"
New-Item -ItemType Directory -Path $platformsDir -Force | Out-Null
$qwindows = Join-Path $qtDir "plugins\platforms\qwindows.dll"
if ((Test-Path $qwindows) -and -not (Test-Path (Join-Path $platformsDir "qwindows.dll"))) {
    Copy-Item $qwindows $platformsDir -Force
}

# Copy QML plugins
Write-Host "      Copying QML plugins..." -ForegroundColor Gray
$qmlDir = Join-Path $qtDir "qml"
if (Test-Path $qmlDir) {
    $qmlDeployDir = Join-Path $deployDir "qml"
    if (-not (Test-Path $qmlDeployDir)) {
        New-Item -ItemType Directory -Path $qmlDeployDir | Out-Null
    }
    Copy-Item $qmlDir -Destination $qmlDeployDir -Recurse -Force -ErrorAction SilentlyContinue
}

# Copy image format plugins
Write-Host "      Copying image format plugins..." -ForegroundColor Gray
$imageFormatsDir = Join-Path $qtDir "plugins\imageformats"
if (Test-Path $imageFormatsDir) {
    $imgDeployDir = Join-Path $deployDir "imageformats"
    New-Item -ItemType Directory -Path $imgDeployDir -Force | Out-Null
    Copy-Item "$imageFormatsDir\*.dll" $imgDeployDir -Force -ErrorAction SilentlyContinue
}

Write-Host "      OK Deploy directory ready" -ForegroundColor Green

# Count files
Write-Host "`n[2/4] Verifying files..." -ForegroundColor Yellow
$fileCount = (Get-ChildItem $deployDir -Recurse -File).Count
Write-Host "      Total files: $fileCount" -ForegroundColor Cyan

if ($fileCount -lt 10) {
    Write-Host "      [WARNING] Expected more files in deploy directory" -ForegroundColor Yellow
}

# Build installer
Write-Host "`n[3/4] Building NSIS installer..." -ForegroundColor Yellow
Push-Location $projectDir

try {
    & $nsis /V2 "installer.nsi"
    
    if ($LASTEXITCODE -ne 0) {
        throw "NSIS build failed with exit code $LASTEXITCODE"
    }
} catch {
    Write-Host "[ERROR] NSIS build failed: $_" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# Verify installer
Write-Host "`n[4/4] Verifying installer..." -ForegroundColor Yellow
$installerPath = Join-Path $projectDir "LogParser_Setup.exe"

if (Test-Path $installerPath) {
    $installerSize = (Get-Item $installerPath).Length
    $installerSizeMB = [math]::Round($installerSize / 1MB, 2)
    
    Write-Host "      Installer size: $installerSizeMB MB ($installerSize bytes)" -ForegroundColor Cyan
    Write-Host "      Location: $installerPath" -ForegroundColor Cyan
} else {
    Write-Host "[ERROR] Installer not found at $installerPath" -ForegroundColor Red
    exit 1
}

Write-Host "`n============================================" -ForegroundColor Green
Write-Host "  PACKAGING SUCCESSFUL!" -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Green
Write-Host "`nInstaller created: LogParser_Setup.exe" -ForegroundColor White
Write-Host "Size: $installerSizeMB MB`n" -ForegroundColor White
