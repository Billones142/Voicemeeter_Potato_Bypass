#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <tlhelp32.h> // Incluir para enumerar procesos

#define RESOURCE_TYPE RT_RCDATA
//#define TEMP_DLL_NAME "temp_dll.dll"

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

    // Generar un nombre de archivo único en la carpeta temporal
    if (GetTempFileNameW(tempPath, L"temp_dll.dll", 0, tempFileName) == 0)
    {
        std::cout << "Error al crear nombre de archivo temporal: " << GetLastError() << std::endl;
        return false;
    }

    // Eliminar el archivo creado por GetTempFileName (lo vamos a recrear)
    DeleteFileW(tempFileName);

    // Añadir la extensión .dll
    std::wstring dllPath = tempFileName;
    if (dllPath.find(L".dll") == std::wstring::npos)
    {
        dllPath += L".dll";
    }

    // Para C++17 y posteriores, puedes usar:
    // std::filesystem::path path(dllPath);
    // std::ofstream outFile(path, std::ios::out | std::ios::binary);

    // Para versiones anteriores, convierte la ruta wstring a string:
    std::string narrowPath;
    int requiredSize = WideCharToMultiByte(CP_ACP, 0, dllPath.c_str(), -1, NULL, 0, NULL, NULL);
    if (requiredSize > 0)
    {
        narrowPath.resize(requiredSize);
        WideCharToMultiByte(CP_ACP, 0, dllPath.c_str(), -1, &narrowPath[0], requiredSize, NULL, NULL);
        // Eliminar el carácter nulo que WideCharToMultiByte añade al final
        narrowPath.resize(requiredSize - 1);
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
        std::cout << "Error al crear archivo temporal" << std::endl;
        return false;
    }

    outFile.write(static_cast<const char *>(pData), size);
    outFile.close();

    // Guardar la ruta en alguna variable global o estructura para uso posterior
    // Por ejemplo:
    // g_tempDllPath = dllPath;

    return true;
}

bool InjectDLL(DWORD processId)
{
    // Obtener handle al proceso
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess)
    {
        std::cout << "Error al abrir el proceso: " << GetLastError() << std::endl;
        return false;
    }

    // Obtener la ruta completa del DLL temporal
    wchar_t wideDllPath[MAX_PATH];
    GetFullPathNameW(tempFileName, MAX_PATH, wideDllPath, NULL);

    // Convertir la ruta de wchar_t a char para usarla con VirtualAllocEx
    char dllPath[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, wideDllPath, -1, dllPath, MAX_PATH, NULL, NULL);

    // Asignar memoria en el proceso objetivo para la ruta del DLL
    void *pRemotePath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1,
                                       MEM_COMMIT, PAGE_READWRITE);
    if (!pRemotePath)
    {
        std::cout << "Error en VirtualAllocEx: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Escribir la ruta del DLL en el proceso objetivo
    if (!WriteProcessMemory(hProcess, pRemotePath, dllPath, strlen(dllPath) + 1, NULL))
    {
        std::cout << "Error en WriteProcessMemory: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Obtener dirección de LoadLibraryA
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");

    // Crear un hilo remoto que cargue el DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
                                        (LPTHREAD_START_ROUTINE)pLoadLibrary,
                                        pRemotePath, 0, NULL);
    if (!hThread)
    {
        std::cout << "Error en CreateRemoteThread: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Esperar a que termine el hilo
    WaitForSingleObject(hThread, INFINITE);

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

    for (size_t i = 0; i < sizeof(voicemeeterVariantsx86); i++)
    {
        DWORD processId = GetProcessIDByName(voicemeeterVariantsx86[i]);
        if (processId != 0)
        {
            dllExtractionResult = ExtractDLLFromResource(L"MYDLLX86");
            return processId;
        }
    }
#if _WIN64
    for (size_t i = 0; i < sizeof(voicemeeterVariantsx64); i++)
    {
        DWORD processId = GetProcessIDByName(voicemeeterVariantsx64[i]);
        if (processId != 0)
        {
            dllExtractionResult = ExtractDLLFromResource(L"MYDLLX64");
            return processId;
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
        std::cout << "No se encontro el proceso de voicemeeter" << std::endl;
        return 1;
    }

    // Extrae el DLL del recurso
    if (!dllExtractionResult)
    {
        std::cout << "Error al extraer el DLL del recurso" << std::endl;
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
    }

    // Opcional: Eliminar el archivo temporal
    char ansiFileName[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, tempFileName, -1, ansiFileName, MAX_PATH, NULL, NULL);
    DeleteFileA(ansiFileName);

    return 0;
}