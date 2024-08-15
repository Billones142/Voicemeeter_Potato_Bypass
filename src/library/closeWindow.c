#include <stdbool.h>
#include <Windows.h>

// Function to close a specific Voicemeeter window
bool closeWindow(const char *windowTitle)
{
    HWND hWnd = FindWindow(NULL, windowTitle);

    if (hWnd == NULL)
    {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        makelog(true,"Window (%s) not found.", windowTitle);
#endif

        return false;
    }

    SendMessage(hWnd, WM_CLOSE, 0, 0);
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    char logMsg[128];
    sprintf(logMsg, "Window (%s) closed successfully.\n", windowTitle);
    makelog("Window (%s) closed successfully.\n", windowTitle);
    // makelog(bool,"Window (%s) closed successfully.", windowTitle);
#endif
    return true;
}