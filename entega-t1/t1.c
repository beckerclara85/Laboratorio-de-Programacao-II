#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N_ESCUDOS 3
#define N_ATAQUES 10
#define N_POS (N_ESCUDOS + N_ATAQUES)
#define TIROS_ONDA 30
#define ATAQUES_ONDA 20
#define INTERVALO_INICIAL 2.0
#define FATOR_INTERVALO 0.9
#define PONTOS_TIRO_SOBRA 2
#define PONTOS_ESCUDO 10
#define N_RECORDES 3
#define ARQ_RECORDES "recordes.txt"
#define DIR_SONS "Sons"
#define ARMAS "0123456789n"
#define ESCUDO ')'
#define VAZIO ' '
#define ESC 27
#define SONS_FIM_ONDA "02468"
#define SONS_FIM_PARTIDA "N)N)N"

typedef struct timespec crono;

// struct do jogo
typedef struct {
    int pontos;                 // pontos 
    int tiros;                  // tiros 
    char arma;                  // tipo de arma
    char campo[N_POS + 1];      // escudos + ataques 
    int pendentes;              // ataques ainda nao ativados na onda
    int onda;                   // numero da onda atual
    double intervalo;           // segundos entre movimentos
    crono relogio;              // marca o ultimo movimento
    int recordes[N_RECORDES];   // maiores pontuacoes, em ordem
} estado;

// configura o terminal para o modo "cru"
void configura_terminal()
{
    if (system("stty raw opost -echo min 0 time 1") != 0) {
        perror("erro na execução de system(\"stty\")");
        fprintf(stderr, "você tem o programa stty instalado?\n");
        exit(1);
    }
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
        perror("erro na execução de setvbuf()");
        exit(1);
    }
}

// configura o terminal para o modo normal
void normaliza_terminal()
{
    system("stty sane");
}

// le um caractere do teclado.
char lechar()
{
    fflush(stdout);
    char c;
    if (fread(&c, 1, 1, stdin) == 1) return c;
    return 0;
}

// inicializa um cronometro com a hora atual
void crono_inicia(crono *c)
{
    clock_gettime(CLOCK_MONOTONIC, c);
}

// retorna o tempo passado desde que o cronometro *c foi iniciado,
//   em segundos
double crono_parcial(crono *c)
{
    crono agora;
    clock_gettime(CLOCK_MONOTONIC, &agora);
    double segundos = agora.tv_sec - c->tv_sec;
    double nanosegundos = agora.tv_nsec - c->tv_nsec;
    return segundos + 1e-9 * nanosegundos;
}

// retorna 1 se o caractere c representa um ataque inimigo, 0 se nao
int eh_ataque(char c)
{
    return (c >= '0' && c <= '9') || c == 'N' || c == 'n';
}

// coloca em nome o nome do arquivo de som (sem diretorio e extensao)
//   correspondente ao tipo (ataque, arma, escudo ou espaco)
void nome_som(char tipo, char nome[])
{
    if (tipo >= '0' && tipo <= '9') {
        sprintf(nome, "%c", tipo);
        return;
    }
    if (tipo == 'N' || tipo == 'n') {
        strcpy(nome, "11");
        return;
    }
    if (tipo == ESCUDO) {
        strcpy(nome, "12");
        return;
    }
    strcpy(nome, "x");
}

// toca em sequencia (sem esperar terminar) os sons correspondentes
//   a cada caractere da string tipos
void toca_sons(char tipos[])
{
    char cmd[600] = "aplay -q";
    char nome[4];
    char arq[40];
    for (int i = 0; tipos[i] != '\0'; i++) {
        nome_som(tipos[i], nome);
        sprintf(arq, " %s/%s.3.wav", DIR_SONS, nome);
        strcat(cmd, arq);
    }
    strcat(cmd, " >/dev/null 2>&1 &");
    system(cmd);
}

// toca o som correspondente a um unico tipo
void toca_som(char tipo)
{
    char s[2];
    s[0] = tipo;
    s[1] = '\0';
    toca_sons(s);
}

// sorteia o tipo de um novo ataque: '0' a '9' ou 'N'
char sorteia_ataque()
{
    int n = rand() % 11;
    if (n == 10) return 'N';
    return '0' + n;
}

// inicializa o estado para o inicio de uma nova partida
void inicia_partida(estado *e)
{
    e->pontos = 0;
    e->onda = 0;
    e->arma = '0';
    e->intervalo = INTERVALO_INICIAL;
    for (int i = 0; i < N_POS; i++) {
        e->campo[i] = (i < N_ESCUDOS) ? ESCUDO : VAZIO;
    }
    e->campo[N_POS] = '\0';
}

// inicializa o estado para o inicio de uma nova onda: recarrega tiros
//   e ataques, acelera os ataques (a partir da 2a onda) e zera o tempo
void inicia_onda(estado *e)
{
    e->onda++;
    if (e->onda > 1) e->intervalo *= FATOR_INTERVALO;
    e->tiros = TIROS_ONDA;
    e->pendentes = ATAQUES_ONDA;
    for (int i = N_ESCUDOS; i < N_POS; i++) {
        e->campo[i] = VAZIO;
    }
    crono_inicia(&e->relogio);
}

// desenha a linha com o estado do jogo e volta ao inicio da linha
void desenha_tela(estado *e)
{
    printf(" %d %d %c%s  \r", e->pontos, e->tiros, e->arma, e->campo);
}

// move todos os ataques ativos uma posicao para a esquerda.
// ataque que encosta em escudo destroi o escudo e e destruido.
// retorna 1 se algum ataque atingiu a base (fim da partida), 0 se nao
int move_ataques(estado *e)
{
    char *c = e->campo;
    if (eh_ataque(c[0])) return 1;
    for (int i = 0; i < N_POS - 1; i++) {
        if (eh_ataque(c[i + 1])) {
            if (c[i] == ESCUDO) {
                c[i] = VAZIO;
            } else {
                c[i] = c[i + 1];
            }
            c[i + 1] = VAZIO;
        }
    }
    return 0;
}

// se ainda houver ataque nao ativado na onda, ativa um novo ataque
//   na posicao mais a direita e toca o seu som
void ativa_ataque(estado *e)
{
    if (e->pendentes == 0) return;
    char tipo = sorteia_ataque();
    e->campo[N_POS - 1] = tipo;
    e->pendentes--;
    toca_som(tipo);
}

// trata a passagem do tempo: se ja passou o intervalo, move os
//   ataques e ativa um novo.
// retorna 1 se a base foi atingida, 0 se nao
int passa_tempo(estado *e)
{
    if (crono_parcial(&e->relogio) < e->intervalo) return 0;
    crono_inicia(&e->relogio);
    if (move_ataques(e)) return 1;
    ativa_ataque(e);
    return 0;
}

// retorna 1 se a onda terminou (nao ha ataques pendentes nem ativos)
int onda_terminou(estado *e)
{
    if (e->pendentes > 0) return 0;
    for (int i = 0; i < N_POS; i++) {
        if (eh_ataque(e->campo[i])) return 0;
    }
    return 1;
}

// troca a arma selecionada pela proxima da sequencia e toca seu som
void proxima_arma(estado *e)
{
    char *armas = ARMAS;
    int i = strchr(armas, e->arma) - armas;
    e->arma = armas[(i + 1) % strlen(armas)];
    toca_som(e->arma);
}

// retorna a posicao do ataque mais a esquerda que pode ser atingido
//   pela arma selecionada, ou -1 se nao houver nenhum
int alvo(estado *e)
{
    for (int i = 0; i < N_POS; i++) {
        char c = e->campo[i];
        if (c == e->arma) return i;
        if (e->arma == 'n' && c == 'N') return i;
    }
    return -1;
}

// destroi o ataque na posicao pos e soma os pontos correspondentes:
//   1 na posicao de nascimento, +1 por posicao andada, dobro se 'n'
void destroi_ataque(estado *e, int pos)
{
    int pontos = N_POS - pos;
    if (e->campo[pos] == 'n') pontos *= 2;
    e->pontos += pontos;
    e->campo[pos] = VAZIO;
}

// executa um tiro com a arma selecionada, se houver tiro disponivel.
// um 'N' atingido vira 'n'; os outros ataques sao destruidos.
void atira(estado *e)
{
    if (e->tiros == 0) {
        toca_som(VAZIO);
        return;
    }
    e->tiros--;
    int pos = alvo(e);
    if (pos < 0) {
        toca_som(VAZIO);
        return;
    }
    toca_som(e->arma);
    if (e->campo[pos] == 'N') {
        e->campo[pos] = 'n';
    } else {
        destroi_ataque(e, pos);
    }
}

// processa uma tecla digitada pelo jogador.
// retorna 1 se o jogador desistiu (Esc), 0 se nao
int processa_tecla(estado *e, char c)
{
    switch (c) {
    case ESC:
        return 1;
    case '\t':
        proxima_arma(e);
        break;
    case '\r':
    case '\n':
        atira(e);
        break;
    case ' ':
        toca_sons(e->campo);
        break;
    }
    return 0;
}

// executa uma onda: desenha a tela e processa teclado e tempo
//   ate a onda terminar.
// retorna 1 se a partida acabou (base atingida ou Esc), 0 se nao
int executa_onda(estado *e)
{
    for (;;) {
        desenha_tela(e);
        char c = lechar();
        if (c != 0 && processa_tecla(e, c)) return 1;
        if (passa_tempo(e)) return 1;
        if (onda_terminou(e)) return 0;
    }
}

// espera ate que seja digitado um dos caracteres de validas
//   e retorna o caractere digitado
char espera_tecla(char validas[])
{
    char c = 0;
    while (c == 0 || strchr(validas, c) == NULL) {
        c = lechar();
    }
    return c;
}

// retorna o numero de escudos ainda intactos
int conta_escudos(estado *e)
{
    int n = 0;
    for (int i = 0; i < N_ESCUDOS; i++) {
        if (e->campo[i] == ESCUDO) n++;
    }
    return n;
}

// encerra uma onda: soma os pontos de tiros e escudos que sobraram,
//   mostra um resumo e espera o 'r' para continuar.
// retorna 1 se o jogador desistiu (Esc), 0 se nao
int fecha_onda(estado *e)
{
    int escudos = conta_escudos(e);
    e->pontos += PONTOS_TIRO_SOBRA * e->tiros;
    e->pontos += PONTOS_ESCUDO * escudos;
    desenha_tela(e);
    toca_sons(SONS_FIM_ONDA);
    printf("\n*** Fim da onda %d ***\n", e->onda);
    printf("Pontos: %d  Tiros sobrando: %d  Escudos: %d\n",
           e->pontos, e->tiros, escudos);
    printf("Tecle r para a proxima onda (Esc para desistir)\n");
    char c = espera_tecla("rR\033");
    return c == ESC;
}

// executa uma partida completa, onda apos onda, ate ela terminar
void joga_partida(estado *e)
{
    int acabou = 0;
    inicia_partida(e);
    while (!acabou) {
        inicia_onda(e);
        acabou = executa_onda(e);
        if (!acabou) {
            acabou = fecha_onda(e);
        }
    }
}

// le os recordes do arquivo; se nao conseguir, considera todos zero
void le_recordes(int r[])
{
    for (int i = 0; i < N_RECORDES; i++) {
        r[i] = 0;
    }
    FILE *f = fopen(ARQ_RECORDES, "r");
    if (f == NULL) return;
    for (int i = 0; i < N_RECORDES; i++) {
        if (fscanf(f, "%d", &r[i]) != 1) r[i] = 0;
    }
    fclose(f);
}

// grava os recordes no arquivo; se nao conseguir, nao faz nada
void grava_recordes(int r[])
{
    FILE *f = fopen(ARQ_RECORDES, "w");
    if (f == NULL) return;
    for (int i = 0; i < N_RECORDES; i++) {
        fprintf(f, "%d\n", r[i]);
    }
    fclose(f);
}

// insere pontos na lista de recordes (em ordem decrescente), se couber.
// retorna a posicao (0 a N_RECORDES-1) ou -1 se nao entrou
int insere_recorde(int r[], int pontos)
{
    int pos = N_RECORDES;
    while (pos > 0 && pontos > r[pos - 1]) {
        pos--;
    }
    if (pos == N_RECORDES) return -1;
    for (int i = N_RECORDES - 1; i > pos; i--) {
        r[i] = r[i - 1];
    }
    r[pos] = pontos;
    return pos;
}

// mostra se a pontuacao entrou nos recordes e a lista de recordes
void mostra_recordes(int r[], int pos)
{
    if (pos >= 0) {
        printf("Parabens! Voce ficou em %do lugar nos recordes!\n",
               pos + 1);
    } else {
        printf("Sua pontuacao nao ficou entre as %d maiores.\n",
               N_RECORDES);
    }
    printf("Recordes:");
    for (int i = 0; i < N_RECORDES; i++) {
        printf(" %d", r[i]);
    }
    printf("\n");
}

// mostra o resumo do final da partida e atualiza os recordes
void encerra_partida(estado *e)
{
    desenha_tela(e);
    toca_sons(SONS_FIM_PARTIDA);
    printf("\n\n===== FIM DA PARTIDA =====\n");
    printf("Ondas: %d  Pontuacao final: %d\n", e->onda, e->pontos);
    int pos = insere_recorde(e->recordes, e->pontos);
    mostra_recordes(e->recordes, pos);
    grava_recordes(e->recordes);
}

// pergunta se o jogador quer jogar novamente.
// retorna 1 para sim, 0 para nao
int quer_jogar_novamente()
{
    printf("Jogar novamente? (s/n)\n");
    char c = espera_tecla("sSnN\033");
    return c == 's' || c == 'S';
}

// laco principal: executa partidas enquanto o jogador quiser
int main()
{
    estado e;
    int de_novo = 1;
    srand(time(NULL));
    le_recordes(e.recordes);
    configura_terminal();
    while (de_novo) {
        joga_partida(&e);
        encerra_partida(&e);
        de_novo = quer_jogar_novamente();
    }
    normaliza_terminal();
    return 0;
}