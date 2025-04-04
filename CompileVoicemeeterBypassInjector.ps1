# Obtener el directorio del script
$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot

Set-Location $scriptDirectory
Remove-Item "VoicemeeterBypassInjector_x*.exe"

$mingw32Dir = "C:/msys64/mingw32/bin"
$mingw64Dir = "C:/msys64/mingw64/bin"

.\src\lib\staticMinhook\compileMinhook.ps1
.\src\closeRegisterWindowOnOpen\compile_closeRegisterWindowOnOpen.dll.ps1
.\src\resources\compileResources.ps1

# 32 bits
Set-Location $mingw32Dir
& .\g++ -m32 -static "$originalDirectory\src\VoicemeeterBypassInjector.cpp" "$originalDirectory\src\resources\resources_x86.o" -o "$originalDirectory\VoicemeeterBypassInjector_x86.exe"

# 64 bits
Set-Location $mingw64Dir
& .\g++ -m64 -static "$originalDirectory\src\VoicemeeterBypassInjector.cpp" "$originalDirectory\src\resources\resources_x64.o" -o "$originalDirectory\VoicemeeterBypassInjector_x64.exe"

Set-Location $originalDirectory