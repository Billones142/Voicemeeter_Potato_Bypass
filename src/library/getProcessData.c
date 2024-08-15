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
#if (defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)) && defined(DEBUG_LOGS)
  makelog("before searching process\n", true);
#endif
  *processId = GetProcessIdByName(processName);
#if (defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)) && defined(DEBUG_LOGS)
  makelog("after seaching process\n", true);
#endif
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
  DWORD64 baseAddress;
  HANDLE hProcess;
} ProcessData;

void changeAdressTo(AddressTo *adressTo, ProcessData *processData)
{
  const DWORD64 absoluteVariableAddress = processData->baseAddress + adressTo->relativeAddress;
  const BYTE changeValueTo[sizeof(adressTo->newValue)];
  memcpy((void *)changeValueTo, adressTo->newValue, sizeof(changeValueTo));

  if (IsMemoryAccessible(processData->hProcess, (LPVOID)absoluteVariableAddress))
  {
    if (WriteProcessMemory(processData->hProcess, (LPVOID)absoluteVariableAddress, changeValueTo, sizeof(changeValueTo), NULL))
    {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
      makelog("Timer value successfully modified.\n");
#endif
    }
    else
    {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
      makelog("Failed to modify the timer value.\n");
#endif
    }
  }
  else
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog("Error accesing timer value.\n");
#endif
  }
}

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

  inSearchProcessData->baseAddress = GetModuleBaseAddress(inSearchProcessData->processId, inSearchProcessData->processName);
  if (inSearchProcessData->baseAddress == 0)
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

ProcessData *searchVariant(const VoicemeeterInit(voicemeeterInitProvided)[], size_t voicemeeterInitProvidedArrSize)
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

  inSearchProcessData->processName = NULL;
  inSearchProcessData->initChoise = 0;
  inSearchProcessData->processId = 0;
  inSearchProcessData->baseAddress = 0;
  inSearchProcessData->hProcess = 0;

  UINT32 sleepTime = SLEEPTIME_BETWEN_ATEPMTS_MILISECONDS;
  UINT8 attempts = ATEMPTS;

  bool processHasBeenFound = false;
  for (size_t i = attempts; i >= 0; i--)
  {
#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
    makelog(true, "Waiting for Voicemeeter to start... attempts left: %i\n", i);
#endif
    for (int i = 0; i < voicemeeterInitProvidedArrSize; i++) // sizeof() gives the size in bytes of a variable
    {
      if (findProcess(voicemeeterInitProvided[i].processName, &(inSearchProcessData->processId)))
      {
#if (defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)) && defined(DEBUG_LOGS)
        makelog("Process has been found\n", true);
#endif
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
#if (defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)) && defined(DEBUG_LOGS)
    makelog("before asigning string to\n", true);
#endif
    size_t nameLength = strlen(voicemeeterInitProvided[inSearchProcessData->initChoise].processName) + 1; // +1 para el carácter nulo
    inSearchProcessData->processName = (char *)malloc(nameLength);

    snprintf(inSearchProcessData->processName, nameLength, "%s", voicemeeterInitProvided[inSearchProcessData->initChoise].processName);
#if (defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)) && defined(DEBUG_LOGS)
    Sleep(1000);
    makelog("after asigning string to\n", true);
#endif
    return inSearchProcessData;
  }
};