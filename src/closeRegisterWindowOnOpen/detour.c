#include <windows.h>
#include <stdint.h>

#include "voicemeeterVersions.h"

// Estructura para manejar nuestro detour
typedef struct
{
    void *originalFunction;   // Dirección de la función original
    void *detourFunction;     // Dirección de nuestra función personalizada
    void *trampolineFunction; // Espacio donde guardaremos los bytes originales
    uint8_t *originalBytes;   // Copia de los bytes originales
    size_t hookSize;          // Tamaño del hook (debe ser al menos 5 bytes en x86/x64)
} DetourInfo;

// Función para crear un detour
BOOL CreateDetour(HANDLE hProcess, ChangeAdressTo *modifyerData)
{
    DetourInfo *detourInfo= &modifyerData->detourInfo;
    void *targetAddress= modifyerData->relativeAddress;
    uint8_t *newCode= modifyerData->newValue;
    size_t newCodeSize= modifyerData->newValueSize;
    
    DWORD oldProtect;
    size_t hookSize = 5; // Mínimo para un JMP en x86/x64

    // 1. Guardar bytes originales
    detourInfo->originalFunction = targetAddress;
    detourInfo->hookSize = hookSize;
    detourInfo->originalBytes = (uint8_t *)malloc(hookSize);

    if (!ReadProcessMemory(hProcess, targetAddress, detourInfo->originalBytes, hookSize, NULL))
    {
        printf("Error al leer la memoria original: %lu\n", GetLastError());
        free(detourInfo->originalBytes);
        return FALSE;
    }

    // 2. Asignar memoria para el trampoline (código nuevo)
    detourInfo->trampolineFunction = VirtualAllocEx(
        hProcess,
        NULL,
        newCodeSize + hookSize + 5, // Tamaño para: código nuevo + código original + JMP de retorno
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE);

    if (!detourInfo->trampolineFunction)
    {
        printf("Error al asignar memoria para el trampoline: %lu\n", GetLastError());
        free(detourInfo->originalBytes);
        return FALSE;
    }

    // 3. Escribir el código nuevo en el trampoline
    if (!WriteProcessMemory(hProcess, detourInfo->trampolineFunction, newCode, newCodeSize, NULL))
    {
        printf("Error al escribir el código nuevo: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, detourInfo->trampolineFunction, 0, MEM_RELEASE);
        free(detourInfo->originalBytes);
        return FALSE;
    }

    // 4. Agregar los bytes originales después del código nuevo
    if (!WriteProcessMemory(hProcess, (uint8_t *)detourInfo->trampolineFunction + newCodeSize,
                            detourInfo->originalBytes, hookSize, NULL))
    {
        printf("Error al escribir los bytes originales: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, detourInfo->trampolineFunction, 0, MEM_RELEASE);
        free(detourInfo->originalBytes);
        return FALSE;
    }

    // 5. Agregar un JMP de retorno al final del trampoline
    uint8_t jmpBack[5] = {0xE9, 0x00, 0x00, 0x00, 0x00}; // JMP relativo

    // Calcular el offset para el salto de retorno
    uint32_t returnOffset = (uint32_t)((uint8_t *)targetAddress + hookSize -
                                       ((uint8_t *)detourInfo->trampolineFunction + newCodeSize + hookSize + 5));

    memcpy(jmpBack + 1, &returnOffset, sizeof(uint32_t));

    if (!WriteProcessMemory(hProcess, (uint8_t *)detourInfo->trampolineFunction + newCodeSize + hookSize,
                            jmpBack, 5, NULL))
    {
        printf("Error al escribir el JMP de retorno: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, detourInfo->trampolineFunction, 0, MEM_RELEASE);
        free(detourInfo->originalBytes);
        return FALSE;
    }

    // 6. Cambiar la protección de la memoria original para poder escribir
    if (!VirtualProtectEx(hProcess, targetAddress, hookSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        printf("Error al cambiar la protección de memoria: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, detourInfo->trampolineFunction, 0, MEM_RELEASE);
        free(detourInfo->originalBytes);
        return FALSE;
    }

    // 7. Escribir el JMP al principio del código original para que salte a nuestro trampoline
    uint8_t jmpToDetour[5] = {0xE9, 0x00, 0x00, 0x00, 0x00}; // JMP relativo

    // Calcular el offset para el salto al trampoline
    uint32_t detourOffset = (uint32_t)((uint8_t *)detourInfo->trampolineFunction - ((uint8_t *)targetAddress + 5));

    memcpy(jmpToDetour + 1, &detourOffset, sizeof(uint32_t));

    if (!WriteProcessMemory(hProcess, targetAddress, jmpToDetour, hookSize, NULL))
    {
        printf("Error al escribir el JMP de detour: %lu\n", GetLastError());
        VirtualProtectEx(hProcess, targetAddress, hookSize, oldProtect, &oldProtect);
        VirtualFreeEx(hProcess, detourInfo->trampolineFunction, 0, MEM_RELEASE);
        free(detourInfo->originalBytes);
        return FALSE;
    }

    // 8. Restaurar la protección original
    VirtualProtectEx(hProcess, targetAddress, hookSize, oldProtect, &oldProtect);

    return TRUE;
}

// Función para eliminar el detour y restaurar el código original
BOOL RemoveDetour(HANDLE hProcess, DetourInfo *detourInfo)
{
    DWORD oldProtect;

    // Cambiar la protección de la memoria original
    if (!VirtualProtectEx(hProcess, detourInfo->originalFunction, detourInfo->hookSize,
                          PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        return FALSE;
    }

    // Restaurar los bytes originales
    if (!WriteProcessMemory(hProcess, detourInfo->originalFunction, detourInfo->originalBytes,
                            detourInfo->hookSize, NULL))
    {
        VirtualProtectEx(hProcess, detourInfo->originalFunction, detourInfo->hookSize,
                         oldProtect, &oldProtect);
        return FALSE;
    }

    // Restaurar la protección original
    VirtualProtectEx(hProcess, detourInfo->originalFunction, detourInfo->hookSize,
                     oldProtect, &oldProtect);

    // Liberar la memoria del trampoline
    VirtualFreeEx(hProcess, detourInfo->trampolineFunction, 0, MEM_RELEASE);

    // Liberar la memoria de los bytes originales
    free(detourInfo->originalBytes);

    return TRUE;
}

// Ejemplo de uso para inyectar "mov [r12+00000A68],0"
//int main()
//{
//    // Opcodes para "mov qword ptr [r12+0A68h], 0"
//    // Nota: Esta es una representación aproximada, puede variar según el ensamblador exacto
//    // 49 C7 84 24 68 0A 00 00 00 00 00 00
//    uint8_t newInstructions[] = {
//        0x49, 0xC7, 0x84, 0x24, 0x68, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    // Obtener el proceso objetivo (ejemplo)
//    DWORD processId = 1234; // Reemplazar con el PID real
//    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
//
//    if (!hProcess)
//    {
//        printf("No se pudo abrir el proceso: %lu\n", GetLastError());
//        return 1;
//    }
//
//    // Dirección a modificar (ejemplo)
//    void *targetAddress = (void *)0x00400000; // Reemplazar con la dirección real
//
//    // Crear el detour
//    DetourInfo detourInfo;
//    if (CreateDetour(hProcess, targetAddress, newInstructions, sizeof(newInstructions), &detourInfo))
//    {
//        printf("Detour creado exitosamente.\n");
//
//        // Hacer algo más...
//
//        // Cuando terminemos, podemos restaurar el código original
//        if (RemoveDetour(hProcess, &detourInfo))
//        {
//            printf("Detour eliminado exitosamente.\n");
//        }
//        else
//        {
//            printf("Error al eliminar el detour.\n");
//        }
//    }
//    else
//    {
//        printf("Error al crear el detour.\n");
//    }
//
//    CloseHandle(hProcess);
//
//    __DATE__;
//    __TIME__;
//    return 0;
//}