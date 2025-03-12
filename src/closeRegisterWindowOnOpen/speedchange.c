#include <windows.h>
#include <MinHook.h>

// Definimos los tipos de punteros a función para el hooking
typedef DWORD(WINAPI *TIMEGETTIME)(void);
typedef BOOL(WINAPI *QUERYPERFORMANCECOUNTER)(LARGE_INTEGER *);
typedef BOOL(WINAPI *QUERYPERFORMANCEFREQUENCY)(LARGE_INTEGER *);

// Variables para almacenar los punteros a las funciones originales
static TIMEGETTIME fpTimeGetTime = NULL;
static QUERYPERFORMANCECOUNTER fpQueryPerformanceCounter = NULL;
static QUERYPERFORMANCEFREQUENCY fpQueryPerformanceFrequency = NULL;

// Variable global para el factor de velocidad (1.0 = velocidad normal)
static float g_speedFactor = 1.0f;
static BOOL g_isInitialized = FALSE;

// Funciones de hook

// Hook para timeGetTime
static DWORD WINAPI DetourTimeGetTime(void)
{
    // Llamamos a la función original
    DWORD originalTime = fpTimeGetTime();

    // Modificamos el valor de retorno según nuestro factor de velocidad
    return (DWORD)(originalTime * g_speedFactor);
}

// Hook para QueryPerformanceCounter
static BOOL WINAPI DetourQueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    // Llamamos a la función original
    BOOL result = fpQueryPerformanceCounter(lpPerformanceCount);

    // Modificamos el valor según nuestro factor de velocidad
    lpPerformanceCount->QuadPart = (LONGLONG)(lpPerformanceCount->QuadPart * g_speedFactor);

    return result;
}

// Hook para QueryPerformanceFrequency
static BOOL WINAPI DetourQueryPerformanceFrequency(LARGE_INTEGER *lpFrequency)
{
    // Llamamos a la función original
    BOOL result = fpQueryPerformanceFrequency(lpFrequency);

    // Modificamos la frecuencia para afectar a la velocidad
    lpFrequency->QuadPart = (LONGLONG)(lpFrequency->QuadPart / g_speedFactor);

    return result;
}

/**
 * Función para inicializar los hooks, llamar cuando minhook este inicializado
 */
BOOL initializeSpeedChange()
{
    // Si ya está inicializado, no hacemos nada
    if (g_isInitialized)
    {
        return TRUE;
    }
    // Creamos hooks para las funciones de tiempo
    BOOL atLeastOneHookCreated = FALSE;

    // timeGetTime (winmm.dll)
    if (MH_CreateHookApi(L"winmm.dll", "timeGetTime", &DetourTimeGetTime, (LPVOID *)&fpTimeGetTime) == MH_OK)
    {
        atLeastOneHookCreated = TRUE;
    }

    // QueryPerformanceCounter (kernel32.dll)
    if (MH_CreateHookApi(L"kernel32.dll", "QueryPerformanceCounter", &DetourQueryPerformanceCounter, (LPVOID *)&fpQueryPerformanceCounter) == MH_OK)
    {
        atLeastOneHookCreated = TRUE;
    }

    // QueryPerformanceFrequency (kernel32.dll)
    if (MH_CreateHookApi(L"kernel32.dll", "QueryPerformanceFrequency", &DetourQueryPerformanceFrequency, (LPVOID *)&fpQueryPerformanceFrequency) == MH_OK)
    {
        atLeastOneHookCreated = TRUE;
    }

    // Habilitamos todos los hooks
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        return FALSE;
    }

    // Establecemos el flag de inicialización
    g_isInitialized = TRUE;
    g_speedFactor = 1.0f; // Velocidad inicial normal

    return TRUE;
}

// Función para cambiar la velocidad
BOOL changeSpeed(float speedFactor)
{
    // Comprobamos que esté inicializado
    if (!g_isInitialized)
    {
        return FALSE;
    }

    // Comprobamos que el factor de velocidad sea válido
    if (speedFactor <= 0.0f)
    {
        return FALSE;
    }

    // Establecemos el nuevo factor de velocidad
    g_speedFactor = speedFactor;

    return TRUE;
}

// Función para cerrar/limpiar los hooks
BOOL uninitializeSpeedChange()
{
    // Si no está inicializado, no hacemos nada
    if (!g_isInitialized)
    {
        return TRUE;
    }

    // Deshabilitamos y limpiamos todos los hooks
    MH_DisableHook(MH_ALL_HOOKS);

    // Reseteamos las variables
    fpTimeGetTime = NULL;
    fpQueryPerformanceCounter = NULL;
    fpQueryPerformanceFrequency = NULL;
    g_speedFactor = 1.0f;
    g_isInitialized = FALSE;

    return TRUE;
}