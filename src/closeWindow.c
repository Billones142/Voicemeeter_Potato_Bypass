#if defined(CLOSE_REGISTRATION_WINDOW) | defined(CLOSEMAINWINDOW) | defined(CLOSE_DRIVER_ERROR_WINDOW)
// Function to close a specific Voicemeeter window
BOOL closeWindow(const char *windowTitle)
{
    HWND hWnd = FindWindow(NULL, windowTitle);

    if (hWnd == NULL)
    {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
        char logMsg[128];
        sprintf(logMsg, "Window (%s) not found.\n", windowTitle);
        makelog(logMsg);
#endif
        return FALSE;
    }

    SendMessage(hWnd, WM_CLOSE, 0, 0);
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    char logMsg[128];
    sprintf(logMsg, "Window (%s) closed successfully.\n", windowTitle);
    makelog(logMsg);
#endif
    return TRUE;
}
#endif