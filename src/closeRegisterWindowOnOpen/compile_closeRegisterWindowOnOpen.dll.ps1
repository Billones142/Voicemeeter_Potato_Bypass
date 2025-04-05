# Obtener el directorio del script
$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot

Set-Location $scriptDirectory

Remove-Item "$scriptDirectory\*.dll"

if (-not (Test-Path variable:mingw32Dir)) {
    $mingw32Dir = "C:/msys64/mingw32/bin"
}

if (-not (Test-Path variable:mingw64Dir)) {
    $mingw64Dir = "C:/msys64/mingw64/bin"
}

# 32 bits
Set-Location $mingw32Dir
& .\gcc -m32 -static -shared "$scriptDirectory\dllMain.c" "$scriptDirectory/../lib/staticMinhook/libminhook_x86.a" -lpsapi -o "$scriptDirectory/closeRegisterWindowOnOpen_x86.dll" -I"$scriptDirectory/../lib/staticMinhook/minhook-1.3.3-source/include"

# 64 bits
Set-Location $mingw64Dir
& .\gcc -m64 -static -shared "$scriptDirectory\dllMain.c" "$scriptDirectory/../lib/staticMinhook/libminhook_x64.a" -lpsapi -o "$scriptDirectory/closeRegisterWindowOnOpen_x64.dll" -I"$scriptDirectory/../lib/staticMinhook/minhook-1.3.3-source/include"

Set-Location $originalDirectory