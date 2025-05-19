
#ifndef CACA_TESOURO_H
#define CACA_TESOURO_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h> 
#include <string.h>

#define TAM_MAX 8

typedef struct {
    unsigned short posicao;
    char arquivo[64];
} tesouro_t;

char **inicializa_tabuleiro ();

void sorteia_tesouros (char **matriz, tesouro_t *tesouros);

void exibe_tabuleiro (char **matriz, int pos_x, int pos_y);

void abrir_arquivo(char *arquivo);

void movimentacao (char **matriz, char comando, int *pos_x, int *pos_y, unsigned short *cont_tesouros, tesouro_t *tesouros);

#endif