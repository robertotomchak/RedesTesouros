#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "gerenciador.h"
#include "caca_tesouro.h"
#include "tipo.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sys/stat.h>

#define REDE_ENVIA "enp1s0"

const char tipo_do_comando(int comando);

void envia(const char *nome_arquivo, gerenciador_t *gerenciador);

const char* obter_extensao(const char *arquivo);

void servidor();

#endif
