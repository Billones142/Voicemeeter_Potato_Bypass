#include <windows.h>
#include <tlhelp32.h>
#include <stdbool.h>
#include "./voicemeeterVariants.c"

#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
#include "./makelog.c"
#endif

#if !defined(SLEEPTIME_BETWEN_ATEPMTS_MILISECONDS) | !defined(ATEMPTS)
#warning time attempt variables not defined in config, using default
#define SLEEPTIME_BETWEN_ATEPMTS_MILISECONDS 5000
#define ATEMPTS 24
#endif

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
bool findProcess(const char *processName, DWORD *processId)
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
bool IsMemoryAccessible(HANDLE hProcess, LPCVOID address)
{
  MEMORY_BASIC_INFORMATION mbi;
  if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)))
  {
    return mbi.State == MEM_COMMIT;
  }
  return false;
}

typedef struct ProcessData
{
  char *processName;
  int initChoise;
  DWORD processId;
  DWORD64 voicemeeterBaseAddress;
  HANDLE hProcess;
} ProcessData;

ProcessData *getProcessData(ProcessData *inSearchProcessData) // TODO: dividir el buscador de la variante de voicemeeter
{
  if (inSearchProcessData == NULL)
  {
    return NULL;
  }

  /*  */

  inSearchProcessData->hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_VM_READ, false, inSearchProcessData->processId);
  if (inSearchProcessData->hProcess == NULL)
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("Failed to open the process.\n");
#endif
    free(inSearchProcessData);
    return NULL;
  }

  inSearchProcessData->voicemeeterBaseAddress = GetModuleBaseAddress(inSearchProcessData->processId, inSearchProcessData->processName);
  if (inSearchProcessData->voicemeeterBaseAddress == 0)
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("Failed to find the module base address.\n");
#endif
    CloseHandle(inSearchProcessData->hProcess);
    free(inSearchProcessData);
    return NULL;
  }

  return inSearchProcessData;
}

ProcessData *searchVariant(const VoicemeeterInit (voicemeeterInitProvided)[], size_t voicemeeterInitProvidedArrSize)
{
  ProcessData *inSearchProcessData = (ProcessData *)malloc(sizeof(ProcessData));
  if (inSearchProcessData == NULL)
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("Fallo de asignacion de memoria para searchedProcessData, funcion searchVariant", true);
#endif
    // fallo de asignacion
    return NULL;
  }

  inSearchProcessData->initChoise = 0;
  inSearchProcessData->processId = 0;
  inSearchProcessData->voicemeeterBaseAddress = 0;
  inSearchProcessData->hProcess = 0;

  UINT32 sleepTime = SLEEPTIME_BETWEN_ATEPMTS_MILISECONDS;
  UINT8 attempts = ATEMPTS;

  bool processHasBeenFound = false;
  for (size_t i = attempts; i >= 0; i--)
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog(true, "Waiting for Voicemeeter to start... attempts left: %i\n", attempts);
#endif
    for (int i = 0; i < voicemeeterInitProvidedArrSize; i++) // sizeof() gives the size in bytes of a variable
    {
      if (findProcess(voicemeeterInitProvided[i].processName, &(inSearchProcessData->processId)))
      {
        processHasBeenFound = true;

        inSearchProcessData->initChoise = i;
        break; // PORQUE?????????????????, si no esta esplosta todo
               // tambien no se porque no se me ocurrio antes...
               // era por el sizeof lptm, no vi que sacaba la cantidad de bytes y no de elementos en el array, soy boludo me olvide que este es un leguaje low level
      }
    }
    if (processHasBeenFound == true)
    {
      break;
    }

    Sleep(sleepTime); // Wait 5 seconds before trying again
  }

  if (processHasBeenFound == false) // if not found
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("La funcion searchVariant no encontro el proceso", true);
#endif
    free(inSearchProcessData);
    return NULL;
  }
  else
  {
    return inSearchProcessData;
  }
};