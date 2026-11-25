#include "calc.h"
#include "lista.h"
#include "str.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("uso: %s arquivo_entrada arquivo_saida\n", argv[0]);
        return 1;
    }

    Str entrada = s_cria_de_arquivo(argv[1]);
    Str quebra = s_cria("\r\n");
    Lista linhas = l_cria_separando(entrada, quebra);

    
    Lista saida = l_cria();
    int n = l_tam(linhas);
    for (int i = 0; i < n; i++) {
        l_insere_fim(saida, calculadora(l_dado_pos(linhas, i)));
    }

    Str nl = s_cria("\n");
    Str texto = s_cria_unindo(saida, nl);
    s_anexa(texto, nl);
    s_grava_arquivo(texto, argv[2]);

    s_destroi(texto);
    s_destroi(nl);
    while (!l_vazia(saida)) s_destroi(l_remove(saida));
    l_destroi(saida);
    while (!l_vazia(linhas)) s_destroi(l_remove(linhas));
    l_destroi(linhas);
    s_destroi(quebra);
    s_destroi(entrada);
    return 0;
}
