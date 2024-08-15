#include <windows.h>
#include <stdbool.h>

#ifndef VOICEMEETER_VARIANTS
#define VOICEMEETER_VARIANTS
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

// list of variants of voicemeeter
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

bool checkIf_ChangeAdressTo_IsValid(AddressTo *changeAddressTo)
{
    bool result = false;

    bool relativeAdress_IsValid = changeAddressTo->relativeAddress == 0x0;
    bool changeTo_IsValid = (sizeof(changeAddressTo->newValue) == sizeof(BYTE)) && (changeAddressTo->newValue[0] == 0x0);

    if (relativeAdress_IsValid && changeTo_IsValid)
    {
        result = true;
    }

    return result;
};

#endif