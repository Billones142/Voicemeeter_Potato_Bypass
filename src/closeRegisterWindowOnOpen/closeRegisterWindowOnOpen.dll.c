#include <windows.h>
#include "./MinHook_133_bin/include/MinHook.h"

// Dirección de memoria a modificar
void *windowCloseCheckMemoryAddress = (void *)0x12AD3;                  // Reemplaza con la dirección de memoria real
BYTE windowCloseCheckNewBytes[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90}; // Reemplaza con los bytes que deseas escribir

// Definición de la función CreateWindowExW original
typedef HWND(WINAPI *CreateWindowExW_t)(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);

// Puntero a la función original
CreateWindowExW_t fpCreateWindowExW = NULL;

// Nombre de la ventana a bloquear
LPCWSTR targetWindowName = L"Nombre de la Ventana"; // Reemplaza con el nombre de la ventana que deseas bloquear

// Función "detour" que intercepta las llamadas a CreateWindowExW
HWND WINAPI DetourCreateWindowExW(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    // Verifica si el nombre de la ventana coincide con el objetivo
    if (lpWindowName && wcscmp(lpWindowName, targetWindowName) == 0)
    {
        // Bloquear la creación de la ventana
        MessageBox(NULL, L"Ventana detectada y bloqueada!", L"MinHook", MB_OK);

        // Modificar la dirección de memoria
        SIZE_T bytesWritten;
        if (WriteProcessMemory(GetCurrentProcess(), windowCloseCheckMemoryAddress, windowCloseCheckNewBytes, sizeof(windowCloseCheckNewBytes), &bytesWritten))
        {
            MessageBox(NULL, L"Memoria modificada con éxito!", L"MinHook", MB_OK);
        }
        else
        {
            MessageBox(NULL, L"Error al modificar la memoria!", L"MinHook", MB_OK);
        }

        // Retornar NULL para evitar que la ventana se cree
        return NULL;
    }

    // Llama a la función original si no coincide el nombre
    return fpCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

int main()
{
    // Inicializa MinHook
    if (MH_Initialize() != MH_OK)
    {
        return 1;
    }

    // Crea un hook para CreateWindowExW
    if (MH_CreateHook(&CreateWindowExW, &DetourCreateWindowExW, (LPVOID *)&fpCreateWindowExW) != MH_OK)
    {
        return 1;
    }

    // Habilita el hook
    if (MH_EnableHook(&CreateWindowExW) != MH_OK)
    {
        return 1;
    }

    // Tu código aquí...

    // Deshabilita el hook y desinicializa MinHook antes de salir
    MH_DisableHook(&CreateWindowExW);
    MH_Uninitialize();

    return 0;
}
