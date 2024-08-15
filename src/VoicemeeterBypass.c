#include <windows.h>
// #include <tlhelp32.h>
#include "./mainImports.h"

int main(int argc, char **argv)
{
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
            makelog("Starting voicemeeter bypass\n", true);
#endif
    ProcessData *processData = searchVariant(&initVoicemeeter, sizeof(initVoicemeeter) / sizeof(VoicemeeterInit));
    if (processData == NULL)
    {
        return 1;
        // makelog
    }
    
    getProcessData(processData);


#ifdef CLOSE_DRIVER_ERROR_WINDOW
    const char *installation = "Installation Warning...";
    closeWindow(installation);
#endif

#ifdef MODIFY_TIME_LEFT_VARIABLE
    const DWORD64 absoluteVariableAddress = processData->voicemeeterBaseAddress + initVoicemeeter[processData->initChoise].timeVariableRelativeAddress.relativeAddress;
    const BYTE changeValueTo[sizeof(initVoicemeeter[processData->initChoise].timeVariableRelativeAddress.newValue)];
    memcpy((void *)changeValueTo, initVoicemeeter[processData->initChoise].timeVariableRelativeAddress.newValue, sizeof(changeValueTo));

    if (IsMemoryAccessible(processData->hProcess, (LPVOID)absoluteVariableAddress))
    {
        if (WriteProcessMemory(processData->hProcess, (LPVOID)absoluteVariableAddress, changeValueTo, sizeof(changeValueTo), NULL))
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
    const DWORD64 absoluteFunctionAddress = processData->voicemeeterBaseAddress + initVoicemeeter[processData->initChoise].timeFunctionRelativeAddress.relativeAddress;
    const BYTE changeFunctionTo[sizeof(initVoicemeeter[processData->initChoise].timeFunctionRelativeAddress.newValue)];
    memcpy((void *)changeFunctionTo, initVoicemeeter[processData->initChoise].timeFunctionRelativeAddress.newValue, sizeof(changeFunctionTo));

    if (IsMemoryAccessible(processData->hProcess, (LPVOID)absoluteFunctionAddress))
    {
        if (WriteProcessMemory(processData->hProcess, (LPVOID)absoluteFunctionAddress, changeFunctionTo, sizeof(changeFunctionTo), NULL))
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
    CloseHandle(processData->hProcess);
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
