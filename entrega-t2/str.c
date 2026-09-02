// includes, constantes e declarações {{{1
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MIN_ALLOC 8    // alocação mínima

struct str {
    byte *dados; 
    int nbytes;
    int alloc;
};
// A memória para conter os bytes de uma string deve ser alocada e/ou
//   realocada conforme a necessidade, cuidando para que a quantidade
//   de memória alocada seja sempre:
//   - nula (não alocada) se a string for vazia, ou
//   - não inferior ao necessário para armazenar os bytes da codificação utf8;
//   - não inferior à alocação mínima;
//   - não superior ao triplo do número de bytes necessários
//     (exceto quando for o mínimo);
//   - uma potência de 2.

// funções auxiliares {{{1

// verifica se a string cad está de acordo com a especificação
// aborta o programa se não tiver
static void s_ok(Str_c s)
{
  if (s == NULL) return;
  if (s->nbytes == 0){
    assert(s->dados == NULL);
    assert(s->alloc == 0);
  } else {
    assert(s->dados != NULL);
    assert(s->alloc >= s->nbytes);
    assert(s->alloc >= MIN_ALLOC);
    assert(s->alloc <= 3 * s-> nbytes || s->alloc == MIN_ALLOC);
    assert((s->alloc & (s->alloc - 1)) == 0);

    int ncar = u8_conta_unichar_nos_bytes(s->nbytes, s->dados);
    assert(ncar >= 0);
  }
}

static int s_potencia2(int n) {
  int p = MIN_ALLOC;
  while (p < n) {
    p = p * 2;
  }
  return p;
}

static int s_clamp(int v, int min, int max) {
  if (v < min) return min;
  if (v > max) return max;
  return v;
}

static void s_redimensiona(Str s, int nbytes){
  int novo_alloc = s_potencia2(nbytes);
  byte *novo_dados = realloc(s->dados, novo_alloc);
  assert(novo_dados != NULL);

  s->dados = novo_dados;
  s->alloc = novo_alloc;
}

static void s_garante_espaco(Str s, int nbytes){
  if (nbytes <= s->alloc) {
    return;
  }
  s_redimensiona(s, nbytes);
}

static void s_encolhe_se_necessario(Str s){
  if(s->nbytes == 0) {
    free(s->dados);
    s->dados = NULL;
    s->alloc = 0;
  } else if (s->alloc > 3* s->nbytes && s->alloc > MIN_ALLOC){
    s_redimensiona(s, s->nbytes);
  }
}

// operações de criação e destruição {{{1

Str s_cria(char const *strC)
{
  Str s = malloc(sizeof(*s));
  assert(s != NULL);
  s->dados = NULL;
  s->nbytes = 0;
  s->alloc = 0;

  if(strC != NULL) {
    int len = strlen(strC);
    int ncar = u8_conta_unichar_nos_bytes(len, (byte *) strC);
    if (ncar >= 0) {
      if (len > 0) {
        s_garante_espaco(s, len);
        memcpy(s->dados, strC, len);
        s->nbytes = len;
      }
    }
  }
  s_ok(s);
  return s;
}

void s_destroi(Str s)
{
  s_ok(s);
  free(s->dados);
  free(s);
}

Str s_cria_substring(Str_c s, int pos, int tam)
{
   Str nova = s_cria("");
   s_substring(nova, s, pos, tam);
   return nova;
}

Str s_cria_cópia(Str_c s)
{
   return s_cria_substring(s, 0, -1);
}

// Retorna uma nova string com o conteúdo do arquivo chamado nome.
// Retorna uma string vazia em caso de erro.
Str s_cria_de_arquivo(char *nome)
{
  Str s = s_cria("");
  
  FILE *f  = fopen(nome, "rb");
  if (f == NULL) return s;

  fseek(f, 0, SEEK_END);
  long tam = ftell(f);
  rewind(f);

  if (tam > 0) {
    byte *buf = malloc(tam);
    assert(buf != NULL);
    size_t lidos = fread(buf, 1, tam, f);

    if (lidos == (size_t) tam) {
      int ncar = u8_conta_unichar_nos_bytes((int) tam, buf);
      if (ncar >= 0) {
        s_garante_espaco(s, (int) tam);
        memcpy(s->dados, buf, tam);
        s->nbytes = (int) tam;
      }
    }
    free(buf);
  }

  fclose(f);
  s_ok(s);
  return s;
}

// operações de acesso {{{1

int s_tam(Str_c s)
{
  s_ok(s);
  return u8_conta_unichar_nos_bytes(s->nbytes, s->dados);
}

char *s_strc(Str_c s)
{
  s_ok(s);
  char *r = malloc(s->nbytes + 1);
  assert(r != NULL);
  if (s->nbytes > 0) {
    memcpy(r, s->dados, s->nbytes);
  }
  r[s->nbytes] = '\0';
  return r;
}

unichar s_ch(Str_c s, int pos)
{
  s_ok(s);
  int ncar = s_tam(s);
  int idx = (pos >= 0) ? pos : ncar + pos + 1;
  if (idx < 0 || idx >= ncar) {
    return UNI_INV;
  }

  byte *p = u8_avanca_unichar(s->dados, idx);
  int maxn = s->nbytes - (p - s->dados);
  unichar uni;
  int nb = u8_unichar_nos_bytes(maxn, p, &uni);
  assert(nb >= 0);
  return uni;
}


// operações de busca e comparação {{{1

static int s_offset_bytes(Str_c s, int idx_char) {
  if (idx_char <=0 || s->nbytes ==0) return 0;
  byte *p =u8_avanca_unichar(s->dados,idx_char);
  assert(p != NULL);
  return (int)(p - s->dados);
}

static bool s_contem_char(Str_c sb, unichar c) {
  int n = s_tam(sb);
  for (int i = 0; i < n; i++) {
    if (s_ch(sb, i) == c) return true;
  }
  return false;
}

bool s_igual(Str_c s, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  if (s->nbytes != sb->nbytes) return false;
  if (s->nbytes == 0) return true;
  return memcmp(s->dados, sb->dados, s->nbytes) == 0;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int ncar = s_tam(s);
  int idx = (pos >= 0) ? pos : ncar + pos + 1;
  if (idx < 0) idx = 0;
  for (int i = idx; i < ncar; i++){
    if(s_contem_char(sb, s_ch(s, i))) return i;
  } 
  return -1;
}

int s_busca_nc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int ncar = s_tam(s);
  int idx = (pos >= 0) ? pos : ncar + pos + 1;
  for (int i = idx; i < ncar; i++){
    if(!s_contem_char(sb, s_ch(s, i))) return i;
  } 
  return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int ncar = s_tam(s);
  int idx = (pos >= 0) ? pos : ncar + pos + 1;
  if  (idx > ncar) idx = ncar;
  for (int i = idx - 1; i >= 0; i--) {
    if (s_contem_char(sb, s_ch(s,i))) return i;
  }
  return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int ncar = s_tam(s);
  int idx = (pos >= 0) ? pos : ncar + pos + 1;
  if  (idx > ncar) idx = ncar;
  for (int i = idx - 1; i >= 0; i--) {
    if (!s_contem_char(sb, s_ch(s,i))) return i;
  }
  return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
  s_ok(s);
  s_ok(buscada);
  int ncar = s_tam(s);
  int idx = (pos >= 0) ? pos : ncar + pos + 1;
  if (idx < 0) idx = 0;
  if (idx > ncar) idx = ncar;

  int nb = s_tam(buscada);
  if (nb == 0) return idx;

  for (int i  = idx; i + nb <= ncar; i++) {
    int off1 = s_offset_bytes(s, i);
    int off2 = s_offset_bytes(s, i + nb);
    int len = off2 - off1;
    if  (len == buscada->nbytes && memcmp(s->dados + off1, buscada->dados, len) == 0) {
      return i;
    }
  }
  return -1;
}


// operações de alteração {{{1

void s_substitui(Str s, int pos, int tam, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  
  int ncar = s_tam(s);
  int start_raw = (pos >= 0) ? pos : ncar + pos + 1;
  int end_raw = (tam < 0) ? ncar : start_raw + tam;

  int p0 = s_clamp(start_raw, 0, ncar);
  int p1 = s_clamp(end_raw, 0, ncar);
  if (p1 < p0) p1 = p0;

  int b0 = s_offset_bytes(s, p0);
  int b1 = s_offset_bytes(s, p1);

  int nbytes_sb = (sb == NULL) ? 0 : sb->nbytes;
  int novo_nbytes = s->nbytes - (b1 - b0) + nbytes_sb;

  s_garante_espaco(s, novo_nbytes);

  memmove(s->dados + b0 + nbytes_sb, s->dados + b1, s->nbytes - b1);
  if (nbytes_sb > 0) {
    memcpy(s->dados + b0, sb->dados, nbytes_sb);
  }
  
  s->nbytes = novo_nbytes;
  s_encolhe_se_necessario(s);
  s_ok(s);
}

void s_substring(Str s, Str_c sb, int pos, int tam)
{
  s_ok(s);
  s_ok(sb);

  int ncar = s_tam(sb);
  int start_raw = (pos >= 0) ? pos : ncar + pos + 1;
  int end_raw = (tam < 0) ? ncar : start_raw + tam;

  int p0 = s_clamp(start_raw, 0, ncar);
  int p1 = s_clamp(end_raw, 0, ncar);
  if (p1 < p0) p1 = p0;

  int b0 = s_offset_bytes(sb, p0);
  int b1 = s_offset_bytes(sb, p1);
  int novo_nbytes = b1 - b0;

  if (sb == s) {
    byte *copia = NULL;
      if (novo_nbytes > 0) {
        copia = malloc(novo_nbytes);
        assert(copia != NULL);
        memcpy(copia, sb->dados + b0, novo_nbytes);
      }
    s_garante_espaco(s, novo_nbytes);
    if (novo_nbytes > 0) memcpy(s->dados, copia, novo_nbytes);
    free(copia);
  } else {
    s_garante_espaco(s, novo_nbytes);
    if (novo_nbytes > 0) memcpy(s->dados, sb->dados + b0, novo_nbytes);
  }

  s->nbytes = novo_nbytes;
  s_encolhe_se_necessario(s);
  s_ok(s);
}

void s_copia(Str s, Str_c sb)
{
  s_substring(s, sb, 0, -1);
}

void s_insere(Str s, int pos, Str_c sb)
{
  s_substitui(s, pos, 0, sb);
}

void s_insere_c(Str s, int pos, unichar c)
{
  s_ok(s);
  
  byte buf[4];
  int nb = u8_converte_pra_utf8(c, buf);
  assert(nb > 0);

  struct str tmp;
  tmp.dados = buf;
  tmp.nbytes = nb;
  tmp.alloc = MIN_ALLOC;

  s_substitui(s, pos, 0, &tmp);
}

void s_anexa(Str s, Str_c sb)
{
  s_substitui(s, -1, 0, sb);
}

void s_anexa_c(Str s, unichar c)
{
  s_insere_c(s, -1, c);
}

void s_remove(Str s, int pos, int tam)
{
  s_substitui(s, pos, tam, NULL);
}

void s_apara(Str s, Str_c sobras)
{
  s_ok(s);
  s_ok(sobras);
  
  int inicio = s_busca_nc(s, 0, sobras);

  if(inicio == -1) {
    s_substitui(s, 0, -1, NULL);
    return;
  }

  int ncar = s_tam(s);
  int fim = s_busca_rnc(s, ncar, sobras);

  s_substitui(s, fim + 1, -1, NULL);
  s_substitui(s, 0, inicio, NULL);

  s_ok(s);
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{
  s_ok(s);
  if (s->nbytes > 0) {
    fwrite(s->dados, 1, s->nbytes, stdout);
  }
}

void s_grava_arquivo(Str_c s, char *nome)
{
  s_ok(s);
  FILE *f = fopen(nome, "wb");
  assert(f != NULL);

  if (s->nbytes > 0) {
    fwrite(s->dados, 1, s->nbytes, f);
  }

  fclose(f);
}


// vim: foldmethod=marker shiftwidth=2