#include <windows.h>
#include <iostream>
#include <MinHook.h>

// Tipo de puntero a función para MessageBoxW
typedef int (WINAPI *MessageBoxW_t)(HWND, LPCWSTR, LPCWSTR, UINT);

// Original MessageBoxW
MessageBoxW_t fpOriginalMessageBoxW = nullptr;

// Nuestra versión de MessageBoxW
int WINAPI DetourMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) {
    // Modificamos el texto
    return fpOriginalMessageBoxW(hWnd, L"¡Texto interceptado!", lpCaption, uType);
}

int main() {
    // Inicializar MinHook
    if (MH_Initialize() != MH_OK) {
        std::cout << "Error al inicializar MinHook" << std::endl;
        return 1;
    }

    // Obtenemos la dirección de MessageBoxW
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    // Convertir explícitamente FARPROC a void*
    void* pMessageBoxW = (void*)GetProcAddress(hUser32, "MessageBoxW");

    // Creamos el hook con las conversiones correctas
    if (MH_CreateHook(pMessageBoxW, 
                     (LPVOID)&DetourMessageBoxW, 
                     (LPVOID*)&fpOriginalMessageBoxW) != MH_OK) {
        std::cout << "Error al crear el hook" << std::endl;
        return 1;
    }

    // Habilitamos el hook
    if (MH_EnableHook(pMessageBoxW) != MH_OK) {
        std::cout << "Error al habilitar el hook" << std::endl;
        return 1;
    }

    // Llamamos a MessageBoxW (será interceptado)
    MessageBoxW(NULL, L"Texto original", L"Título", MB_OK);

    // Deshabilitamos y limpiamos
    MH_DisableHook(pMessageBoxW);
    MH_Uninitialize();

    return 0;
}