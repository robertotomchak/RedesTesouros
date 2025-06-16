#ifndef CLIENTE_H
#define CLIENTE_H

#include "gerenciador.h"
#include "caca_tesouro.h"
#include "tipo.h"

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#define REDE_RECEBE "enx00e04c68011f"

void abrir_arquivo(const char *arquivo);

int tipo_de_movimento (char comando);

void envia_comando (gerenciador_t *gerenciador, int movimento);

void receba(const char *nome_arquivo, gerenciador_t *gerenciador);

void cliente();

#endif
