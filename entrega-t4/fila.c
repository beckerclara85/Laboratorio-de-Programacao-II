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
};

struct fila {
    int tam_do_dado; // tamanho em bytes
    Nó *primeiro; // inicio da fila
    Nó *ultimo; // final da fila
    Nó *atual;
    bool para_frente;
};

Fila f_cria(int tam_do_dado){
    Fila f =malloc(sizeof(struct fila));
    assert(f != NULL);
    f->tam_do_dado  = tam_do_dado;
    f->primeiro     = NULL;
    f->ultimo       = NULL;
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

void f_remove(Fila self, void *pdado)
{
  assert(!f_tá_vazia(self));
 
  Nó *n = self->primeiro;
  if (pdado != NULL) {
    memcpy(pdado, n->dado, self->tam_do_dado);
  }
 
  self->primeiro = n->prox;
  if (self->primeiro == NULL) {
    self->ultimo = NULL;
  } else {
    self->primeiro->ant = NULL;
  }
 
  free(n->dado);
  free(n);
}

void f_inicia_percurso(Fila self, int pos_inicial)
{
  Nó *n;
  if (pos_inicial >= 0) {
    self->para_frente = true;
    n = self->primeiro;
    for (int i = 0; i < pos_inicial && n != NULL; i++) {
      n = n->prox;
    }
  } else {
    self->para_frente = false;
    n = self->ultimo;
    for (int i = -1; i > pos_inicial && n != NULL; i--) {
      n = n->ant;
    }
  }
  self->atual = n;
}

bool f_próximo(Fila self, void *pdado)
{
  if (self->atual == NULL) return false;
  if (pdado != NULL) {
    memcpy(pdado, self->atual->dado, self->tam_do_dado);
  }
  if (self->para_frente) {
    self->atual = self->atual->prox;
  } else {
    self->atual = self->atual->ant;
  }
  return true;
}
 