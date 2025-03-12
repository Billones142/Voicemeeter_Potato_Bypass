/**
 * This dll is only compatible with the x86 or x64 version of voicemeeter depending on the compiled version, uses preprocesor variables to determine the versions in array voicemeeterVariants
 */

#include <stdio.h>
#include "Windows.h"
#include "string.h"
#include <MinHook.h>
#include <Psapi.h>

#include "delayedFunction.c"

// #define SPEEDCHANGE

#ifdef SPEEDCHANGE
#include "speedchange.c" //TODO: not ready
#endif

#if _WIN64
#warning compiling closeRegisterWindowOnOpen for 64bits
#else
#warning compiling closeRegisterWindowOnOpen for 32bits
#endif

void *pCreateWindowExA;

typedef struct ChangeAdressTo
{
    void *relativeAddress; // address where the newValue will start to be written
    BYTE newValue[15];     // i dont expect to need more than 15 bytes
    size_t newValueSize;   // amount of bytes that has to be written
} ChangeAdressTo;

// Struct representing possible addresses for the variable and its modifying function
typedef struct VoicemeeterInit
{
    char *processName;
    ChangeAdressTo timeLeftVariable; // controls when you can close the registration window
    ChangeAdressTo timeFunction;     // every second decreases variable by 1
    ChangeAdressTo windowVariable;   //
    ChangeAdressTo windowFunction;   // Allows to close the window by ignoring if the time has reached 0
} VoicemeeterInit;

int voicemeeterVersionIndex = -1;
// variants of voicemeeter
const VoicemeeterInit voicemeeterVariants[] = {
#if _WIN64
    {
        "voicemeeter8x64.exe",                                                           // Voicemeeter Potato x64
        {(void *)0x156858, {0x0, 0x0, 0x0, 0x0}, 4},                                     // 1 second
        {(void *)0x13D2E, {0x90, 0xC7, 0x84, 0x24, 0x68, 0x0A, 0x0, 0x0, 0x0, 0x0}, 10}, // automatically sets variable to 0
        {(void *)0x0, {0x0}, 0},                                                         // not implemented
        {(void *)0x1365B, {0x90, 0x90}, 2},                                              // remplazar la funcion que chequea si se puede cerrar la ventana por nop's
    },
#else
    {
        "voicemeeter8.exe", // Voicemeeter Potato x86
        {(void *)0x13B518, {0x0, 0x0, 0x0, 0x0}, 4},
        {(void *)0x13CEE, {0xC7, 0x87, 0x38, 0x0A, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90}, 10},
        {(void *)0x0, {0x0}, 0}, // not implemented
        {(void *)0x0, {0x0}, 0}, // not implemented
    },
#endif
};

const VoicemeeterInit *getCurrentVariant()
{
    return &voicemeeterVariants[voicemeeterVersionIndex];
}

BOOL closeWindow(const char *windowTitle)
{
    HWND hWnd = FindWindow(NULL, windowTitle);

    if (hWnd == NULL)
    {
        return FALSE;
    }

    SendMessage(hWnd, WM_CLOSE, 0, 0);
    return TRUE;
}

void nonBlocking_Messagebox(const char *message, const char *windowName)
{
    HWND hwnd = CreateWindowEx(0, "STATIC", windowName,
                               WS_VISIBLE | WS_POPUP,
                               100, 100, 400, 300, NULL, NULL, NULL, NULL);
    SetWindowText(hwnd, message);
}

/**
 * returns true if there was an error
 */
BOOL writeStruct_ChangeAdressTo(const ChangeAdressTo *currentJob)
{
    if (currentJob->newValueSize == 0)
    {
        nonBlocking_Messagebox("ChangeAdressTo struct not finished", "Bypass writeStruct_ChangeAdressTo");
        return FALSE;
    }

    // Variables para VirtualProtect
    DWORD oldProtect;
    LPVOID absoluteAddress;
    char errorMsg[256];

    // Obtener el handle del módulo de Voicemeeter
    HMODULE hModule = GetModuleHandle(NULL); // GetModuleHandle(NULL) obtiene el módulo principal
    if (!hModule)
    {
        sprintf(errorMsg, "No se pudo obtener el handle del módulo. Error: %lu", GetLastError());
        nonBlocking_Messagebox(errorMsg, "Bypass GetModuleHandle");
        return TRUE;
    }

    // Calcular dirección absoluta sumando el offset a la dirección base del módulo
    absoluteAddress = (LPVOID)((BYTE *)hModule + (DWORD_PTR)currentJob->relativeAddress);

    // Cambiar permisos de memoria a PAGE_EXECUTE_READWRITE
    if (!VirtualProtect(absoluteAddress, currentJob->newValueSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        DWORD errorCode = GetLastError();
        sprintf(errorMsg, "Error al cambiar protección de memoria. Código: %lu, Dirección: %p, Tamaño: %zu",
                errorCode, absoluteAddress, currentJob->newValueSize);
        nonBlocking_Messagebox(errorMsg, "Bypass VirtualProtect");
        return TRUE; // Hubo error
    }

    SIZE_T bytesWritten;

    // Modificar la dirección de memoria
    BOOL writeResult = WriteProcessMemory(GetCurrentProcess(),
                                          absoluteAddress,
                                          currentJob->newValue,
                                          currentJob->newValueSize,
                                          &bytesWritten);

    if (!writeResult)
    {
        DWORD errorCode = GetLastError();
        sprintf(errorMsg, "Error al escribir memoria. Código: %lu, Bytes escritos: %zu/%zu",
                errorCode, bytesWritten, currentJob->newValueSize);
        nonBlocking_Messagebox(errorMsg, "Bypass WriteProcessMemory");
    }

    // Restaurar la protección original
    DWORD dummy;
    VirtualProtect(absoluteAddress, currentJob->newValueSize, oldProtect, &dummy);

    // Verificar si la escritura fue exitosa
    return !writeResult || (bytesWritten < currentJob->newValueSize);
}

void write_timeLeftVariable()
{
    if (writeStruct_ChangeAdressTo(&getCurrentVariant()->timeLeftVariable))
    {
        nonBlocking_Messagebox("Error al modificar la memoria!", "Bypass write_timeLeftVariable");
    }
}

void write_timeFunction()
{
    if (writeStruct_ChangeAdressTo(&getCurrentVariant()->timeFunction))
    {
        nonBlocking_Messagebox("Error al modificar la memoria!", "Bypass write_timeFunction");
    }
}

/**
 * Allows to close the window by ignoring if the time has reached 0
 */
void write_windowFunction()
{
    if (writeStruct_ChangeAdressTo(&getCurrentVariant()->windowFunction))
    {
        nonBlocking_Messagebox("Error al modificar la memoria!", "Bypass write_windowFunction");
    }
}

void write_windowVariable()
{
    if (writeStruct_ChangeAdressTo(&getCurrentVariant()->windowVariable))
    {
        nonBlocking_Messagebox("Error al modificar la memoria!", "Bypass write_windowVariable");
    }
}

char *getHostProcessFilename()
{
    static CHAR filePath[MAX_PATH];
    static CHAR filename[MAX_PATH];
    DWORD procesoPID;
    DWORD tamano = MAX_PATH;

    // Inicializar strings
    filePath[0] = '\0';
    filename[0] = '\0';

    // Obtiene el ID del proceso actual
    procesoPID = GetCurrentProcessId();

    // Abre un handle al proceso con el ID obtenido
    HANDLE hProceso = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procesoPID);

    if (hProceso == NULL)
    {
        return filename; // Retorna string vacío en caso de error
    }

    // Obtiene la ruta completa del ejecutable
    if (GetModuleFileNameEx(hProceso, NULL, filePath, tamano))
    {
        // Extrae solo el nombre del archivo (sin la ruta)
        char *ptrNombreSolo = strrchr(filePath, '\\');
        if (ptrNombreSolo != NULL)
        {
            ptrNombreSolo++; // Avanza después de la última barra invertida
            strcpy(filename, ptrNombreSolo);
        }
        else
        {
            strcpy(filename, filePath);
        }
    }

    CloseHandle(hProceso);
    return filename;
}

// Definición de la función CreateWindowExA original
typedef HWND(WINAPI *CreateWindowExA_t)(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);

CreateWindowExA_t fpCreateWindowExA = NULL;

// Nombre de la ventana a bloquear
const char *registrationWindowName = "About / Registration info...";

void close_registrationWindow()
{
    closeWindow(registrationWindowName);
}

void close_mainWindow()
{
    closeWindow("Voicemeeter");
}

// Función "detour" que intercepta las llamadas a CreateWindowExA
HWND WINAPI DetourCreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
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
    // nonBlocking_Messagebox("Se abrio una ventana","Bypass DetourCreateWindowExA"); // crashes
    if (lpWindowName && strcmp(lpWindowName, "Activate") == 0)
    {
        executeSimpleAfterDelay(close_registrationWindow, 50);
        executeSimpleAfterDelay(close_registrationWindow, 500); // just in case it was too fast
        executeSimpleAfterDelay(close_mainWindow, 500);
        return NULL;
    }
    return fpCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

int getVersionIndex()
{
    int foundIndex = -1;
    const char *executableName = getHostProcessFilename();
    for (size_t i = 0; i < (sizeof(voicemeeterVariants) / sizeof(VoicemeeterInit)); i++)
    {
        if (strcmp(executableName, voicemeeterVariants[i].processName) == 0)
        {
            foundIndex = i;
        }
    }

    return foundIndex;
}

int main()
{
    // Inicializa MinHook
    if (MH_Initialize() != MH_OK)
    {
        nonBlocking_Messagebox("Failed to initialize MinHook", "Error");
        return 1;
    }

    // Obtener la dirección real de la función CreateWindowExW
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (!hUser32)
    {
        nonBlocking_Messagebox("Failed to get User32.dll handle", "Error");
        MH_Uninitialize();
        return 1;
    }

    pCreateWindowExA = (void *)GetProcAddress(hUser32, "CreateWindowExA");
    if (!pCreateWindowExA)
    {
        nonBlocking_Messagebox("Failed to get CreateWindowExW address", "Error");
        MH_Uninitialize();
        return 1;
    }

    // Crea un hook para CreateWindowExW
    MH_STATUS status = MH_CreateHook(pCreateWindowExA,
                                     (LPVOID)&DetourCreateWindowExA,
                                     (LPVOID *)&fpCreateWindowExA);

    if (status != MH_OK)
    {
        char errorMsg[256];
        sprintf(errorMsg, "Failed to create hook: %d", status);
        nonBlocking_Messagebox(errorMsg, "Error");
        MH_Uninitialize();
        return 1;
    }

    voicemeeterVersionIndex = getVersionIndex();
    if (voicemeeterVersionIndex < 0)
    {
        nonBlocking_Messagebox("Voicemeeter version not found", "Bypass main");
        MH_Uninitialize();
        return 1;
    }

    // Habilita el hook
    status = MH_EnableHook(pCreateWindowExA);
    if (status != MH_OK)
    {
        MH_Uninitialize();
        return 1;
    }

    // write_timeFunction();
    // write_timeLeftVariable();
    write_windowFunction();
    // write_windowVariable();

    closeWindow("Installation Warning...");
    closeWindow(registrationWindowName);
    executeSimpleAfterDelay(close_mainWindow, 500);

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        main();
#ifdef SPEEDCHANGE
        initializeSpeedChange();
        changeSpeed(3000);
#endif
        break;
    case DLL_PROCESS_DETACH:
        // Liberar recursos
        MH_DisableHook(pCreateWindowExA);
        // uninitializeSpeedChange();
        MH_Uninitialize();
        break;
    }
    return TRUE;
}