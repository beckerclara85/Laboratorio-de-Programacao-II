#include "calc.h"
#include "dicionario.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

// auxiliares

static bool é_espaço(unichar c) {
    return c == ' ' || c == '\t' || c == '\n';
}

static bool é_dígito_ou_ponto(unichar c) {
    return c == '.' || (c >= '0' && c <= '9');
}

static bool é_início_de_identificador(unichar c)
{
  return c == '_' || c == '$' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool é_continuação_de_identificador(unichar c) {
    return é_início_de_identificador(c) || (c >= '0' && c <= '9');
}

Lista tokeniza(Str txt) {
    Lista resultado = l_cria();
    int pos = 0;
    int n = s_tam(txt);
    
    while (pos < n) {
        unichar c = s_ch(txt, pos);
        
        if (é_espaço(c)) {
            pos++;
        }else if (é_dígito_ou_ponto(c)) {
            int fim = pos;
            while (fim < n && é_dígito_ou_ponto(s_ch(txt, fim))) {
                fim++;
            }
            Str token = s_cria_substring(txt, pos, fim - pos);
            l_insere_fim(resultado, token);
            pos = fim;
        }else if (é_início_de_identificador(c)) {
            int fim = pos;
            while (fim < n && é_continuação_de_identificador(s_ch(txt, fim))) {
                fim++;
            }
            Str token = s_cria_substring(txt, pos, fim - pos);
            l_insere_fim(resultado, token);
            pos = fim;
        }else {
            Str token = s_cria_substring(txt, pos, 1);
            l_insere_fim(resultado, token);
            pos++;
        }
    }
    return resultado;
}

enum {
    ACAO_EMPILHA,
    ACAO_OPERA,
    ACAO_DESCARTA,
    ACAO_TERMINA,
    ACAO_ERRO
};

enum {
    CAT_FIM_OU_VAZIA,
    CAT_ADITIVO,
    CAT_MULTI,
    CAT_POT,
    CAT_ABRE,
    CAT_FECHA,
    N_CATEGORIAS
};

static const int tabela[N_CATEGORIAS][N_CATEGORIAS] = {
  // coluna:      F                +-               */               ^                (                )
  /* V  */     { ACAO_TERMINA,  ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_ERRO   },
  /* +- */     { ACAO_OPERA,    ACAO_OPERA,      ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_OPERA  },
  /* */ */     { ACAO_OPERA,    ACAO_OPERA,      ACAO_OPERA,      ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_OPERA  },
  /* ^  */     { ACAO_OPERA,    ACAO_OPERA,      ACAO_OPERA,      ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_OPERA  },
  /* (  */     { ACAO_ERRO,     ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_DESCARTA },
};

static int categoria_de(unichar c) {
    switch (c) {
        case '+': case '-': return CAT_ADITIVO;
        case '*': case '/': return CAT_MULTI;
        case '^': return CAT_POT;
        case '(': return CAT_ABRE;
        case ')': return CAT_FECHA;
        default: return -1; 
    }
}

Str calculadora(Str expressão) {

}