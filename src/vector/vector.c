#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estructura para representar un vector dinámico genérico
typedef struct
{
  void *data;      // Puntero genérico a los datos
  size_t size;     // Número de elementos almacenados
  size_t capacity; // Capacidad total del vector
  size_t elemSize; // Tamaño de cada elemento
} Vector;

// Función para inicializar el vector
void initVector(Vector *vec, size_t initialCapacity, size_t elemSize)
{
  vec->data = malloc(initialCapacity * elemSize);
  vec->size = 0;
  vec->capacity = initialCapacity;
  vec->elemSize = elemSize;
}

// Función para agregar un elemento al vector
void pushBack(Vector *vec, void *element)
{
  if (vec->size == vec->capacity)
  {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, vec->capacity * vec->elemSize);
  }
  // Copiar el elemento al final del vector
  memcpy((char *)vec->data + vec->size * vec->elemSize, element, vec->elemSize);
  vec->size++;
}

// Función para obtener un elemento del vector por índice
void *getElement(Vector *vec, size_t index)
{
  if (index >= vec->size)
  {
    return NULL; // Índice fuera de los límites
  }
  return (char *)vec->data + index * vec->elemSize;
}

// Función para liberar la memoria del vector
void freeVector(Vector *vec)
{
  free(vec->data);
  vec->data = NULL;
  vec->size = 0;
  vec->capacity = 0;
}