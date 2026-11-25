#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "grafo.h"

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
    Grafo g = malloc(sizeof(struct grafo));
    assert(g != NULL);

    g->tam_no = tam_nó;
    g->tam_aresta = tam_aresta;
    g->n_nos = 0;
    g->capacidade = 0;
    g->nos = NULL;
    g->consulta_partem = true;
    g->consulta_no = 0;
    g->origem_atual = 0;
    g->aresta_atual = NULL;
    return g;
}

static void liberta_arestas(Aresta *a) {
    while (a != NULL) {
        Aresta *prox = a->prox;
        free(a->valor);
        free(a);
        a = prox;
    }
}

void g_destrói(Grafo self) {
    for (int i = 0; i < self->n_nos; i++) {
        liberta_arestas(self->nos[i].arestas);
        free(self->nos[i].valor);
    }
    free(self->nos);
    free(self);
}

int g_insere_nó(Grafo self, void *p_dado) {
    if (self->n_nos == self->capacidade) {
        if(self->capacidade == 0){
            self->capacidade = 8;
        } else {
            self->capacidade = self->capacidade * 2;
        }
        self->nos = realloc(self->nos, self->capacidade * sizeof(No));
        assert(self->nos != NULL);
    }
    int pos = self->n_nos;
    self->nos[pos].valor = malloc(self->tam_no);
    assert(self->nos[pos].valor != NULL);
    memcpy(self->nos[pos].valor, p_dado, self->tam_no);
    self->nos[pos].arestas = NULL;
    self->n_nos++;
    return pos;
}

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
                if (ant == NULL) {
                    self->nos[i].arestas = prox;
                } else {
                    ant->prox = prox;
                }
                free(atual->valor);
                free(atual);
            } else {
                if (atual->destino > nó) {
                    atual->destino--;
                }
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

void g_altera_valor_do_nó(Grafo self, int nó, void *p_dado) {
    memcpy(self->nos[nó].valor, p_dado, self->tam_no);
}

void *g_valor_do_nó(Grafo self, int nó) {
    return self->nos[nó].valor;
}

int g_num_nós(Grafo self) {
    return self->n_nos;
}

void g_altera_valor_da_aresta(Grafo self, int nó_origem, int nó_destino, void *p_dado) {
    Aresta *ant = NULL;
    Aresta *atual = self->nos[nó_origem].arestas;
    while (atual != NULL && atual->destino != nó_destino) {
        ant = atual;
        atual = atual->prox;
    }
    
    if (p_dado == NULL) {
        if (atual == NULL) {
            return;
        } else if (ant == NULL) {
            self->nos[nó_origem].arestas = atual->prox;
        } else {
            ant->prox = atual->prox;
        }
        free(atual->valor);
        free(atual);
    } else if (atual != NULL) {
        memcpy(atual->valor, p_dado, self->tam_aresta);
    } else {
        Aresta *nova = malloc(sizeof(Aresta));
        assert(nova != NULL);
        nova->valor = malloc(self->tam_aresta);
        assert(nova->valor != NULL);
        memcpy(nova->valor, p_dado, self->tam_aresta);
        nova->destino = nó_destino;

        nova->prox = self->nos[nó_origem].arestas;
        self->nos[nó_origem].arestas = nova;
    }
}

void *g_valor_da_aresta(Grafo self, int nó_origem, int nó_destino) {
    Aresta *a = self->nos[nó_origem].arestas;
    while(a != NULL) {
        if (a->destino == nó_destino) {
            return a->valor;
        }
        a = a->prox;
    }
    return NULL;
}

void g_arestas_que_partem(Grafo self, int nó_origem) {
    self->consulta_partem = true;
    self->consulta_no = nó_origem;
    self->aresta_atual = self->nos[nó_origem].arestas;

}

void g_arestas_que_chegam(Grafo self, int nó_destino) {
    self->consulta_partem = false;
    self->consulta_no = nó_destino;
    self->origem_atual = 0;
    if (self->n_nos > 0) {
        self->aresta_atual = self->nos[0].arestas;
    } else {
        self->aresta_atual = NULL;
    }
}

void *g_próxima_aresta(Grafo self, int *p_nó_vizinho) {
    if (self->consulta_partem) {
        if(self->aresta_atual == NULL) return NULL;
        Aresta *a = self->aresta_atual;
        self->aresta_atual = a->prox;
        if(p_nó_vizinho != NULL) {
            *p_nó_vizinho = a->destino;
        }
        return a->valor;
    }

    while (self->origem_atual < self->n_nos) {
        while(self->aresta_atual != NULL) {
            Aresta *a = self->aresta_atual;
            self->aresta_atual = a->prox;
            if (a->destino == self->consulta_no) {
                if (p_nó_vizinho != NULL) {
                    *p_nó_vizinho = self->origem_atual;
                }
                return a->valor;
            }
        }
        self->origem_atual++;
        if (self->origem_atual < self->n_nos) {
            self->aresta_atual = self->nos[self->origem_atual].arestas;
        }
    }
    return NULL;
}

bool g_tem_ciclo(Grafo self) {
    return false;
}

Fila g_ordem_topológica(Grafo self) {
    Fila f = f_cria(sizeof(int));
    for (int i = 0; i < self->n_nos; i++) {
        f_insere(f, &i);
    }
    return f;
}