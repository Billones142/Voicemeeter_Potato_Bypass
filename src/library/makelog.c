#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

/* #define CONSOLE_LOGS
#define FILE_LOG_NAME "prueba.log" */

#ifdef FILE_LOG_NAME
#include <time.h>
#endif

// Prototipos de las funciones
void makelog_char(const char *message);
void makelog_char_bool(const char *message, bool lineChange);
void makelog_bool_char_any(bool lineChange, const char *message, ...);
void makelog_default(void);

// Macro para detectar el número de argumentos
#define GET_MACRO(_1, _2, _3, NAME, ...) NAME

// Macro para sobrecargar la función makelog
#define makelog(...) GET_MACRO(__VA_ARGS__, makelog_bool_char_any, makelog_char_bool, makelog_char)(__VA_ARGS__)

// Implementación de la función principal de logging
void makelog_char_bool(const char *message, bool lineChange)
{
  char messageFinal[1024];
  if (lineChange)
  {
    snprintf(messageFinal, sizeof(messageFinal), "%s\n", message);
  }
  else
  {
    snprintf(messageFinal, sizeof(messageFinal), "%s", message);
  }

#ifdef CONSOLE_LOGS
  printf("%s", message);
#endif

#ifdef FILE_LOG_NAME
  FILE *logFile = fopen(FILE_LOG_NAME, "a");
  if (logFile != NULL)
  {
    if (lineChange)
    {
      // Obtener la hora actual
      time_t now = time(NULL);
      struct tm *localTime = localtime(&now);

      // Formatear la fecha y hora
      char timeBuffer[20];
      strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

      // Escribir la fecha, hora y mensaje en el archivo de log
      fprintf(logFile, "[%s] %s\n", timeBuffer, message);
    }
    fprintf(logFile, "%s", message);
    fclose(logFile);
  }
  else
  {
    printf("Error opening log file.\n");
  }
#endif
}

// Implementación para logging con y sin salto de línea
void makelog_char(const char *message)
{
  makelog_char_bool(message, true);
}

// Implementación para logging con múltiples argumentos
void makelog_bool_char_any(bool lineChange, const char *message, ...)
{
  char formattedMessage[1024];
  va_list args;
  va_start(args, message);

  vsnprintf(formattedMessage, sizeof(formattedMessage), message, args);
  va_end(args);

  if (lineChange)
  {
    // Agregar un salto de línea al mensaje formateado
    char messageWithNewline[1024];
    snprintf(messageWithNewline, sizeof(messageWithNewline), "%s\n", formattedMessage);
    makelog_char(messageWithNewline);
  }
  else
  {
    // Loguear el mensaje formateado tal cual
    makelog_char(formattedMessage);
  }
}

void makelog_default(void)
{
  makelog_char_bool("unknown makelog arguments", true);
}
