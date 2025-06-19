/* Biblioteca para funções gerais do projeto*/
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include "tipo.h"

size_t tamanho_arquivo(const char *caminho_arquivo);

// define se valor deve ser imprimido em termos de B, KB, MB ou GB
char grandeza(size_t valor);

// função auxiliar para imprimir envio do arquivo bonitinho
void imprime_progresso_envio(const char *nome_arquivo, size_t atual, size_t total);

// deixa apenas o nome do arquivo sem a extensão
char *sem_extensao(char *arquivo);

// pega apenas a extensão do arquivo
const char* obter_extensao(const char *arquivo);

// recebe um comando da mensagem (pega pelo tipo) e devolve qual é o caracter que representa
const char tipo_do_comando(int comando);

// recebe um comando do teclado (input) e retorna o tipo que ela representa na mensagem
int tipo_de_movimento (char comando);

#endif