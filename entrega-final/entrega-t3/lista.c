#include "lista.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct nó {
    dado_t dado;
    struct nó *prox;
    struct nó *ant;
} nó;

struct lista {
    nó *sentinela;
    int n;
};

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
    return l->n;
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

void l_insere_inicio(Lista l, dado_t d) {
    l_insere_pos(l, d, 0);
}

void l_insere_fim(Lista l, dado_t d){
    l_insere_pos(l, d, l_tam(l));
}

dado_t l_dado_pos(Lista l, int pos) {
    nó *b = l_nó_pos(l, pos);
    return b->dado;
}

dado_t l_dado_inicio(Lista l) {
    return l_dado_pos(l, 0);
}

dado_t l_dado_fim(Lista l) {
    return l_dado_pos(l, l_tam(l) - 1);
}

dado_t l_remove_pos(Lista l, int pos) {
    nó *alvo = l_nó_pos(l, pos);
    dado_t d = alvo->dado;

    alvo->ant->prox = alvo->prox;
    alvo->prox->ant = alvo->ant;
    
    free(alvo);
    l->n--;

    return d;
}

dado_t l_remove_inicio(Lista l) {
    return l_remove_pos(l, 0);
}

dado_t l_remove_fim(Lista l) {
    return l_remove_pos(l, l_tam(l) - 1);
}

dado_t l_primeiro(Lista l) {
    return l_dado_inicio(l);
}

void l_insere(Lista l, dado_t d) {
    l_insere_fim(l, d);
}

dado_t l_remove(Lista l) {
    return l_remove_inicio(l);
}

dado_t l_topo(Lista l) {
    return l_dado_fim(l);
}

void l_empilha(Lista l, dado_t d) {
    l_insere_fim(l, d);
}

dado_t l_desempilha(Lista l) {
    return l_remove_fim(l);
}

void l_imprime(Lista l) {
    int n = l_tam(l);
    for (int i = 0; i < n; i++) {
        s_imprime(l_dado_pos(l, i));
        printf("\n"); 
    }
}

Lista l_cria_separando(Str s, Str sep) {
    Lista resultado = l_cria();
    int pos = 0;

    while(true) {
        int inicio = s_busca_nc(s, pos, sep);
        if (inicio == -1) break;
        
        int fim = s_busca_c(s, inicio, sep);
        if (fim == -1) fim = s_tam(s);

        Str pedaço = s_cria_substring(s, inicio, fim - inicio);
        l_insere_fim(resultado, pedaço);

        pos = fim;
    }

    return resultado;
}