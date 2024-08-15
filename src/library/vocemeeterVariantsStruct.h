#include <stdbool.h>
#include <windows.h>
#include <stdio.h>

#ifndef VOICEMEETER_VARIANTS_STRUCTURE
#define VOICEMEETER_VARIANTS_STRUCTURE
typedef struct AddressTo
{
    DWORD64 relativeAddress;
    BYTE newValue[15]; // Quita el `const`
} AddressTo;

// Struct representing possible addresses for the variable and its modifying function
typedef struct VoicemeeterInit
{
    char *processName;
    AddressTo timeVariableRelativeAddress;
    AddressTo timeFunctionRelativeAddress;
    AddressTo windowVariableRelativeAddress;
    AddressTo windowFunctionRelativeAddress;
} VoicemeeterInit;
#endif