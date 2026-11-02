#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
} 

Grafo g_cria(int tam_nó, int tam_aresta) {
    
}