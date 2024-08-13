#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

#define MODIFY_TIME_LEFT_VARIABLE
#define MODIFY_FUNCTION_CODE

#define CLOSE_DRIVER_ERROR_WINDOW
#define CLOSE_REGISTRATION_WINDOW
#define CLOSEMAINWINDOW // recomended with system tray enabled

#define CONSOLE_LOGS
#define FILE_LOG_NAME "VoicemeeterBypass.log"

// Struct representing possible addresses for the variable and its modifying function
typedef struct
{
    char *processName;
    DWORD64 relativeVariableAddress;
    DWORD64 relativeFunctionAddress;
    BYTE newInstruction[10];
} VoicemeeterInit;

// variants of voicemeeter
const VoicemeeterInit initVoicemeeter[] = {
    { "voicemeeter8.exe",    0x13B518, 0x13CEE, { 0xC7, 0x87, 0x38, 0x0A, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 }/*mov [edi+00000A38],0 nop nop nop*/}, //Voicemeeter Potato x32
    { "voicemeeter8x64.exe", 0x156858, 0x10901, { 0x41, 0xC7, 0x84, 0x24, 0x68, 0x0A, 0x00, 0x00, 0x00, 0x00 }/*mov [r12+00000A68],0*/},             //Voicemeeter Potato x64
};

#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
// Function to log messages to console and/or file
void makelog(const char *message)
{
    #ifdef CONSOLE_LOGS
        printf("%s", message);
    #endif

    #ifdef FILE_LOG_NAME
        FILE *logFile = fopen(FILE_LOG_NAME, "a");
        if (logFile != NULL)
        {
            fprintf(logFile, "%s", message);
            fclose(logFile);
        }
        else
        {
            printf("Error opening log file.\n");
        }
    #endif
}
#endif

#if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
// Function to get the process ID by name
DWORD GetProcessIdByName(const char *processName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog("Failed to create snapshot.\n");
        #endif
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

// Function to wait for a process to start running
BOOL findProcess(const char *processName, DWORD *processId)
{
    *processId = GetProcessIdByName(processName);
    return (*processId != 0);
}


// Function to get the base address of a module
DWORD64 GetModuleBaseAddress(DWORD processId, const char *moduleName)
{
    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog("Failed to create module snapshot.\n");
        #endif
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
#endif


#if defined(CLOSE_REGISTRATION_WINDOW) | defined(CLOSEMAINWINDOW) | defined(CLOSE_DRIVER_ERROR_WINDOW)
// Function to close a specific Voicemeeter window
BOOL closeWindow(const char *windowTitle)
{
    HWND hWnd = FindWindow(NULL, windowTitle);

    if (hWnd == NULL)
    {
        #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog("Window not found.\n");
        #endif
        return FALSE;
    }

    SendMessage(hWnd, WM_CLOSE, 0, 0);
    #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("Window closed successfully.\n");
    #endif
    return TRUE;
}
#endif

int main(int argc, char **argv)
{
    #if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
    DWORD processId;

    UINT8 initChoice= 0;
    DWORD64 voicemeeterBaseAddress;

    UINT32 sleepTime = 5000; // 5 seconds
    UINT8 attempts = 24;     // 2 minutes

    BOOL processHasBeenFound= FALSE;
    while (attempts > 0)
    {
        #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        char logMsg[128];
        sprintf(logMsg, "Waiting for Voicemeeter to start... attempts left: %i\n", attempts);
        makelog(logMsg);
        #endif
        for (int i = 0; i < sizeof(initVoicemeeter); i++)
        {
            if (findProcess(initVoicemeeter[i].processName, &processId))
            {
                processHasBeenFound= TRUE;
                initChoice= i;
                break; // PORQUE?????????????????, si no esta esplosta todo, tambien no se porque no se me ocurrio antes...
            }
        }
        if (processHasBeenFound == TRUE)
        {
            makelog("se encontro el proceso\n");
            break;
        }

        Sleep(sleepTime); // Wait 5 seconds before trying again
        attempts--;
    }
    if (processHasBeenFound == FALSE) // if not found
    {
        makelog("no se encontro el proceso\n");
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL)
    {
        #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog("Failed to open the process.\n");
        #endif
        return 1;
    }

    voicemeeterBaseAddress = GetModuleBaseAddress(processId, initVoicemeeter[initChoice].processName);
    if (voicemeeterBaseAddress == 0)
    {
        #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog("Failed to find the module base address.\n");
        #endif
        CloseHandle(hProcess);
        return 1;
    }
    #endif

    #ifdef CLOSE_DRIVER_ERROR_WINDOW
        const char *installation = "Installation Warning...";
        closeWindow(installation);
    #endif

    #ifdef MODIFY_TIME_LEFT_VARIABLE
        const DWORD64 absoluteVariableAddress = voicemeeterBaseAddress + initVoicemeeter[initChoice].relativeVariableAddress;
        const BYTE valorCero[4] = {0x0, 0x0, 0x0, 0x0};

        if (IsMemoryAccessible(hProcess,(LPVOID)absoluteVariableAddress))
        {
            if (WriteProcessMemory(hProcess, (LPVOID)absoluteVariableAddress, valorCero, sizeof(valorCero), NULL))
            {
                #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
                makelog("Timer value successfully modified.\n");
                #endif
            }
            else
            {
                #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
                makelog("Failed to modify the timer value.\n");
                #endif
            }
        } else
        {
            #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
            makelog("Error accesing timer value.\n");
            #endif
        }
        
        
    #endif

    #ifdef MODIFY_FUNCTION_CODE
        const DWORD64 absoluteFunctionAddress = voicemeeterBaseAddress + initVoicemeeter[initChoice].relativeFunctionAddress;

        if (IsMemoryAccessible(hProcess,(LPVOID)absoluteFunctionAddress))
        {
            if (WriteProcessMemory(hProcess, (LPVOID)absoluteFunctionAddress, initVoicemeeter[initChoice].newInstruction, sizeof(initVoicemeeter[initChoice].newInstruction), NULL))
            {
                #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
                makelog("Instruccion modificada exitosamente.\n");
                #endif
            }
            else
            {
                #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
                makelog("Error al modificar la instruccion.\n");
                #endif
            }
        } else
        {
            #if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
            makelog("Error al acceder a la instruccion.\n");
            #endif
        }
    #endif

    #if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
        CloseHandle(hProcess);
    #endif

    #ifdef CLOSE_REGISTRATION_WINDOW
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
