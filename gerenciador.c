/*
    Gerencia o uso da rede
    Tanto envio quando recebimento de mensagens
*/

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>

#include "gerenciador.h"

/* FUNÇÕES WIKI TODT */
 
int cria_raw_socket(char* nome_interface_rede) {
    // Cria arquivo para o socket sem qualquer protocolo
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1);
    }
 
    int ifindex = if_nametoindex(nome_interface_rede);
 
    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;
    // Inicializa socket
    if (bind(soquete, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) {
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(-1);
    }
 
    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    // Não joga fora o que identifica como lixo: Modo promíscuo
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }
 
    return soquete;
}

/* ------------------------------------- */


/*
inicia_gerenciador: inicia a estrutura do gerenciador
parâmetros:
    gerenciador: ponteiro para o gerenciador
    nome_interface_rede: string com nome da rede (descoberto com ip addr)
retorno: 0 se houve sucesso; != 0 se houve erro
*/
int inicia_gerenciador(gerenciador_t *gerenciador, char *nome_interface_rede) {
    gerenciador->socket = cria_raw_socket(nome_interface_rede);
    gerenciador->ultima_enviada = NULL;
    gerenciador->ultima_recebida = NULL;
}

/*
envia_mensagem: envia uma mensagem na rede
parâmetros:
    gerenciador: ponteiro para o gerenciador
    tamanho (7 bits): quantos bytes tem dados
    tipo (4 bits): qual o tipo de mensagem
    dados (<tamanho> bytes): ponteiro para vetor de dados
retorno: 0 se houve sucesso; != 0 se houve erro
*/
int envia_mensagem(gerenciador_t *gerenciador, uchar_t tamanho, uchar_t tipo, uchar_t *dados) {
    // determina numero da sequencia baseado na mensagem anterior
    uchar_t sequencia;
    if (gerenciador->ultima_enviada)
        sequencia = (gerenciador->ultima_enviada->sequencia + 1) % TAM_SEQUENCIA;
    else
        sequencia = 0;

    // cria e envia o protocolo da nova mensagem
    protocolo_t *novo_protocolo = cria_protocolo(tamanho, sequencia, tipo, dados);
    send(gerenciador->socket, novo_protocolo, PROTOCOLO_TAM_MAX, 0);

    // cria mensagem desse protocolo e salva
    mensagem_t *nova_mensagem = cria_mensagem(novo_protocolo);
    libera_protocolo(novo_protocolo);
    if (gerenciador->ultima_enviada)
        libera_mensagem(gerenciador->ultima_enviada);
    gerenciador->ultima_enviada = nova_mensagem;

    return 0;
}

/*
recebe_mensagem: recebe uma mensagem da rede
parâmetros:
    gerenciador: ponteiro para o gerenciador
retorno: ponteiro para mensagem recebida; NULL se não recebeu nenhuma mensagem
*/
mensagem_t *recebe_mensagem(gerenciador_t *gerenciador) {
    // le protocolo da rede
    uchar_t buffer[PROTOCOLO_TAM_MAX];
    recv(gerenciador->socket, buffer, PROTOCOLO_TAM_MAX, 0);
    
    // cria mensagem (se válida)
    mensagem_t *nova_mensagem = obtem_mensagem(buffer);
    // se mensagem não for válida, faz nada
    if (!nova_mensagem)
        return NULL;

    // verifica se sequencia ta correta
    // se for primeira mensagem e sequencia = 0 ou sequencia = sequencia_anterior + 1
    uchar_t sequencia_correta;
    if (gerenciador->ultima_recebida)
        sequencia_correta = (gerenciador->ultima_recebida->sequencia + 1) % TAM_SEQUENCIA;
    else
        sequencia_correta = 0;
    if (nova_mensagem->sequencia != sequencia_correta)
        return NULL;

    // libera antiga mensagem e guarda nova
    if (gerenciador->ultima_recebida)
        libera_mensagem(gerenciador->ultima_recebida);
    gerenciador->ultima_recebida = nova_mensagem;

    return nova_mensagem;
}

/*
espera_ack: espera a outra máquina avisar que recebeu (ou não) uma mensagem
parâmetros:
    gerenciador: ponteiro para o gerenciador
retorno: 0 (ack), 1 (nack) ou -1 (timeout)
*/
int espera_ack(gerenciador_t *gerenciador);

/*
libera_gerenciador: libera memória alocada pelo gerenciador
parâmetros:
    gerenciador: ponteiro para o gerenciador
*/
void libera_gerenciador(gerenciador_t *gerenciador) {
    // libera mensagens (se necessário)
    if (gerenciador->ultima_enviada)
        free(gerenciador->ultima_enviada);
    if (gerenciador->ultima_recebida)
        free(gerenciador->ultima_recebida);
    free(gerenciador);
}
