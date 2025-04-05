# Get script directory
$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot

Set-Location $scriptDirectory
Remove-Item "VoicemeeterBypassInjector_x*.exe"

# Check for MSYS2 installation in different possible locations
$possiblePaths = @(
    "C:/msys64",
    "C:/tools/msys64"
)

$mingw32Dir = $null
$mingw64Dir = $null

foreach ($path in $possiblePaths) {
    if (Test-Path "$path/mingw32/bin/g++.exe") {
        $mingw32Dir = "$path/mingw32/bin"
    }
    if (Test-Path "$path/mingw64/bin/g++.exe") {
        $mingw64Dir = "$path/mingw64/bin"
    }
}

if (-not $mingw32Dir) {
    Write-Error "Could not find 32-bit MinGW installation"
    exit 1
}

if (-not $mingw64Dir) {
    Write-Error "Could not find 64-bit MinGW installation"
    exit 1
}

Write-Host "Using 32-bit MinGW from: $mingw32Dir"
Write-Host "Using 64-bit MinGW from: $mingw64Dir"

.\src\lib\staticMinhook\compileMinhook.ps1
.\src\closeRegisterWindowOnOpen\compile_closeRegisterWindowOnOpen.dll.ps1
.\src\resources\compileResources.ps1

# 32 bits
Set-Location $mingw32Dir
& .\g++.exe -m32 -static "$originalDirectory\src\VoicemeeterBypassInjector.cpp" "$originalDirectory\src\resources\resources_x86.o" -o "$originalDirectory\VoicemeeterBypassInjector_x86.exe"

# 64 bits
Set-Location $mingw64Dir
& .\g++.exe -m64 -static "$originalDirectory\src\VoicemeeterBypassInjector.cpp" "$originalDirectory\src\resources\resources_x64.o" -o "$originalDirectory\VoicemeeterBypassInjector_x64.exe"

Set-Location $originalDirectory