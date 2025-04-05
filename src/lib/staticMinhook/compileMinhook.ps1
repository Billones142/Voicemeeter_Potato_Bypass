$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot

cd $scriptDirectory

Remove-Item "$scriptDirectory\*.a"

if (-not (Test-Path variable:mingw32Dir)) {
    $mingw32Dir = "C:/msys64/mingw32/bin"
}

if (-not (Test-Path variable:mingw64Dir)) {
    $mingw64Dir = "C:/msys64/mingw64/bin"
}

# 32 bits
Set-Location $mingw32Dir
echo $PWD
# Compilar los archivos objeto de MinHook x86
.\gcc -c -m32 "$scriptDirectory/minhook-1.3.3-source/src/hook.c" -o "$scriptDirectory\hook_x86.o"
& .\gcc -c -m32 "$scriptDirectory/minhook-1.3.3-source/src/buffer.c" -o "$scriptDirectory\buffer_x86.o"
& .\gcc -c -m32 "$scriptDirectory/minhook-1.3.3-source/src/hde/hde32.c" -o "$scriptDirectory\hde32_x86.o"
& .\gcc -c -m32 "$scriptDirectory/minhook-1.3.3-source/src/hde/hde64.c" -o "$scriptDirectory\hde64_x86.o"
& .\gcc -c -m32 "$scriptDirectory/minhook-1.3.3-source/src/trampoline.c" -o "$scriptDirectory\trampoline_x86.o"
# Crear la biblioteca estática x86
& .\ar rcs "$scriptDirectory\libminhook_x86.a" "$scriptDirectory\hook_x86.o" "$scriptDirectory\buffer_x86.o" "$scriptDirectory\trampoline_x86.o" "$scriptDirectory\hde32_x86.o" "$scriptDirectory\hde64_x86.o"
& .\ranlib "$scriptDirectory\libminhook_x86.a"

# 64 bits
Set-Location $mingw64Dir
echo $PWD
# Compilar los archivos objeto de MinHook x64
.\gcc.exe -c -m64 "$scriptDirectory/minhook-1.3.3-source/src/hook.c" -o "$scriptDirectory\hook_x64.o"
& .\gcc -c -m64 "$scriptDirectory/minhook-1.3.3-source/src/buffer.c" -o "$scriptDirectory\buffer_x64.o"
& .\gcc -c -m64 "$scriptDirectory/minhook-1.3.3-source/src/hde/hde32.c" -o "$scriptDirectory\hde32_x64.o"
& .\gcc -c -m64 "$scriptDirectory/minhook-1.3.3-source/src/hde/hde64.c" -o "$scriptDirectory\hde64_x64.o"
& .\gcc -c -m64 "$scriptDirectory/minhook-1.3.3-source/src/trampoline.c" -o "$scriptDirectory\trampoline_x64.o"
# Crear la biblioteca estática x64
& .\ar rcs "$scriptDirectory\libminhook_x64.a" "$scriptDirectory\hook_x64.o" "$scriptDirectory\buffer_x64.o" "$scriptDirectory\trampoline_x64.o" "$scriptDirectory\hde32_x64.o" "$scriptDirectory\hde64_x64.o"
& .\ranlib "$scriptDirectory\libminhook_x64.a"

Remove-Item "$scriptDirectory\*.o"

# Crear la biblioteca estática
cd $originalDirectory

# Compilar tu programa enlazando con la biblioteca estática
#gcc tuarchivo.cpp -I./minhook-1.3.3-source/include -L./staticMinhook -lminhook -o tuprograma