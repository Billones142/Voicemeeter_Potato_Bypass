#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

#define MODIFYTIMELEFTVARIABLE
#define MODIFYFUNCTIONCODE

#define CLOSEREGISTRATIONWINDOW
#define CLOSEMAINWINDOW // recomended with system tray enabled
#define CLOSEDRIVERERRORWINDOW

#define CONSOLELOGS TRUE
#define FILELOGS TRUE
#define FILELOGNAME "VoicemeeterBypass.log"

// Function to log messages to console and/or file
void makelog(const char *message)
{
    if (CONSOLELOGS)
    {
        printf("%s", message);
    }
    if (FILELOGS)
    {
        FILE *logFile = fopen(FILELOGNAME, "a");
        if (logFile != NULL)
        {
            fprintf(logFile, "%s", message);
            fclose(logFile);
        }
        else
        {
            printf("Error opening log file.\n");
        }
    }
}

// Function to get the process ID by name
DWORD GetProcessIdByName(const char *processName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        makelog("Failed to create snapshot.\n");
        return 0;
    }

    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (_stricmp(entry.szExeFile, processName) == 0)
            {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

// Function to get the base address of a module
DWORD64 GetModuleBaseAddress(DWORD processId, const char *moduleName)
{
    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        makelog("Failed to create module snapshot.\n");
        return 0;
    }

    if (Module32First(snapshot, &moduleEntry))
    {
        do
        {
            if (_stricmp(moduleEntry.szModule, moduleName) == 0)
            {
                CloseHandle(snapshot);
                return (DWORD64)moduleEntry.modBaseAddr;
            }
        } while (Module32Next(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return 0;
}

// Function to check if memory is accessible
BOOL IsMemoryAccessible(HANDLE hProcess, LPCVOID address)
{
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)))
    {
        return mbi.State == MEM_COMMIT;
    }
    return FALSE;
}

// Function to wait for a process to start running
BOOL WaitForProcess(const char *processName, DWORD *processId)
{
    UINT8 attempts = 24; // 2 minutes
    while (attempts > 0)
    {
        *processId = GetProcessIdByName(processName);
        if (*processId != 0)
        {
            return TRUE;
        }
        char logMsg[128];
        sprintf(logMsg, "Waiting for Voicemeeter to start... attempts left: %i\n", attempts);
        makelog(logMsg);
        Sleep(5000); // Wait 5 seconds before trying again
        attempts--;
    }
    return FALSE;
}

// Function to close a specific Voicemeeter window
BOOL closeWindow(const char *windowTitle)
{
    HWND hWnd = FindWindow(NULL, windowTitle);

    if (hWnd == NULL)
    {
        makelog("Window not found.\n");
        return FALSE;
    }

    SendMessage(hWnd, WM_CLOSE, 0, 0);
    makelog("Window closed successfully.\n");
    return TRUE;
}

// Struct representing possible addresses for the variable and its modifying function
typedef struct VoicemeeterAddress
{
    DWORD64 relativeVariableAddress;
    DWORD64 relativeFunctionAddress;
} VoicemeeterInit;

int main(int argc, char **argv)
{
    const char *processName = "voicemeeter8x64.exe";
    DWORD processId;
    VoicemeeterInit initVoicemeeter[] = {
        {0x156858, 0x10901},
        {0x13B518, 0x13D2E}};

    UINT8 initChoice = 0;
    DWORD64 voicemeeterBaseAddress;

    if (!WaitForProcess(processName, &processId))
    {
        makelog("Failed to find the process.\n");
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL)
    {
        makelog("Failed to open the process.\n");
        return 1;
    }

    voicemeeterBaseAddress = GetModuleBaseAddress(processId, processName);
    if (voicemeeterBaseAddress == 0)
    {
        makelog("Failed to find the module base address.\n");
        CloseHandle(hProcess);
        return 1;
    }

    for (UINT8 i = 0; i < sizeof(initVoicemeeter) / sizeof(initVoicemeeter[0]); i++)
    {
        if (IsMemoryAccessible(hProcess, (LPCVOID)(voicemeeterBaseAddress + initVoicemeeter[i].relativeVariableAddress)))
        {
            initChoice = i;
            break;
        }
    }

    #ifdef MODIFYTIMELEFTVARIABLE
    const DWORD64 absoluteVariableAddress = voicemeeterBaseAddress + initVoicemeeter[initChoice].relativeVariableAddress;
    const BYTE valorCero[4] = {0x0, 0x0, 0x0, 0x0};

    if (WriteProcessMemory(hProcess, (LPVOID)absoluteVariableAddress, valorCero, sizeof(valorCero), NULL))
    {
        makelog("Value successfully modified.\n");
    }
    else
    {
        makelog("Failed to modify the value.\n");
    }
    #endif

    #ifdef MODIFYFUNCTIONCODE
    const DWORD64 absoluteFunctionAddress = voicemeeterBaseAddress + initVoicemeeter[initChoice].relativeFunctionAddress;
    const BYTE newInstruction[10] = {0x41, 0xC7, 0x84, 0x24, 0x68, 0x0A, 0x00, 0x00, 0x00, 0x00};

    if (WriteProcessMemory(hProcess, (LPVOID)absoluteFunctionAddress, newInstruction, sizeof(newInstruction), NULL))
    {
        makelog("Instruction successfully modified.\n");
    }
    else
    {
        makelog("Failed to modify the instruction.\n");
    }
    #endif

    CloseHandle(hProcess);

    #ifdef CLOSEDRIVERERRORWINDOW
        const char *installation = "Installation Warning...";
        closeWindow(installation);
    #endif

    #ifdef CLOSEREGISTRATIONWINDOW
        const char *registerInfo = "About / Registration info...";
        Sleep(200); // just in case, idk if its needed
        closeWindow(registerInfo);
    #endif

    #ifdef CLOSEMAINWINDOW
        const char *voicemeeterMain = "Voicemeeter";
        Sleep(100);
        closeWindow(voicemeeterMain);
    #endif
    
    return 0;
}
