# Obtener el directorio del script
$originalDirectory = $PWD.Path
$scriptDirectory = $PSScriptRoot

Set-Location $scriptDirectory

# 32 bits , libMinHook-x64-v141-md.lib
zig cc -target x86-windows-gnu -shared -o ".\speedChange_x32.dll" ".\speedChange.dll.c" -L".\MinHook_133_bin\bin" -l"MinHook.x64" -luser32 -lkernel32

# 64 bits , libMinHook-x64-v141-md.lib
zig cc -target x86_64-windows-gnu -shared -o ".\speedChange_x64.dll" ".\speedChange.dll.c" -L".\MinHook_133_bin\bin" -l"MinHook.x64" -luser32 -lkernel32

Set-Location $originalDirectory