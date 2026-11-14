#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <grafo.h>

typedef struct aresta Aresta;
struct aresta 
{
    int destino;
    void *valor;
    Aresta *prox;
};

typedef struct {
    void *valor;
    Aresta *arestas;
} No;

struct grafo {
    int tam_no;
    int tam_aresta;
    int n_nos;
    int capacidade;
    No *nos;
    bool consulta_partem;
    int consulta_no;
    int origem_atual;
    Aresta *aresta_atual;
};

Grafo g_cria(int tam_nó, int tam_aresta) {
    Grafo g = malloc(sizeoff(struct grafo));
    assert(g != NULL);

    g->tam_no = tam_nó;
    g->tam_aresta = tam_aresta;
    g->n_nos = n_nos;
    g->capacidade = 0
    g->nos = NULL;
    g->consulta_partem = true;
    g->consulta_no = 0;
    g->origem_atual = 0;
    g->aresta_atual = NULL
};

static void liberta_arestas(Aresta *a) {
    while (a != NULL) {
        Aresta *prox = a->prox;
        free(a->valor);
        free(a);
        a = prox;
    }
}

void g_destrói(Grafo self) {
    for (int i = 0; i < self->n-nos; i++) {
        liberta_arestas(self->nos[i].arestas);
        free(self->nos[i].valor);
    }
    free(self->nos);
    free(self);
};

int g_insere_nó(Grafo self, void *p_dado) {
    if (self->n_nos == self->capacidade) {
        if(self->capacidade == 0){
            self->capacidade = 8;
        } else {
            self->capacidade = self->capacidade * 2;
        }
        self->nos = realloc(self-nos, self->capacidade * sizeof(No));
        assert(self->nos != NULL);
    }
    int pos = self->n_nos;
    self->nos[pos].valor = malloc(self->tam_no);
    assert(self->nos[pos].valor != NULL);
    memcpy(self->nos[pos].valor, p_dado, self->tam_no);
    self->nos[pos].arestas = NULL;
    self->n_nos++;
    return pos;
};

void g_remove_nó(Grafo self, int nó) {
    liberta_arestas(self->nos[nó].arestas);
    free(self->nos[nó].valor);
    
    for (int i = 0; i < self->n_nos; i++) {
        if (i == nó) continue;
        Aresta *ant = NULL;
        Aresta *atual = self->nos[i].arestas;
        while(atual != NULL) {
            Aresta *prox = atual->prox;
            if (atual->destino == nó) {
                // ...
            } else {
                //..
                ant = atual;
            }
            atual = prox;
        }
    }
    for (int i = nó; i < self->n_nos - 1; i++){
        self->nos[i] = self->nos[i + 1];
    }
    self->n_nos--;
}