# Obtener el directorio del script
$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot

Set-Location $scriptDirectory

# 32 bits
#& zig cc -target x86-windows-gnu -shared -o ".\closeRegisterWindowOnOpen.dll" ".\closeRegisterWindowOnOpen.dll.c" -L".\MinHook_133_bin\bin" -l"MinHook.x86" -luser32 -lkernel32

# 64 bits
& zig cc -target x86_64-windows-gnu -shared -o ".\closeRegisterWindowOnOpen.dll" ".\closeRegisterWindowOnOpen.dll.c" -L".\MinHook_133_bin\bin" -l"MinHook.x64" -luser32 -lkernel32

#& python3 .\makeEmbededHFile.py

Set-Location $originalDirectory