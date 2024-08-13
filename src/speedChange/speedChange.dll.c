#include <windows.h>
#include ".\minhook-master\include\MinHook.h"

typedef DWORD(WINAPI* GetTickCount_t)(void);
GetTickCount_t fpGetTickCount = NULL;

DWORD WINAPI HookedGetTickCount()
{
    return fpGetTickCount() * 0.1; // Reduce la velocidad en un 50%
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
