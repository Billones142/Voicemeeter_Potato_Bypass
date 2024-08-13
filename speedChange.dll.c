#include <windows.h>
#include <detours.h>
#include <stdio.h>

static double g_speedMultiplier = 0.1; // Speed multiplier for 0.1x speed

typedef BOOL(WINAPI* QueryPerformanceCounter_t)(LARGE_INTEGER* lpPerformanceCount);
QueryPerformanceCounter_t TrueQueryPerformanceCounter = NULL;

BOOL WINAPI HookedQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
    BOOL result = TrueQueryPerformanceCounter(lpPerformanceCount);
    if (result)
    {
        // Modify the counter to slow down the speed
        lpPerformanceCount->QuadPart = (LONGLONG)(lpPerformanceCount->QuadPart * g_speedMultiplier);
    }
    return result;
}

void InstallHooks()
{
    // Hook QueryPerformanceCounter
    TrueQueryPerformanceCounter = (QueryPerformanceCounter_t)DetourFunction(
        (PBYTE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "QueryPerformanceCounter"),
        (PBYTE)HookedQueryPerformanceCounter
    );
}

void RemoveHooks()
{
    // Remove the hook
    DetourRemove((PBYTE)TrueQueryPerformanceCounter, (PBYTE)HookedQueryPerformanceCounter);
}

int main()
{
    // Begin detour transaction
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    InstallHooks(); // Install the hooks
    DetourTransactionCommit();

    // Keep the application running so the hook stays active
    printf("Speedhack applied. Running at 0.5x speed.\nPress any key to exit...\n");
    getchar(); // Wait for user input to exit

    // Remove hooks when exiting
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    RemoveHooks();
    DetourTransactionCommit();

    return 0;
}
