
// changes manually the timer variable to 0, it lets you instantly to be able to close the regitration window, will only work once
#define MODIFY_TIME_LEFT_VARIABLE

// changes the address of the function of the timer so it always sets it to 0, lets you to be able to close the regitration window, will work until the program is closed, normaly every 6 hours
#define MODIFY_FUNCTION_CODE

#if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
#define SLEEPTIME_BETWEN_ATEPMTS_MILISECONDS 5000 // 5 seconds default
#define ATEMPTS 24                                // 2 minutes default
#endif

// closes the driver instalation error window
#define CLOSE_DRIVER_ERROR_WINDOW

#if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
// closes the registration window
#define CLOSE_REGISTRATION_WINDOW
#endif

#define CLOSEMAINWINDOW // recomended with system tray enabled

// if defined, it will log everything that it's doing to console
#define CONSOLE_LOGS

// if defined with a string, it will log everything that has done to a file with the name string of the string provided
#define FILE_LOG_NAME "VoicemeeterBypass.log"