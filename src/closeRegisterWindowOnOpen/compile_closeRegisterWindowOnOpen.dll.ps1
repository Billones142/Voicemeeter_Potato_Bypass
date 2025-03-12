# Obtener el directorio del script
$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot

Set-Location $scriptDirectory

Remove-Item "$scriptDirectory\*.dll"

# 32 bits
Set-Location C:\msys64\mingw32\bin
& .\gcc -m32 -static -shared "$scriptDirectory\closeRegisterWindowOnOpen.dll.c" "$scriptDirectory/lib/staticMinhook/libminhook_x86.a" -lpsapi -o "$scriptDirectory/closeRegisterWindowOnOpen_x86.dll" -I"$scriptDirectory/lib/staticMinhook/minhook-1.3.3-source/include"

# 64 bits
Set-Location C:\msys64\mingw64\bin
& .\gcc -m64 -static -shared "$scriptDirectory\closeRegisterWindowOnOpen.dll.c" "$scriptDirectory/lib/staticMinhook/libminhook_x64.a" -lpsapi -o "$scriptDirectory/closeRegisterWindowOnOpen_x64.dll" -I"$scriptDirectory/lib/staticMinhook/minhook-1.3.3-source/include"

Set-Location $originalDirectory