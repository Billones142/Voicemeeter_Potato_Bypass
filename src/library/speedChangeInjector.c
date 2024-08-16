#include "./voicemeeterVariantsStruct.h"

bool InjectDLL(const char* dllPath, const VoicemeeterInit* vmInit) {
    DWORD processId = FindProcessId(vmInit->processName);
    if (processId == 0) {
        printf("Failed to find process: %s\n", vmInit->processName);
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
    if (hProcess == NULL) {
        printf("Failed to open process.\n");
        return false;
    }

    LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (pDllPath == NULL) {
        printf("Failed to allocate memory in the target process.\n");
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pDllPath, (LPVOID)dllPath, strlen(dllPath) + 1, NULL)) {
        printf("Failed to write DLL path into the target process.\n");
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandle("Kernel32");
    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pDllPath, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create remote thread in the target process.\n");
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    printf("DLL successfully injected.\n");
    return true;
}