$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot


Remove-Item "$scriptDirectory\*.o"

if (-not (Test-Path variable:mingw32Dir)) {
    $mingw32Dir = "C:/msys64/mingw32/bin"
}

if (-not (Test-Path variable:mingw64Dir)) {
    $mingw64Dir = "C:/msys64/mingw64/bin"
}

# 32 bits
Set-Location $mingw32Dir
& .\windres "$scriptDirectory/resources_x86.rc" -o "$scriptDirectory/resources_x86.o"

# 64 bits
Set-Location $mingw64Dir
& .\windres "$scriptDirectory/resources_x64.rc" -o "$scriptDirectory/resources_x64.o"

cd $originalDirectory