/*
    Gerencia o uso da rede
    Tanto envio quando recebimento de mensagens
*/

#ifndef GERENCIADOR_H
#define GERENCIADOR_H

#include "mensagem.h"

/*
Estrutura do gerenciador
*/
typedef struct {
    int socket;  // fd do socket
    mensagem_t *ultima_enviada;
    mensagem_t *ultima_recebida;
} gerenciador_t;

// quantas sequências podem ser identificadas antes de recomeçar
// 5 bits
#define TAM_SEQUENCIA 1 << 5

// tempo do time out (em segundos)
#define TIMEOUT 60


/*
inicia_gerenciador: inicia a estrutura do gerenciador
parâmetros:
    gerenciador: ponteiro para o gerenciador
    nome_interface_rede: string com nome da rede (descoberto com ip addr)
retorno: 0 se houve sucesso; != 0 se houve erro
*/
int inicia_gerenciador(gerenciador_t *gerenciador, char *nome_interface_rede);

/*
envia_mensagem: envia uma mensagem na rede
parâmetros:
    gerenciador: ponteiro para o gerenciador
    tamanho (7 bits): quantos bytes tem dados
    tipo (4 bits): qual o tipo de mensagem
    dados (<tamanho> bytes): ponteiro para vetor de dados
retorno: 0 se houve sucesso; != 0 se houve erro
*/
int envia_mensagem(gerenciador_t *gerenciador, uchar_t tamanho, uchar_t tipo, uchar_t *dados);

/*
recebe_mensagem: recebe uma mensagem da rede
parâmetros:
    gerenciador: ponteiro para o gerenciador
    resposta: armazena o tipo de resposta a ser enviado (ack -> 0, nack -> 1, não responder -> -1)
retorno: ponteiro para mensagem recebida; NULL se não recebeu nenhuma mensagem
*/
mensagem_t *recebe_mensagem(gerenciador_t *gerenciador, int *resposta);

/*
espera_ack: espera a outra máquina avisar que recebeu (ou não) uma mensagem
parâmetros:
    gerenciador: ponteiro para o gerenciador
    tipo: armazena o tipo da mensagem recebida
retorno: 0 (ack), 1 (nack) ou -1 (timeout)
*/
int espera_ack(gerenciador_t *gerenciador, uchar_t *tipo);

/*
libera_gerenciador: libera memória alocada pelo gerenciador
parâmetros:
    gerenciador: ponteiro para o gerenciador
*/
void libera_gerenciador(gerenciador_t *gerenciador);

#endif