#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <tlhelp32.h> // Incluir para enumerar procesos

#define RESOURCE_TYPE RT_RCDATA
// #define TEMP_DLL_NAME "temp_dll.dll"

bool dllExtractionResult;
wchar_t tempFileName[MAX_PATH];

#if _WIN64
const std::wstring voicemeeterVariantsx64[] = {
    L"voicemeeter8x64.exe",
};
#endif

const std::wstring voicemeeterVariantsx86[] = {
    L"voicemeeter8.exe",
};

// Función para obtener el PID de un proceso por su nombre
DWORD GetProcessIDByName(const std::wstring &processName)
{
    DWORD processID = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe))
    {
        do
        {
            // Convertir pe.szExeFile (ANSI) a wstring
            std::wstring exeFileName;
            int size_needed = MultiByteToWideChar(CP_ACP, 0, (LPCCH)pe.szExeFile, -1, NULL, 0);
            if (size_needed > 0)
            {
                wchar_t *wideExeFile = new wchar_t[size_needed];
                MultiByteToWideChar(CP_ACP, 0, (LPCCH)pe.szExeFile, -1, wideExeFile, size_needed);
                exeFileName = wideExeFile;
                delete[] wideExeFile;
            }

            if (processName == exeFileName)
            {
                processID = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return processID;
}

/*bool ExtractDLLFromResource(const wchar_t *resourceName)
{
    // Obtener el handle al recurso
    HRSRC hRes = FindResourceW(NULL, resourceName, (LPWSTR)RESOURCE_TYPE);
    if (!hRes)
    {
        std::cout << "Error al encontrar el recurso: " << GetLastError() << std::endl;
        return false;
    }

    // Cargar el recurso en memoria
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData)
    {
        std::cout << "Error al cargar el recurso: " << GetLastError() << std::endl;
        return false;
    }

    // Obtener puntero a los datos y tamaño
    DWORD size = SizeofResource(NULL, hRes);
    void *pData = LockResource(hData);

    // Escribir los datos a un archivo temporal
    std::ofstream outFile(TEMP_DLL_NAME, std::ios::out | std::ios::binary);
    if (!outFile)
    {
        std::cout << "Error al crear archivo temporal" << std::endl;
        return false;
    }

    outFile.write(static_cast<const char *>(pData), size);
    outFile.close();

    return true;
}*/

bool ExtractDLLFromResource(const wchar_t *resourceName)
{
    // Obtener el handle al recurso
    HRSRC hRes = FindResourceW(NULL, resourceName, (LPWSTR)RESOURCE_TYPE);
    if (!hRes)
    {
        std::cout << "Error al encontrar el recurso: " << GetLastError() << std::endl;
        return false;
    }

    // Cargar el recurso en memoria
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData)
    {
        std::cout << "Error al cargar el recurso: " << GetLastError() << std::endl;
        return false;
    }

    // Obtener puntero a los datos y tamaño
    DWORD size = SizeofResource(NULL, hRes);
    void *pData = LockResource(hData);

    // Obtener la ruta al directorio temporal del sistema
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath) == 0)
    {
        std::cout << "Error al obtener la carpeta temporal: " << GetLastError() << std::endl;
        return false;
    }

    // Generar un nombre aleatorio para el archivo temporal
    wchar_t randomName[16];
    srand((unsigned int)time(NULL)); // Inicializar la semilla para rand()
    for (int i = 0; i < 8; i++)
    {
        // Generar caracteres alfanuméricos aleatorios
        int randCharacter = rand() % 36;
        if (randCharacter < 10)
            randomName[i] = L'0' + randCharacter; // Dígitos 0-9
        else
            randomName[i] = L'a' + (randCharacter - 10); // Letras a-z
    }
    randomName[8] = L'\0'; // Null-terminar el string

    // Crear la ruta completa
    wcscpy(tempFileName, tempPath);
    wcscat(tempFileName, randomName);
    wcscat(tempFileName, L".dll");

    // Eliminar archivo si ya existe
    DeleteFileW(tempFileName);

    // Convertir la ruta wstring a string para ofstream
    std::string narrowPath;
    int requiredSize = WideCharToMultiByte(CP_ACP, 0, tempFileName, -1, NULL, 0, NULL, NULL);
    if (requiredSize > 0)
    {
        narrowPath.resize(requiredSize);
        WideCharToMultiByte(CP_ACP, 0, tempFileName, -1, &narrowPath[0], requiredSize, NULL, NULL);
        narrowPath.resize(requiredSize - 1); // Remover el null-terminator
    }
    else
    {
        std::cout << "Error al convertir la ruta a formato ANSI: " << GetLastError() << std::endl;
        return false;
    }

    // Escribir los datos a un archivo temporal
    std::ofstream outFile(narrowPath.c_str(), std::ios::out | std::ios::binary);
    if (!outFile)
    {
        std::cout << "Error al crear archivo temporal: " << narrowPath << std::endl;
        return false;
    }

    outFile.write(static_cast<const char *>(pData), size);
    outFile.close();

    // Verificar que el archivo se creó correctamente
    if (GetFileAttributesW(tempFileName) == INVALID_FILE_ATTRIBUTES)
    {
        std::cout << "Error: El archivo DLL no se creó correctamente" << std::endl;
        std::wcout << L"Ruta del archivo: " << tempFileName << std::endl;
        return false;
    }

    std::wcout << L"DLL escrito en: " << tempFileName << std::endl;

    return true;
}

bool InjectDLL(DWORD processId)
{
    // Obtener handle al proceso
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
    if (!hProcess)
    {
        std::cout << "Error al abrir el proceso: " << GetLastError() << std::endl;
        return false;
    }

    // Verificar que el archivo DLL existe
    if (GetFileAttributesW(tempFileName) == INVALID_FILE_ATTRIBUTES)
    {
        std::cout << "Error: El archivo DLL no existe en la ruta: " << std::endl;
        std::wcout << tempFileName << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Usaremos la ruta Unicode directamente para evitar problemas de codificación
    // Asignar memoria en el proceso objetivo para la ruta del DLL (en formato UNICODE)
    SIZE_T dlPathSize = (wcslen(tempFileName) + 1) * sizeof(wchar_t);
    void *pRemotePath = VirtualAllocEx(hProcess, NULL, dlPathSize,
                                       MEM_COMMIT, PAGE_READWRITE);
    if (!pRemotePath)
    {
        std::cout << "Error en VirtualAllocEx: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Escribir la ruta UNICODE del DLL en el proceso objetivo
    if (!WriteProcessMemory(hProcess, pRemotePath, tempFileName, dlPathSize, NULL))
    {
        std::cout << "Error en WriteProcessMemory: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Obtener dirección de LoadLibraryW (versión UNICODE de LoadLibrary)
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32)
    {
        std::cout << "Error al obtener handle de kernel32.dll: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibrary)
    {
        std::cout << "Error al obtener dirección de LoadLibraryW: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Crear un hilo remoto que cargue el DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
                                        (LPTHREAD_START_ROUTINE)pLoadLibrary,
                                        pRemotePath, 0, NULL);
    if (!hThread)
    {
        DWORD errorCode = GetLastError();
        std::cout << "Error en CreateRemoteThread: " << errorCode << std::endl;

        // Información adicional sobre errores comunes
        if (errorCode == ERROR_ACCESS_DENIED)
            std::cout << "Acceso denegado. Posiblemente necesites permisos de administrador." << std::endl;
        else if (errorCode == ERROR_NOT_ENOUGH_MEMORY)
            std::cout << "No hay suficiente memoria en el proceso objetivo." << std::endl;

        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Esperar a que termine el hilo con timeout (5 segundos)
    DWORD waitResult = WaitForSingleObject(hThread, 5000); // 5 segundos de timeout

    if (waitResult == WAIT_TIMEOUT)
    {
        // Podemos continuar de todas formas, ya que el DLL podría haberse cargado correctamente
    }
    else if (waitResult != WAIT_OBJECT_0)
    {
        std::cout << "Error al esperar por el hilo: " << GetLastError() << std::endl;
    }

    // Obtener el código de salida (handle del DLL o 0 en caso de error)
    DWORD exitCode = 0;
    if (!GetExitCodeThread(hThread, &exitCode))
    {
        std::cout << "Error al obtener código de salida del hilo: " << GetLastError() << std::endl;
    }
    else if (exitCode == 0)
    {
        std::cout << "LoadLibraryW falló en el proceso remoto. Código: " << exitCode << std::endl;
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Limpiar
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}

/**
 * returns the voicemeeter process id and extracts the dll to inject depending on its arquitechture
 */
DWORD getVoicemeeterProcessId()
{
    DWORD processId;
#if _WIN64
    bool found = false;
#endif

    for (size_t i = 0; i < sizeof(voicemeeterVariantsx86); i++)
    {
        processId = GetProcessIDByName(voicemeeterVariantsx86[i]);
        if (processId != 0)
        {
            dllExtractionResult = ExtractDLLFromResource(L"MYDLLX86");
#if _WIN64
            found = true;
#endif
            break;
        }
    }
#if _WIN64
    if (found)
    {
        return processId;
    }

    for (size_t i = 0; i < sizeof(voicemeeterVariantsx64); i++)
    {
        processId = GetProcessIDByName(voicemeeterVariantsx64[i]);
        if (processId != 0)
        {
            dllExtractionResult = ExtractDLLFromResource(L"MYDLLX64");
            found = true;
            break;
        }
    }
#endif
    return processId;
}

int main()
{
    DWORD processId = getVoicemeeterProcessId();

    if (processId == 0)
    {
        std::cout << "No se encontro el proceso de Voicemeeter" << std::endl;
        std::cout << "Asegurate de que Voicemeeter este en ejecucion" << std::endl;
        return 1;
    }

    std::cout << "Proceso de Voicemeeter encontrado. PID: " << processId << std::endl;

    // Extrae el DLL del recurso
    if (!dllExtractionResult)
    {
        std::cout << "Error al extraer el DLL del recurso" << std::endl;
        std::cout << "Verifica que los recursos MYDLLX86/MYDLLX64 estén incluidos correctamente en el ejecutable" << std::endl;
        return 1;
    }

    // Inyecta el DLL
    if (InjectDLL(processId))
    {
        std::cout << "DLL inyectado correctamente!" << std::endl;
    }
    else
    {
        std::cout << "Error al inyectar el DLL" << std::endl;
        return 1;
    }

    // Eliminar el archivo temporal
    DeleteFileW(tempFileName);
    std::cout << "Archivo temporal eliminado" << std::endl;

    return 0;
}