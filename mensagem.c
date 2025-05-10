/*
    Define a mensagem, bem como seu protocolo
    Define também funções úteis de verificação e busca de mensagem
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mensagem.h"

/* FUNÇÕES AUXILIARES INTERNAS */

// retorna valor do checksum do protocolo dado
uchar_t calcula_checksum(protocolo_t *protocolo) {
    uchar_t tamanho = protocolo->tamanho;
    uchar_t sequencia = protocolo->sequencia;
    uchar_t tipo = protocolo->tipo;
    // primeiro byte = tamanho .. sequencia[0]
    // sequencia tem 5 bits, joga fora os últimos 4 e deixa só o mais significativo
    uchar_t soma = (tamanho << 1) + (sequencia >> 4);

    // segundo byte = sequencia[1:] .. tipo
    // guarda os 4 bits menos significativos de sequenciae já os desloca pro lado
    // tipo tem os outros 4 bits
    soma += (sequencia << 4) + tipo;

    // bytes restantes = dados
    for (int i = 0; i < tamanho; i++)
        soma += protocolo->dados[i];

    return soma;
}


/* --------------------------------------- */

/*
cria_mensagem: cria mensagem a partir do protocolo
parâmetros:
    protocolo: ponteiro para protocolo a ser interpretado
retorno: ponteiro para mensagem criada (NULL se não houver mensagem)
obs: memória é alocada, deve ser liberada após uso com libera_mensagem()
*/
mensagem_t *cria_mensagem(protocolo_t *protocolo) {
    // possui marcador de início?
    if (protocolo->marcador_inicio != MARCADOR_INICIO)
        return NULL;
    // checksum deu problema?
    if (calcula_checksum(protocolo) != protocolo->checksum)
        return NULL;

    // se não houve problemas, criar mensagem
    uchar_t tamanho = protocolo->tamanho;
    mensagem_t *mensagem = malloc(sizeof(protocolo_t) + tamanho);
    mensagem->tamanho = tamanho;
    mensagem->sequencia = protocolo->sequencia;
    mensagem->tipo = protocolo->tipo;
    memcpy(mensagem->dados, protocolo->dados, tamanho);
    return mensagem;
}

/*
cria_protocolo: cria protocolo a partir de seus metadados e dados
parâmetros:
    tamanho (7 bits): quantos bytes tem dados
    sequencia (5 bits): qual o número da sequência
    tipo (4 bits): qual o tipo de mensagem
    dados (<tamanho> bytes): ponteiro para vetor de dados
retorno: ponteiro para protocolo criado
obs: memória é alocada, deve ser liberada após uso com libera_protocolo
*/
protocolo_t *cria_protocolo(uchar_t tamanho, uchar_t sequencia, uchar_t tipo, uchar_t *dados) {
    protocolo_t *protocolo = malloc(sizeof(protocolo) + tamanho);
    protocolo->marcador_inicio = MARCADOR_INICIO;
    protocolo->tamanho = tamanho;
    protocolo->sequencia = sequencia;
    protocolo->tipo = tipo;
    memcpy(protocolo->dados, dados, tamanho);
    protocolo->checksum = calcula_checksum(protocolo);
    return protocolo;
}

/*
libera_mensagem: libera memória alocada por mensagem
parâmetros:
    mensagem: ponteiro para mensagem a ser liberada
retorno: void
*/
void libera_mensagem(mensagem_t *mensagem) {
    // é bem simples na verdade
    free(mensagem);
}

/*
libera_protocolo: libera memória alocada por protocolo
parâmetros:
    protocolo: ponteiro para protocolo a ser liberado
retorno: void
*/
void libera_protocolo(protocolo_t *protocolo) {
    // é bem simples na verdade
    free(protocolo);
}