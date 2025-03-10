#include <Windows.h>

// Estructura para pasar datos al hilo
typedef struct {
    void (*callback)(void);  // Puntero a la función sin parámetros
    DWORD delayMs;           // Retraso en milisegundos
} SimpleDelayedCallInfo;

// Función que ejecuta el hilo
DWORD WINAPI SimpleDelayedThreadProc(LPVOID lpParameter) {
    SimpleDelayedCallInfo* info = (SimpleDelayedCallInfo*)lpParameter;
    
    // Esperar el tiempo especificado
    Sleep(info->delayMs);
    
    // Ejecutar la función de callback
    info->callback();
    
    // Liberar la memoria asignada para la estructura
    free(info);
    
    return 0;
}

// Función principal que inicia la ejecución retardada para funciones void sin parámetros
BOOL executeSimpleAfterDelay(void (*callback)(void), DWORD delayMs) {
    // Asignar memoria para la estructura
    SimpleDelayedCallInfo* info = (SimpleDelayedCallInfo*)malloc(sizeof(SimpleDelayedCallInfo));
    if (!info) {
        return FALSE;
    }
    
    // Inicializar la estructura
    info->callback = callback;
    info->delayMs = delayMs;
    
    // Crear un hilo que se ejecutará después del retraso
    HANDLE hThread = CreateThread(
        NULL,                       // Atributos de seguridad predeterminados
        0,                          // Tamaño de pila predeterminado
        SimpleDelayedThreadProc,    // Función del hilo
        info,                       // Parámetro para la función
        0,                          // Crear el hilo inmediatamente
        NULL                        // No necesitamos el ID del hilo
    );
    
    if (hThread == NULL) {
        free(info);
        return FALSE;
    }
    
    // Cerrar el handle del hilo porque no lo necesitamos
    CloseHandle(hThread);
    
    return TRUE;
}

// Ejemplo de uso
void miFuncionSimple(void) {
    // Tu código aquí
    OutputDebugString("Esta función se ejecutó después del retraso");
}

// Así es como la usarías:
// executeSimpleAfterDelay(miFuncionSimple, 3000);  // Ejecuta después de 3 segundos