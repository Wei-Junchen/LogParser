param(
    [switch]$SkipInstall,
    [switch]$Clean,
    [string]$QtVersion = "6.5.3",
    [string]$QtPath = "C:\Qt"
)

$ErrorActionPreference = "Stop"

Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "  LogParser - Qt MinGW Build System" -ForegroundColor Cyan
Write-Host "============================================`n" -ForegroundColor Cyan

if ($Clean) {
    Write-Host "[Clean] Removing build directory..." -ForegroundColor Yellow
    if (Test-Path "build-mingw") { Remove-Item "build-mingw" -Recurse -Force }
    Write-Host "[Clean] Done`n" -ForegroundColor Green
    exit 0
}

Write-Host "[1/5] Checking CMake..." -ForegroundColor Yellow
$cmakeExists = $false
try {
    $cmakeVersion = cmake --version 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "      OK CMake installed: $($cmakeVersion[0])" -ForegroundColor Green
        $cmakeExists = $true
    }
} catch {}

if (-not $cmakeExists) {
    Write-Host "      CMake not found, installing..." -ForegroundColor Yellow
    try {
        winget install --id Kitware.CMake --silent --accept-source-agreements --accept-package-agreements
        Write-Host "      OK CMake installed" -ForegroundColor Green
        Write-Host "      Please close and reopen terminal, then run this script again" -ForegroundColor Yellow
        exit 0
    } catch {
        Write-Host "      Failed to auto-install CMake" -ForegroundColor Red
        Write-Host "      Please download from: https://cmake.org/download/" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "`n[2/5] Checking Qt MinGW..." -ForegroundColor Yellow

$qtInstallPath = ""
$possiblePaths = @(
    "$QtPath\$QtVersion\mingw_64",
    "$QtPath\$QtVersion\mingw1120_64",
    "$QtPath\6.5.3\mingw_64",
    "$QtPath\6.6.0\mingw_64",
    "C:\Qt\$QtVersion\mingw_64",
    "C:\Qt\6.5.3\mingw_64"
)

foreach ($path in $possiblePaths) {
    if (Test-Path "$path\bin\qmake.exe") {
        $qtInstallPath = $path
        Write-Host "      OK Found Qt: $qtInstallPath" -ForegroundColor Green
        break
    }
}

if (-not $qtInstallPath -and -not $SkipInstall) {
    Write-Host "      Qt MinGW not found" -ForegroundColor Yellow
    Write-Host "`nPlease choose installation method:" -ForegroundColor Cyan
    Write-Host "  1. Download official online installer (recommended)" -ForegroundColor White
    Write-Host "  2. Use aqt command-line tool (fast, requires Python)" -ForegroundColor White
    Write-Host "  3. I have it installed, specify path manually" -ForegroundColor White
    
    $choice = Read-Host "`nEnter option (1/2/3)"
    
    switch ($choice) {
        "1" {
            Write-Host "`nDownloading Qt online installer..." -ForegroundColor Yellow
            $installerPath = "$env:TEMP\qt-installer.exe"
            try {
                Invoke-WebRequest -Uri "https://d13lb3tujbc8s0.cloudfront.net/onlineinstallers/qt-unified-windows-x64-online.exe" -OutFile $installerPath
                Write-Host "OK Downloaded, starting installer..." -ForegroundColor Green
                Write-Host "`nImportant:" -ForegroundColor Cyan
                Write-Host "  - Register or login Qt account (free)" -ForegroundColor White
                Write-Host "  - Select Qt $QtVersion" -ForegroundColor White
                Write-Host "  - Check: MinGW 64-bit" -ForegroundColor White
                Write-Host "  - Check: Qt Charts (required)" -ForegroundColor White
                Write-Host "  - Install path: $QtPath`n" -ForegroundColor White
                
                Start-Process $installerPath -Wait
                
                Write-Host "`nAfter installation completes, please run this script again" -ForegroundColor Yellow
                exit 0
            } catch {
                Write-Host "Download failed: $_" -ForegroundColor Red
                exit 1
            }
        }
        "2" {
            Write-Host "`nInstalling Qt with aqt..." -ForegroundColor Yellow
            
            try {
                python --version 2>$null | Out-Null
                if ($LASTEXITCODE -ne 0) { throw }
            } catch {
                Write-Host "Python not installed, cannot use aqt" -ForegroundColor Red
                Write-Host "Please choose option 1 or install Python" -ForegroundColor Yellow
                exit 1
            }
            
            Write-Host "Installing aqtinstall..." -ForegroundColor Yellow
            python -m pip install aqtinstall --quiet
            
            Write-Host "Downloading Qt $QtVersion MinGW..." -ForegroundColor Yellow
            $installDir = "C:\Qt"
            
            python -m aqt install-qt windows desktop $QtVersion win64_mingw -m qtcharts --outputdir $installDir
            python -m aqt install-tool windows desktop tools_mingw90 --outputdir $installDir
            
            $qtInstallPath = "$installDir\$QtVersion\mingw_64"
            
            if (Test-Path "$qtInstallPath\bin\qmake.exe") {
                Write-Host "OK Qt installed: $qtInstallPath" -ForegroundColor Green
            } else {
                Write-Host "Installation failed" -ForegroundColor Red
                exit 1
            }
        }
        "3" {
            $manualPath = Read-Host "Enter Qt MinGW full path (e.g., C:\Qt\6.5.3\mingw_64)"
            if (Test-Path "$manualPath\bin\qmake.exe") {
                $qtInstallPath = $manualPath
                Write-Host "OK Found Qt: $qtInstallPath" -ForegroundColor Green
            } else {
                Write-Host "Invalid path or qmake.exe not found" -ForegroundColor Red
                exit 1
            }
        }
        default {
            Write-Host "Invalid option" -ForegroundColor Red
            exit 1
        }
    }
}

if (-not $qtInstallPath) {
    Write-Host "Cannot find Qt installation" -ForegroundColor Red
    exit 1
}

$mingwPath = ""
$mingwBin = "$qtInstallPath\bin"

if (Test-Path "$mingwBin\g++.exe") {
    $mingwPath = $mingwBin
} else {
    $possibleMinGW = @(
        "C:\Qt\Tools\mingw1120_64\bin",
        "C:\Qt\Tools\mingw900_64\bin",
        "C:\mingw64\bin"
    )
    
    foreach ($path in $possibleMinGW) {
        if (Test-Path "$path\g++.exe") {
            $mingwPath = $path
            break
        }
    }
}

if (-not $mingwPath) {
    Write-Host "MinGW compiler not found" -ForegroundColor Red
    Write-Host "Usually at C:\Qt\Tools\mingw*_64\bin" -ForegroundColor Yellow
    exit 1
}

Write-Host "      OK MinGW: $mingwPath" -ForegroundColor Green

Write-Host "`n[3/5] Configuring CMake..." -ForegroundColor Yellow

$buildDir = "build-mingw"
if (Test-Path $buildDir) {
    Remove-Item $buildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $buildDir | Out-Null

$env:PATH = "$mingwPath;$qtInstallPath\bin;$env:PATH"

Push-Location $buildDir
try {
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$qtInstallPath" -DCMAKE_C_COMPILER="$mingwPath\gcc.exe" -DCMAKE_CXX_COMPILER="$mingwPath\g++.exe"
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    Write-Host "      OK CMake configured" -ForegroundColor Green
    
    Write-Host "`n[4/5] Building..." -ForegroundColor Yellow
    cmake --build . --config Release -j
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    
    Write-Host "      OK Build successful" -ForegroundColor Green
    
    Write-Host "`n[5/5] Deploying..." -ForegroundColor Yellow
    
    $deployDir = "deploy"
    if (-not (Test-Path $deployDir)) {
        New-Item -ItemType Directory -Path $deployDir | Out-Null
    }
    
    Copy-Item "LogParser.exe" "$deployDir\" -Force
    
    & "$qtInstallPath\bin\windeployqt.exe" --release --no-translations "$deployDir\LogParser.exe"
    
    Write-Host "      OK Deployed" -ForegroundColor Green
    
    $exePath = Resolve-Path "$buildDir\$deployDir\LogParser.exe"
    
    Write-Host "`n============================================" -ForegroundColor Green
    Write-Host "  BUILD SUCCESSFUL!" -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "Executable: $exePath" -ForegroundColor White
    Write-Host "`nTo run:" -ForegroundColor Cyan
    Write-Host "  cd $buildDir\$deployDir" -ForegroundColor White
    Write-Host "  .\LogParser.exe`n" -ForegroundColor White
    
    $run = Read-Host "Run now? (y/n)"
    if ($run -eq "y" -or $run -eq "Y") {
        Start-Process $exePath
    }
    
} catch {
    Write-Host "`n============================================" -ForegroundColor Red
    Write-Host "  BUILD FAILED" -ForegroundColor Red
    Write-Host "============================================" -ForegroundColor Red
    Write-Host "Error: $_" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location
