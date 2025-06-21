#ifndef CLIENTE_H
#define CLIENTE_H

#include "caca_tesouro.h"
#include "tipo.h"

#include <sys/types.h>
#include <sys/time.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/statvfs.h>

void abrir_arquivo(const char *arquivo);

// função apenas para enviar comando para o servidor
void envia_comando (gerenciador_t *gerenciador, int movimento);

// função que serve para receber o arquivo do servidor
void receba(const char *nome_arquivo, gerenciador_t *gerenciador);

// função principal do cliente 
void cliente(char *rede_recebe);

#endif
