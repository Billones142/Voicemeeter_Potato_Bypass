$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot

Set-Location C:\msys64\mingw64\bin

Remove-Item "$scriptDirectory\*.o"

windres "$scriptDirectory/resources_x86.rc" -o "$scriptDirectory/resources_x86.o"
.\windres "$scriptDirectory/resources_x64.rc" -o "$scriptDirectory/resources_x64.o"

cd $originalDirectory