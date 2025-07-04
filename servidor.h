#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "caca_tesouro.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sys/stat.h>

// função que faz o envio do arquivo para o cliente
void envia(const char *nome_arquivo, gerenciador_t *gerenciador);

// função geral que comanda o lado servidor
void servidor(char *rede_envia);

#endif
