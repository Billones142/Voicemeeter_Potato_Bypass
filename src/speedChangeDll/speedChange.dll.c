#include <windows.h>
#include "./MinHook_133_bin/include/MinHook.h"

#define SPEED 0.1

typedef DWORD(WINAPI* GetTickCount_t)(void);
GetTickCount_t fpGetTickCount = NULL;

DWORD WINAPI HookedGetTickCount()
{
    return fpGetTickCount() * SPEED;
}

int main()
{
    // Inicializar MinHook
    if (MH_Initialize() != MH_OK)
    {
        return 1;
    }

    // Crear gancho para GetTickCount
    if (MH_CreateHook(&GetTickCount, &HookedGetTickCount, (LPVOID*)&fpGetTickCount) != MH_OK)
    {
        return 1;
    }

    // Habilitar el gancho
    if (MH_EnableHook(&GetTickCount) != MH_OK)
    {
        return 1;
    }

    // Tu código aquí...

    // Deshabilitar y desinicializar el gancho antes de salir
    MH_DisableHook(&GetTickCount);
    MH_Uninitialize();

    return 0;
}