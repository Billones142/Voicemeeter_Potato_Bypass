/**
 * This dll is only compatible with the x86 or x64 version of voicemeeter depending on the compiled version, uses preprocesor variables to determine the versions in array voicemeeterVariants
 */

#include <stdio.h>
#include "Windows.h"
#include "string.h"
#include <MinHook.h>
#include <Psapi.h>

#include "delayedFunction.c"

#if _WIN64
#warning compiling closeRegisterWindowOnOpen for 64bits
#else
#warning compiling closeRegisterWindowOnOpen for 32bits
#endif

void *pCreateWindowExW;

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
    ChangeAdressTo windowFunction;   // opens the registration window
} VoicemeeterInit;

// i
int voicemeeterVersionIndex = -1;
// variants of voicemeeter
const VoicemeeterInit voicemeeterVariants[] = {
#if _WIN64
    {
        "voicemeeter8x64.exe",                                                               // Voicemeeter Potato x64
        {(void *)0x156858, {0x0, 0x0, 0x0, 0x0}, 4},                                         // 0 seconds
        {(void *)0x13D2E, {0x90, 0xC7, 0x84, 0x24, 0x68, 0x0A, 0x00, 0x00, 0x00, 0x00}, 10}, // automatically sets variable to 0
        {(void *)0x0, {0x0}, 0},                                                             // not implemented
        {(void *)0x12AD3, {0x41, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x90}, 7},                    // remplazar la funcion por nop's
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

void nonBlocking_Messagebox(const char* message, const char* windowName){
    HWND hwnd = CreateWindowEx(0, "STATIC", windowName, 
                          WS_VISIBLE | WS_POPUP,
                          100, 100, 400, 300, NULL, NULL, NULL, NULL);
    SetWindowText(hwnd, message);
}

/**
 * returns true if there was an error
 */
BOOL writeStruct_ChangeAdressTo(const ChangeAdressTo *currentJob, SIZE_T *bytesWritten)
{
    if (currentJob->newValueSize == 0)
    {
        MessageBox(NULL, "ChangeAdressTo struct not finished", "Bypass writeStruct_ChangeAdressTo", MB_OK);
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
        MessageBox(NULL, errorMsg, "Bypass GetModuleHandle", MB_OK);
        return TRUE;
    }
    
    // Calcular dirección absoluta sumando el offset a la dirección base del módulo
    absoluteAddress = (LPVOID)((BYTE*)hModule + (DWORD_PTR)currentJob->relativeAddress);
    
    // Cambiar permisos de memoria a PAGE_EXECUTE_READWRITE
    if (!VirtualProtect(absoluteAddress, currentJob->newValueSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        DWORD errorCode = GetLastError();
        sprintf(errorMsg, "Error al cambiar protección de memoria. Código: %lu, Dirección: %p, Tamaño: %zu", 
                errorCode, absoluteAddress, currentJob->newValueSize);
        MessageBox(NULL, errorMsg, "Bypass VirtualProtect", MB_OK);
        return TRUE; // Hubo error
    }
    
    // Modificar la dirección de memoria
    BOOL writeResult = WriteProcessMemory(GetCurrentProcess(),
                             absoluteAddress,
                             currentJob->newValue,
                             currentJob->newValueSize,
                             bytesWritten);
    
    if (!writeResult)
    {
        DWORD errorCode = GetLastError();
        sprintf(errorMsg, "Error al escribir memoria. Código: %lu, Bytes escritos: %zu/%zu", 
                errorCode, *bytesWritten, currentJob->newValueSize);
        MessageBox(NULL, errorMsg, "Bypass WriteProcessMemory", MB_OK);
    }
    
    // Restaurar la protección original
    DWORD dummy;
    VirtualProtect(absoluteAddress, currentJob->newValueSize, oldProtect, &dummy);
    
    // Verificar si la escritura fue exitosa
    return !writeResult || (*bytesWritten < currentJob->newValueSize);
}

void write_timeLeftVariable()
{
    SIZE_T bytesWritten;
    if (writeStruct_ChangeAdressTo(&getCurrentVariant()->timeLeftVariable, &bytesWritten))
    {
        MessageBox(NULL, "Error al modificar la memoria!", "Bypass write_timeLeftVariable", MB_OK);
    }
}

void write_timeFunction()
{
    SIZE_T bytesWritten;
    if (writeStruct_ChangeAdressTo(&getCurrentVariant()->timeFunction, &bytesWritten))
    {
        MessageBox(NULL, "Error al modificar la memoria!", "Bypass write_timeFunction", MB_OK);
    }
}

void write_windowFunction()
{
    SIZE_T bytesWritten;
    if (writeStruct_ChangeAdressTo(&getCurrentVariant()->windowFunction, &bytesWritten))
    {
        MessageBox(NULL, "Error al modificar la memoria!", "Bypass write_windowFunction", MB_OK);
    }
}

void write_windowVariable()
{
    SIZE_T bytesWritten;
    if (writeStruct_ChangeAdressTo(&getCurrentVariant()->windowVariable, &bytesWritten))
    {
        MessageBox(NULL, "Error al modificar la memoria!", "Bypass write_windowVariable", MB_OK);
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
LPCWSTR registrationWindowName = L"About / Registration info...";
const char *registrationWindowName2 = "About / Registration info...";

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
    if (lpWindowName && wcscmp(lpWindowName, registrationWindowName) == 0)
    {
        nonBlocking_Messagebox("Registration window opened", "Bypass DetourCreateWindowExW");
        executeSimpleAfterDelay(write_timeFunction, 3000);
        executeSimpleAfterDelay(write_timeLeftVariable, 3000);
        // Retornar NULL para evitar que la ventana se cree
        return NULL;
    }

    return fpCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
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
        return 1;
    }

    // Obtener la dirección real de la función CreateWindowExW
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    pCreateWindowExW = (void *)GetProcAddress(hUser32, "CreateWindowExW");

    // Crea un hook para CreateWindowExW
    if (MH_CreateHook(pCreateWindowExW,
                      (LPVOID)&DetourCreateWindowExW,
                      (LPVOID *)&fpCreateWindowExW) != MH_OK)
    {
        return 1;
    }

    voicemeeterVersionIndex = getVersionIndex(); // obtiene el indice antes de activar el hook en caso de llegar a necesitarlo apenas se active

    if (voicemeeterVersionIndex < 0)
    {
        MessageBox(NULL, "Voicemeeter version not found", "Bypass main", MB_OK);
        MH_Uninitialize();
        return 1;
    }
    

    // Habilita el hook
    if (MH_EnableHook(pCreateWindowExW) != MH_OK)
    {
        return 1;
    }

    closeWindow(registrationWindowName2);

    write_timeFunction();
    write_timeLeftVariable();
    write_windowFunction();
    write_windowVariable();

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        main();
        break;
    case DLL_PROCESS_DETACH:
        // Liberar recursos
        MH_DisableHook(pCreateWindowExW);
        MH_Uninitialize();
        break;
    }
    return TRUE;
}