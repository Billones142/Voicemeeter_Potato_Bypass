#include <windows.h>
// #include <tlhelp32.h>
#include "./mainImports.h"

int main(int argc, char **argv)
{
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("Starting voicemeeter bypass\n", true);
#endif
    ProcessData *processData = searchVariant(&initVoicemeeter, sizeof(initVoicemeeter) / sizeof(VoicemeeterInit));
#if (defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)) && defined(DEBUG_LOGS)
    makelog("after searching variant\n", true);
#endif
    if (processData == NULL)
    {
#if (defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)) && defined(DEBUG_LOGS)
        makelog("process data is null", true);
#endif
        return 1;
        // makelog
    }

    getProcessData(processData);

#ifdef CLOSE_DRIVER_ERROR_WINDOW
    const char *installation = "Installation Warning...";
    closeWindow(installation);
#endif

#ifdef MODIFY_TIME_LEFT_VARIABLE
    changeAdressTo(&(initVoicemeeter[processData->initChoise].timeVariableRelativeAddress), processData);
#endif

#ifdef MODIFY_FUNCTION_CODE
    changeAdressTo(&(initVoicemeeter[processData->initChoise].timeVariableRelativeAddress), processData);
#endif

#if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
    CloseHandle(processData->hProcess);
#endif

#ifdef CLOSE_REGISTRATION_WINDOW
    Sleep(200); // just in case, idk if its needed
    closeWindow("About / Registration info...");
#endif

#ifdef CLOSEMAINWINDOW
    Sleep(100);
    closeWindow("Voicemeeter");
#endif

    return 0;
}
