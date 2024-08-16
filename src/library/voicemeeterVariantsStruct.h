#include <stdbool.h>
#include <windows.h>

#ifndef VOICEMEETER_VARIANTS_STRUCTURE
#define VOICEMEETER_VARIANTS_STRUCTURE
typedef struct AddressTo
{
    DWORD64 relativeAddress;
    BYTE newValue[15];
} AddressTo;

typedef struct VoicemeeterInit
{
    char *processName;
    AddressTo timeVariableRelativeAddress;
    AddressTo timeFunctionRelativeAddress;
    AddressTo windowVariableRelativeAddress;
    AddressTo windowFunctionRelativeAddress;
} VoicemeeterInit;
#endif