
#ifndef CACA_TESOURO_H
#define CACA_TESOURO_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h> 
#include <string.h>
#include "gerenciador.h"
#include "utils.h"

#define BUFFER_SIZE (1 << 7) - 1
#define NUM_TESOUROS 8
#define TAM_TABULEIRO 8

// definições para usar na função de movimentação
#define MOVIMENTO_INVALIDO "invalido"
#define MOVIMENTO_ACEITO "aceito"

typedef unsigned char uchar_t;

typedef struct {
    unsigned short posicao;         // posição de 0-63 que indica em qual quadrado está os tesouros
    char arquivo[64];               // nome do arquivo
} tesouro_t;

typedef struct {
    uchar_t pos_x;                  // pos_x e pos_y indica a posição atual do jogador
    uchar_t pos_y;                   
    char **matriz;                  // é o proprio tabuleiro, onde "." é sem tesouro e T o contrario
    char **deslocamento;            // matriz booleana, onde 0 é que caminho desconhecido e 1 o contrario
    tesouro_t tesouros[NUM_TESOUROS];    // vetor de todos os 8 tesouros
    uchar_t cont_tesouros;          // quantidade de tesouros encontrados
} tabuleiro_t;

// função que inicializa o tabuleiro
tabuleiro_t *inicializa_tabuleiro ();

// servidor usa a função para sortear posições dos tesouros no tabuleiro
void sorteia_tesouros (tabuleiro_t *tabuleiro);

// mostra o tabuleiro conforme se é servidor ou cliente
void exibe_tabuleiro(tabuleiro_t *tabuleiro);

// realiza a movimentação do jogador na tela
const char* movimentacao(tabuleiro_t *tabuleiro, const char comando);

void libera_tabuleiro(tabuleiro_t *tabuleiro);

#endif