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

# Check build
$exePath = Join-Path $buildDir "LogParser.exe"
if (-not (Test-Path $exePath)) {
    Write-Host "[ERROR] LogParser.exe not found in $buildDir" -ForegroundColor Red
    Write-Host "Please build the project first!" -ForegroundColor Yellow
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

# Copy Qt DLLs
Write-Host "      Copying Qt DLLs..." -ForegroundColor Gray
$qtDlls = @(
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Widgets.dll",
    "Qt6Charts.dll",
    "Qt6Qml.dll",
    "Qt6Network.dll",
    "Qt6OpenGL.dll",
    "Qt6OpenGLWidgets.dll"
)

foreach ($dll in $qtDlls) {
    $source = Join-Path $qtDir "bin\$dll"
    if (Test-Path $source) {
        Copy-Item $source $deployDir
    } else {
        Write-Host "      [WARNING] $dll not found" -ForegroundColor Yellow
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

# Copy Qt plugins
Write-Host "      Copying Qt plugins..." -ForegroundColor Gray

$platformsDir = Join-Path $deployDir "platforms"
New-Item -ItemType Directory -Path $platformsDir -Force | Out-Null
Copy-Item (Join-Path $qtDir "plugins\platforms\qwindows.dll") $platformsDir

$stylesDir = Join-Path $deployDir "styles"
New-Item -ItemType Directory -Path $stylesDir -Force | Out-Null
$stylePlugin = Join-Path $qtDir "plugins\styles\qwindowsvistastyle.dll"
if (Test-Path $stylePlugin) {
    Copy-Item $stylePlugin $stylesDir
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
