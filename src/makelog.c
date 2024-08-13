// #define CONSOLE_LOGS
// #define FILE_LOG_NAME "VoicemeeterBypass.log"

#ifdef FILE_LOG_NAME
#include <time.h>
#endif

#if defined(CONSOLE_LOGS) | defined(FILE_LOG_NAME)
// Function to log messages to console and/or file
void makelog(const char *message)
{
#ifdef CONSOLE_LOGS
  printf("%s", message);
#endif

#ifdef FILE_LOG_NAME
  FILE *logFile = fopen(FILE_LOG_NAME, "a");
  if (logFile != NULL)
  {
    // Obtener la hora actual
    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);

    // Formatear la fecha y hora
    char timeBuffer[20];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

    // Escribir la fecha, hora y mensaje en el archivo de log
    fprintf(logFile, "[%s] %s\n", timeBuffer, message);
    fclose(logFile);
  }
  else
  {
    printf("Error opening log file.\n");
  }
#endif
}
#endif