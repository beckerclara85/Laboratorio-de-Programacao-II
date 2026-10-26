#include "fila.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

typedef struct nó Nó;
struct nó {
    void *dado;
    Nó *ant;
    Nó *prox;
}

struct fila {
    int tam_do_dado; // tamanho em bytes
    Nó *primeiro; // inicio da fila
    Nó *último; // final da fila
    Nó *atual;
    bool para_frente;
}

Fila f_cria(int tam_do_dado){
    Fila f =malloc(sizeof(struct fila));
    assert(f != NULL);
    f->tam_do_dado  = tam_do_dado;
    f->primeiro     = NULL;
    f->último       = NULL;
    f->atual        = NULL;
    f->para_frente  = false;
    return f;
}

void f_destrói(Fila self)
{
  Nó *n = self->primeiro;
  while (n != NULL) {
    Nó *prox = n->prox; 
    free(n->dado);
    free(n);
    n = prox;
  }
  free(self);
}

bool f_tá_vazia(Fila self){
    if (self->primeiro == NULL) {
        return true;
    }
    return false;
}

void f_insere(Fila self, void *pdado)
{
  Nó *novo = malloc(sizeof(Nó));
  assert(novo != NULL);
  novo->dado = malloc(self->tam_do_dado);
  assert(novo->dado != NULL);
  memcpy(novo->dado, pdado, self->tam_do_dado);
 
  novo->prox = NULL;
  novo->ant  = self->ultimo;
  if (self->ultimo == NULL) {
    // fila vazia: o novo nó é o primeiro e o último
    self->primeiro = novo;
  } else {
    self->ultimo->prox = novo;
  }
  self->ultimo = novo;
}
 