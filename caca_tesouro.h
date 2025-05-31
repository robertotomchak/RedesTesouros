
#ifndef CACA_TESOURO_H
#define CACA_TESOURO_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h> 
#include <string.h>
#include "gerenciador.h"
#include "tipo.h"

#define BUFFER_SIZE (1 << 7) - 1
#define TAM_MAX 8
#define CLIENTE 0
#define SERVIDOR 1

#define MOVIMENTO_INVALIDO "invalido"
#define MOVIMENTO_ACEITO "aceito"

typedef unsigned char uchar_t;

typedef struct {
    unsigned short posicao;
    char arquivo[64];
} tesouro_t;

typedef struct {
    uchar_t pos_x;
    uchar_t pos_y;
    char **matriz;
    char **deslocamento;
    tesouro_t tesouros[TAM_MAX];
    uchar_t cont_tesouros;
} tabuleiro_t;

tabuleiro_t *inicializa_tabuleiro ();

void sorteia_tesouros (tabuleiro_t *tabuleiro);

void exibe_tabuleiro (tabuleiro_t *tabuleiro, int tipo);

const char* movimentacao(tabuleiro_t *tabuleiro, const char comando);

void libera_tabuleiro(tabuleiro_t *tabuleiro);

#endif