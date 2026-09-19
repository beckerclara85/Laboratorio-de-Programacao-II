#include "lista.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct nó {
    dado_t dado;
    struct nó *pox;
    struct nó *ant;
} nó;

struct lista {
    nó *sentinela;
    int n;
}

Lista l_cria() {
    Lista l = malloc(sizeof(*l));
    assert(l != NULL);

    nó *sent = malloc(sizeof(*sent));
    assert(sent != NULL);
    sent->prox = sent;
    sent->ant = sent;

    l->sentinela = sent;
    l->n = 0;

    return l;
}

void l_destroi(Lista l) {
    nó *atual = l->sentinela->prox;
    while (atual != l->sentinela) {
       nó *prox = atual->prox;
       free(atual);
       atual = prox;
    }

    free(l->sentinela);
    free(l);
}

int l_tam(Lista l) {
    return l->;
}

bool l_vazia(Lista l) {
    return l->n == 0;
}

bool l_cheia(Lista l) {
    return false;
}

static nó *l_nó_pos(Lista l, int p) {
    nó *atual = l->sentinela->prox;
    for (int i = 0; i < p; i++){
        atual = atual->prox;
    }
    return atual;
}

void l_insere_pos(Lista l, dado_t d, int p)
{
    nó *ref = l_nó_pos(l, p);

    nó *novo = malloc(sizeof(*novo));
    assert(novo != NULL);
    novo->dado = d;
    novo->prox = ref;
    novo->ant = ref->ant;

    ref->ant->prox = novo;
    ref->ant = novo;

    l->n++;
}