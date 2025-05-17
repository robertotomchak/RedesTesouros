/* 
    Testa o sistema com ack
    Faz isso lançando um arquivo de uma máquina para outra
*/

#include "gerenciador.h"
#include "tipo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_PATH "exemplo.txt"
#define BUFFER_SIZE (1 << 7) - 1

#define REDE_ENVIO "enx00e04c68011f"
#define REDE_RECEBE ""

void envia() {
    gerenciador_t *gerenciador = malloc(sizeof(gerenciador_t));
    inicia_gerenciador(gerenciador, REDE_ENVIO);
    FILE *f = fopen(FILE_PATH, "rb");
    char buffer[BUFFER_SIZE];
    size_t bytes_lidos;
    mensagem_t *msg_ack;
    int erro;
    while ((bytes_lidos = fread(buffer, 1, BUFFER_SIZE, f)) > 0) {
        envia_mensagem(gerenciador, bytes_lidos, TIPO_DADOS, (uchar_t *) buffer);
        erro = espera_ack(gerenciador, &msg_ack);
        while (erro) {
            reenvia(gerenciador);
            erro = espera_ack(gerenciador, &msg_ack);
        }
        printf("MENSAGEM ENVIADA COM SUCESSO\n");
    }
    // última mensagem para dizer que acabou
    envia_mensagem(gerenciador, 0, TIPO_FIM_ARQUIVO, (uchar_t *) buffer);
    erro = espera_ack(gerenciador, &msg_ack);
    while (erro) {
        reenvia(gerenciador);
        erro = espera_ack(gerenciador, &msg_ack);
    }
    fclose(f);
    printf("TERMINOU DE ENVIAR ARQUIVO\n");
    libera_gerenciador(gerenciador);
}

void receba() {
    gerenciador_t *gerenciador = malloc(sizeof(gerenciador_t));
    inicia_gerenciador(gerenciador, REDE_RECEBE);
    FILE *f = fopen(FILE_PATH, "wb");
    mensagem_t *msg_recebida;
    int resposta;
    do {
        msg_recebida = recebe_mensagem(gerenciador, &resposta);
        if (resposta == -1)
            continue;
        // processar mensagem
        if (msg_recebida && msg_recebida->tipo == TIPO_DADOS) {
            printf("ESCREVENDO %d BYTES\n", msg_recebida->tamanho);
            fwrite(msg_recebida->dados, 1, msg_recebida->tamanho, f);
        }
        // enviar ack
        if (resposta == 0) {
            printf("ENVIANDO ACK...\n");
            envia_mensagem(gerenciador, 0, TIPO_ACK, (uchar_t *) 1);
        }
        // enviar nack
        else if (resposta == -1) {
            printf("ENVIANDO NACK...\n");
            envia_mensagem(gerenciador, 0, TIPO_NACK, (uchar_t *) 1);
        }
    } while(!msg_recebida || msg_recebida->tipo != TIPO_FIM_ARQUIVO);
    fclose(f);
    libera_gerenciador(gerenciador);
}

int main(int argc, char **argv) {
    if (argc != 2)
        exit(-1);
    if (argv[1][0] == '0')
        envia();
    else
        receba();
    return 0;
}
