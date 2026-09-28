#include "calc.h"
#include "dicionario.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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
  /* */        { ACAO_OPERA,    ACAO_OPERA,      ACAO_OPERA,      ACAO_EMPILHA,    ACAO_EMPILHA,    ACAO_OPERA  },
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

static int categoria_do_token(Str token){
    if (s_tam(token) != 1) return -1;
    unichar c = s_ch(token, 0);
    return categoria_de(c);
}

static void libera_tudo(Lista tokens, Lista operandos, Lista operadores) {
    while (!l_vazia(operandos)) s_destroi(l_desempilha(operanados));
    l_destroi(operandos);

    l_destroi(operadores);

    while (!l_vazia(tokens)) s_destroi(l_remove(tokens));
    l_destroi(tokens);
}

static bool opera(Lista operandos, unichar op) {
    if(l_tam(operandos) < 2) return false;
    
    Str direito = l_desempilha(operandos);
    Str esquerdo = l_desempilha(operandos);
    double a = s_número(esquerdo);
    double b = s_número(direito);
    double r;
    switch (op) 
    {
    case '+': r = a + b; break;
    case '-': r = a - b; break;
    case '*': r = a * b; break;
    case '/': r = a / b; break;
    case '^': r = pow(a, b); break;
    default: r = 0;
    }

    Str resultado = s_cria_número(r);
    l_empilha(operandos, resultado);
    s_destroi(direito);
    s_destroi(esquerdo); 
    return true;
}

Str calculadora(Str expressão) {
    Lista tokens = tokeniza(expressão);
    int n = l_tam(tokens);
    int i = 0;

    Lista pilha_operandos = l_cria();
    Lista pilha_operadores = l_cria();

    while (true) {
        if(i < n) {
            Str token = l_dado_pos(tokens, i);
            if(categoria_do_token(token) == -1) {
                l_empilha(pilha_operandos, s_cria_cópia(token));
                i++;
                continue;
            } 
        }
        int cat_entrada = (i < n) ? categoria_do_token(l_dado_pos(tokens, i)) : CAT_FIM_OU_VAZIA;
        int cat_topo = l_vazia(pilha_operadores) ? CAT_FIM_OU_VAZIA : categoria_do_token(l_topo(pilha_operadores));
    
        int ação = tabela[cat_topo][cat_entrada];

        if (ação == ACAO_TERMINA) {
            break;
        } else if (ação == ACAO_ERRO) {
            Str erro;
            if (cat_topo == CAT_FIM_OU_VAZIA) {
                erro = s_cria("#ERRO falta de (");
            } else {
                erro = s_cria("#ERRO falta de )");
            }
            libera_tudo(tokens, pilha_operandos, pilha_operadores);
            return erro;
        } else if (ação == ACAO_EMPILHA) {
            l_empilha(pilha_operadores, l_dado_pos(tokens, i));
            i++;
        } else if (ação == ACAO_DESCARTA) {
            l_desempilha(pilha_operadores);
            i++;
        } else if (ação == ACAO_OPERA) {
            Str op = l_desempilha(pilha_operadores);
            if (!opera(pilha_operandos, s_ch(op, 0))) {
                Str erro = s_cria("#ERRO operandos insuficientes");
                libera_tudo(tokens, pilha_operandos, pilha_operadores);
                return erro;
            }
        } 
    }

    if(l_tam(pilha_operandos) != 1) {
        Str erro = s_cria("#ERRp expressão inválida");
        libera_tudo(tokens, pilha_operandos, pilha_operadores);
        return erro;
    }
    
    Str resultado = l_desempilha(pilha_operandos);
    libera_tudo(tokens, pilha_operandos, pilha_operadores);
    return resultado;
}