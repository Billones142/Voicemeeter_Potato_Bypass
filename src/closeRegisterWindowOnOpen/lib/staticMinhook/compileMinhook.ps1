$originalDirectory = $PWD
$scriptDirectory = $PSScriptRoot

cd $scriptDirectory

$staticMinhook= $scriptDirectory

Remove-Item "$staticMinhook\*.a"

cd C:\msys64\mingw64\bin # for some reason g++ 64bit doesnt work if im not in its directory

# Compilar los archivos objeto de MinHook _x86
g++ -c -m32 "$staticMinhook/minhook-1.3.3-source/src/hook.c" -o "$staticMinhook\hook_x86.o"
& g++ -c -m32 "$staticMinhook/minhook-1.3.3-source/src/buffer.c" -o "$staticMinhook\buffer_x86.o"
& g++ -c -m32 "$staticMinhook/minhook-1.3.3-source/src/hde/hde32.c" -o "$staticMinhook\hde32_x86.o"
& g++ -c -m32 "$staticMinhook/minhook-1.3.3-source/src/hde/hde64.c" -o "$staticMinhook\hde64_x86.o"
& g++ -c -m32 "$staticMinhook/minhook-1.3.3-source/src/trampoline.c" -o "$staticMinhook\trampoline_x86.o"
# Crear la biblioteca estática _x86
& ar rcs "$staticMinhook\libminhook_x86.a" "$staticMinhook\hook_x86.o" "$staticMinhook\buffer_x86.o" "$staticMinhook\trampoline_x86.o" "$staticMinhook\hde32_x86.o" "$staticMinhook\hde64_x86.o"
& ranlib "$staticMinhook\libminhook_x86.a"
# Compilar los archivos objeto de MinHook x64
.\g++.exe -c -m64 "$staticMinhook/minhook-1.3.3-source/src/hook.c" -o "$staticMinhook\hook_x64.o"
& .\g++ -c -m64 "$staticMinhook/minhook-1.3.3-source/src/buffer.c" -o "$staticMinhook\buffer_x64.o"
& .\g++ -c -m64 "$staticMinhook/minhook-1.3.3-source/src/hde/hde32.c" -o "$staticMinhook\hde32_x64.o"
& .\g++ -c -m64 "$staticMinhook/minhook-1.3.3-source/src/hde/hde64.c" -o "$staticMinhook\hde64_x64.o"
& .\g++ -c -m64 "$staticMinhook/minhook-1.3.3-source/src/trampoline.c" -o "$staticMinhook\trampoline_x64.o"
# Crear la biblioteca estática x64
& ar rcs "$staticMinhook\libminhook_x64.a" "$staticMinhook\hook_x64.o" "$staticMinhook\buffer_x64.o" "$staticMinhook\trampoline_x64.o" "$staticMinhook\hde32_x64.o" "$staticMinhook\hde64_x64.o"
& .\ranlib "$staticMinhook\libminhook_x64.a"

Remove-Item "$scriptDirectory\*.o"

# Crear la biblioteca estática
cd $originalDirectory

# Compilar tu programa enlazando con la biblioteca estática
#g++ tuarchivo.cpp -I./minhook-1.3.3-source/include -L./staticMinhook -lminhook -o tuprograma