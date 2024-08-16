# Obtener el directorio del script
$originalDirectory = $PWD.Path
$scriptDirectory = $PSScriptRoot

Set-Location $scriptDirectory

# 64 bits , libMinHook-x64-v141-md.lib
zig cc -shared -o ".\speedChange_x64.dll" ".\speedChange.dll.c" -L".\MinHook_133_bin\bin" -l"MinHook.x64" -luser32 -lkernel32

Set-Location $originalDirectory