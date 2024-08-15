#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <stdbool.h>

#define MODIFY_TIME_LEFT_VARIABLE
#define MODIFY_FUNCTION_CODE

#define CLOSE_DRIVER_ERROR_WINDOW
#define CLOSE_REGISTRATION_WINDOW
#define CLOSEMAINWINDOW // recomended with system tray enabled

#define CONSOLE_LOGS
#define FILE_LOG_NAME "VoicemeeterBypass.log"

#include "makelog.c"
#include "getProcessData.c"
#include "closeWindow.c"

typedef struct ChangeAdressTo
{
    DWORD64 relativeAddress;
    BYTE newValue[15]; // 15 just in case its needed
} ChangeAdressTo;

// Struct representing possible addresses for the variable and its modifying function
typedef struct VoicemeeterInit
{
    char *processName;
    ChangeAdressTo timeVariableRelativeAddress;
    ChangeAdressTo timeFunctionRelativeAddress;
    ChangeAdressTo windowVariableRelativeAddress;
    ChangeAdressTo windowFunctionRelativeAddress;
} VoicemeeterInit;


// variants of voicemeeter
const VoicemeeterInit initVoicemeeter[] = {
    {
        "voicemeeter8.exe", // Voicemeeter Potato x32
        {0x13B518, {0x0, 0x0, 0x0, 0x0}},
        {0x13CEE, {0xC7, 0x87, 0x38, 0x0A, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90}},
        {0x0, {0x0}},
        {0x0, {0x0}},
    },

    {
        "voicemeeter8x64.exe", // Voicemeeter Potato x64
        {0x156858, {0x0, 0x0, 0x0, 0x0}},
        {0x13D2E, {0x41, 0xC7, 0x84, 0x24, 0x68, 0x0A, 0x00, 0x00, 0x00, 0x00}},
        {0x0, {0x0}},
        {0x12AD3, {0x41, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x90}},
    },
};

int main(int argc, char **argv)
{
#if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
    DWORD processId;

    UINT8 initChoice = 0;
    DWORD64 voicemeeterBaseAddress;

    UINT32 sleepTime = 5000; // 5 seconds
    UINT8 attempts = 24;     // 2 minutes

    BOOL processHasBeenFound = FALSE;
    while (attempts > 0)
    {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        char logMsg[128];
        sprintf(logMsg, "Waiting for Voicemeeter to start... attempts left: %i\n", attempts);
        makelog(logMsg);
#endif
        for (int i = 0; i < sizeof(initVoicemeeter) / sizeof(VoicemeeterInit); i++) // sizeof() gives the size in bytes of a variable
        {
            if (findProcess(initVoicemeeter[i].processName, &processId))
            {
                processHasBeenFound = TRUE;
                initChoice = i;
                break; // PORQUE?????????????????, si no esta esplosta todo (era por el sizeof lptm, no vi que sacaba la cantidad de bytes y no de elementos en el array), tambien no se porque no se me ocurrio antes...
            }
        }
        if (processHasBeenFound == TRUE)
        {
            break;
        }

        Sleep(sleepTime); // Wait 5 seconds before trying again
        attempts--;
    }
    if (processHasBeenFound == FALSE) // if not found
    {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog("no se encontro el proceso\n");
#endif
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
    const DWORD64 absoluteVariableAddress = voicemeeterBaseAddress + initVoicemeeter[initChoice].timeVariableRelativeAddress.relativeAddress;
    const BYTE changeValueTo[sizeof(initVoicemeeter[initChoice].timeVariableRelativeAddress.newValue)];
    memcpy((void *)changeValueTo, initVoicemeeter[initChoice].timeVariableRelativeAddress.newValue, sizeof(changeValueTo));

    if (IsMemoryAccessible(hProcess, (LPVOID)absoluteVariableAddress))
    {
        if (WriteProcessMemory(hProcess, (LPVOID)absoluteVariableAddress, changeValueTo, sizeof(changeValueTo), NULL))
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
    }
    else
    {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog("Error accesing timer value.\n");
#endif
    }

#endif

#ifdef MODIFY_FUNCTION_CODE
    const DWORD64 absoluteFunctionAddress = voicemeeterBaseAddress + initVoicemeeter[initChoice].timeFunctionRelativeAddress.relativeAddress;
    const BYTE changeFunctionTo[sizeof(initVoicemeeter[initChoice].timeFunctionRelativeAddress.newValue)];
    memcpy((void *)changeFunctionTo, initVoicemeeter[initChoice].timeFunctionRelativeAddress.newValue, sizeof(changeFunctionTo));

    if (IsMemoryAccessible(hProcess, (LPVOID)absoluteFunctionAddress))
    {
        if (WriteProcessMemory(hProcess, (LPVOID)absoluteFunctionAddress, changeFunctionTo, sizeof(changeFunctionTo), NULL))
        {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
            makelog("Instruction modified succesfully.\n");
#endif
        }
        else
        {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
            makelog("Error whille modifing the instruction.\n");
#endif
        }
    }
    else
    {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog("Error while accesing the instruction.\n");
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
