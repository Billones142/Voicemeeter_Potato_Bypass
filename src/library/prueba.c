#include <stdio.h>

typedef struct Lista {
  int hola;
} Lista;

void cambiar(Lista *hola[]){
  hola[1]->hola= 22;
};

int main() {
  Lista adios[2];
  adios[0].hola= 1;
  adios[1].hola= 2;

  cambiar(&adios);

  printf("%d %d\n", adios[0].hola, adios[1].hola);
};