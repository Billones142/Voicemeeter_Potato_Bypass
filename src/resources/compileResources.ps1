$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot


Remove-Item "$scriptDirectory\*.o"

# 32 bits
Set-Location C:\msys64\mingw32\bin
& .\windres "$scriptDirectory/resources_x86.rc" -o "$scriptDirectory/resources_x86.o"

# 64 bits
Set-Location C:\msys64\mingw64\bin
& .\windres "$scriptDirectory/resources_x64.rc" -o "$scriptDirectory/resources_x64.o"

cd $originalDirectory