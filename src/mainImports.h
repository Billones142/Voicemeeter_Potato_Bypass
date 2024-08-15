#include "./compileConfig.h"
#include "./library/voicemeeterVariants.c"

// #include "./library/makelog.c"
#if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
#include "./library/getProcessData.c"
#endif

#if defined(CLOSE_REGISTRATION_WINDOW) | defined(CLOSEMAINWINDOW) | defined(CLOSE_DRIVER_ERROR_WINDOW)
#include "./library/closeWindow.c"
#endif