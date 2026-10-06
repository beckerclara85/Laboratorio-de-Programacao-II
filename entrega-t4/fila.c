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
    int tam_do_dado;
    Nó *primeiro;
    Nó *último;
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

bool f_tá_vazia(Fila self){
    if (self->primeiro == NULL) {
        return true;
    }
    return false;
}