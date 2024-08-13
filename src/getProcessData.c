

#if defined(MODIFY_TIME_LEFT_VARIABLE) | defined(MODIFY_FUNCTION_CODE)
// Function to get the process ID by name
DWORD GetProcessIdByName(const char *processName)
{
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE)
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("Failed to create snapshot.\n");
#endif
    return 0;
  }

  if (Process32First(snapshot, &entry))
  {
    do
    {
      if (_stricmp(entry.szExeFile, processName) == 0)
      {
        CloseHandle(snapshot);
        return entry.th32ProcessID;
      }
    } while (Process32Next(snapshot, &entry));
  }

  CloseHandle(snapshot);
  return 0;
}

// Function to wait for a process to start running
BOOL findProcess(const char *processName, DWORD *processId)
{
  *processId = GetProcessIdByName(processName);
  return (*processId != 0);
}

// Function to get the base address of a module
DWORD64 GetModuleBaseAddress(DWORD processId, const char *moduleName)
{
  MODULEENTRY32 moduleEntry;
  moduleEntry.dwSize = sizeof(MODULEENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
  if (snapshot == INVALID_HANDLE_VALUE)
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("Failed to create module snapshot.\n");
#endif
    return 0;
  }

  if (Module32First(snapshot, &moduleEntry))
  {
    do
    {
      if (_stricmp(moduleEntry.szModule, moduleName) == 0)
      {
        CloseHandle(snapshot);
        return (DWORD64)moduleEntry.modBaseAddr;
      }
    } while (Module32Next(snapshot, &moduleEntry));
  }

  CloseHandle(snapshot);
  return 0;
}

// Function to check if memory is accessible
BOOL IsMemoryAccessible(HANDLE hProcess, LPCVOID address)
{
  MEMORY_BASIC_INFORMATION mbi;
  if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)))
  {
    return mbi.State == MEM_COMMIT;
  }
  return FALSE;
}
#endif