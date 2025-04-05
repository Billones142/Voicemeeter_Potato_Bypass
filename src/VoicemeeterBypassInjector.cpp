#include <Windows.h>
#include <iostream>
#include <fstream>
#include <tlhelp32.h>

#define RESOURCE_TYPE RT_RCDATA

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

// Function to get the PID of a process by its name
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
            // Convert pe.szExeFile (ANSI) to wstring
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

bool ExtractDLLFromResource(const wchar_t *resourceName)
{
    // Get the handle to the resource
    HRSRC hRes = FindResourceW(NULL, resourceName, (LPWSTR)RESOURCE_TYPE);
    if (!hRes)
    {
        std::cout << "Error finding resource: " << GetLastError() << std::endl;
        return false;
    }

    // Load the resource into memory
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData)
    {
        std::cout << "Error loading resource: " << GetLastError() << std::endl;
        return false;
    }

    // Get pointer to the data and size
    DWORD size = SizeofResource(NULL, hRes);
    void *pData = LockResource(hData);

    // Get the path to the system's temporary directory
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath) == 0)
    {
        std::cout << "Error getting temporary folder: " << GetLastError() << std::endl;
        return false;
    }

    // Generate a random name for the temporary file
    wchar_t randomName[16];
    srand((unsigned int)time(NULL)); // Initialize the seed for rand()
    for (int i = 0; i < 8; i++)
    {
        // Generate random alphanumeric characters
        int randCharacter = rand() % 36;
        if (randCharacter < 10)
            randomName[i] = L'0' + randCharacter; // Digits 0-9
        else
            randomName[i] = L'a' + (randCharacter - 10); // Letters a-z
    }
    randomName[8] = L'\0'; // Null-terminate the string

    // Create the full path
    wcscpy(tempFileName, tempPath);
    wcscat(tempFileName, randomName);
    wcscat(tempFileName, L".dll");

    // Delete the file if it already exists
    DeleteFileW(tempFileName);

    // Convert the path from wstring to string for ofstream
    std::string narrowPath;
    int requiredSize = WideCharToMultiByte(CP_ACP, 0, tempFileName, -1, NULL, 0, NULL, NULL);
    ;
    if (requiredSize > 0)
    {
        narrowPath.resize(requiredSize);
        WideCharToMultiByte(CP_ACP, 0, tempFileName, -1, &narrowPath[0], requiredSize, NULL, NULL);
        narrowPath.resize(requiredSize - 1); // Remove the null-terminator
    }
    else
    {
        std::cout << "Error converting path to ANSI format: " << GetLastError() << std::endl;
        return false;
    }

    // Write the data to a temporary file
    std::ofstream outFile(narrowPath.c_str(), std::ios::out | std::ios::binary);
    if (!outFile)
    {
        std::cout << "Error creating temporary file: " << narrowPath << std::endl;
        return false;
    }

    outFile.write(static_cast<const char *>(pData), size);
    outFile.close();

    // Verify that the file was created correctly
    if (GetFileAttributesW(tempFileName) == INVALID_FILE_ATTRIBUTES)
    {
        std::cout << "Error: The DLL file was not created correctly" << std::endl;
        std::wcout << L"File path: " << tempFileName << std::endl;
        return false;
    }

    std::wcout << L"DLL written to: " << tempFileName << std::endl;

    return true;
}

bool InjectDLL(DWORD processId)
{
    // Get handle to the process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
    if (!hProcess)
    {
        std::cout << "Error opening process: " << GetLastError() << std::endl;
        return false;
    }

    // Verify that the DLL file exists
    if (GetFileAttributesW(tempFileName) == INVALID_FILE_ATTRIBUTES)
    {
        std::cout << "Error: The DLL file does not exist at the path: " << std::endl;
        std::wcout << tempFileName << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Use the Unicode path directly to avoid encoding issues
    // Allocate memory in the target process for the DLL path (in UNICODE format)
    SIZE_T dlPathSize = (wcslen(tempFileName) + 1) * sizeof(wchar_t);
    void *pRemotePath = VirtualAllocEx(hProcess, NULL, dlPathSize,
                                       MEM_COMMIT, PAGE_READWRITE);
    if (!pRemotePath)
    {
        std::cout << "Error in VirtualAllocEx: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Write the UNICODE path of the DLL in the target process
    if (!WriteProcessMemory(hProcess, pRemotePath, tempFileName, dlPathSize, NULL))
    {
        std::cout << "Error in WriteProcessMemory: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Get the address of LoadLibraryW (UNICODE version of LoadLibrary)
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32)
    {
        std::cout << "Error getting handle of kernel32.dll: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibrary)
    {
        std::cout << "Error getting address of LoadLibraryW: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Create a remote thread that loads the DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
                                        (LPTHREAD_START_ROUTINE)pLoadLibrary,
                                        pRemotePath, 0, NULL);
    if (!hThread)
    {
        DWORD errorCode = GetLastError();
        std::cout << "Error in CreateRemoteThread: " << errorCode << std::endl;

        // Additional information about common errors
        if (errorCode == ERROR_ACCESS_DENIED)
            std::cout << "Access denied. You may need administrator permissions." << std::endl;
        else if (errorCode == ERROR_NOT_ENOUGH_MEMORY)
            std::cout << "Not enough memory in the target process." << std::endl;

        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Wait for the thread to finish with timeout (5 seconds)
    DWORD waitResult = WaitForSingleObject(hThread, 5000); // 5 seconds timeout

    if (waitResult == WAIT_TIMEOUT)
    {
        // We can continue anyway, as the DLL may have loaded correctly
    }
    else if (waitResult != WAIT_OBJECT_0)
    {
        std::cout << "Error waiting for the thread: " << GetLastError() << std::endl;
    }

    // Get the exit code (DLL handle or 0 in case of error)
    DWORD exitCode = 0;
    if (!GetExitCodeThread(hThread, &exitCode))
    {
        std::cout << "Error getting exit code of the thread: " << GetLastError() << std::endl;
    }
    else if (exitCode == 0)
    {
        std::cout << "LoadLibraryW failed in the remote process. Code: " << exitCode << std::endl;
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Cleanup
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}

/**
 * Returns the Voicemeeter process ID and extracts the DLL to inject depending on its architecture
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
    DWORD processId;

    unsigned int sleepTime = 5000; // 5 seconds
    unsigned char attempts = 24;   // 2 minutes

    while (attempts > 0)
    {
        std::cout << "Waiting for Voicemeeter to start... attempts left: " << (int)attempts << std::endl;
        processId = getVoicemeeterProcessId();

        if (processId != 0)
        {
            break;
        }

        Sleep(sleepTime); // Wait 5 seconds before trying again
        attempts--;
    }

    if (processId == 0)
    {
        std::cout << "Voicemeeter process not found" << std::endl;
        std::cout << "Make sure Voicemeeter is running" << std::endl;
        return 1;
    }

    std::cout << "Voicemeeter process found. PID: " << processId << std::endl;

    Sleep(1000); // Wait for it to start completely

    if (!dllExtractionResult)
    {
        std::cout << "Error extracting the DLL from the resource" << std::endl;
        std::cout << "Check that the resources MYDLLX86/MYDLLX64 are included correctly in the executable" << std::endl;
        return 1;
    }

    // Inject the DLL
    if (InjectDLL(processId))
    {
        std::cout << "DLL injected successfully!" << std::endl;
    }
    else
    {
        std::cout << "Error injecting the DLL" << std::endl;
        return 1;
    }

    // Delete the temporary file
    DeleteFileW(tempFileName);
    std::cout << "Temporary file deleted" << std::endl;

    return 0;
}