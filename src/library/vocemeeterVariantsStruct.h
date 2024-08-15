#include <stdbool.h>
#include <windows.h>
#include <stdio.h>

#ifndef VOICEMEETER_VARIANTS_STRUCTURE
#define VOICEMEETER_VARIANTS_STRUCTURE
typedef struct ChangeAddressTo
{
    DWORD64 relativeAddress;
    BYTE newValue[15]; // Quita el `const`
} ChangeAddressTo;

// Struct representing possible addresses for the variable and its modifying function
typedef struct VoicemeeterInit
{
    char *processName;
    ChangeAddressTo timeVariableRelativeAddress;
    ChangeAddressTo timeFunctionRelativeAddress;
    ChangeAddressTo windowVariableRelativeAddress;
    ChangeAddressTo windowFunctionRelativeAddress;
} VoicemeeterInit;
#endif