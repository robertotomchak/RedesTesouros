/*
    Gerencia o uso da rede
    Tanto envio quando recebimento de mensagens
*/

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <sys/time.h>
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

// usando long long pra (tentar) sobreviver ao ano 2038
// retornar o tempo atual em segundos
long long timestamp_seg() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec;
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

    return 1;
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
    if (gerenciador->ultima_enviada) {
        sequencia = (gerenciador->ultima_enviada->sequencia + 1) % TAM_SEQUENCIA;
    }
    else
        sequencia = 0;

    // cria e envia o protocolo da nova mensagem
    protocolo_t *novo_protocolo = cria_protocolo(tamanho, sequencia, tipo, dados);
    send(gerenciador->socket, novo_protocolo, PROTOCOLO_TAM_MAX, 0);

    // cria mensagem desse protocolo e salva
    int _;  // não precisamos verificar o erro da mensagem
    mensagem_t *nova_mensagem = cria_mensagem(novo_protocolo, &_);
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
    resposta: armazena o tipo de resposta a ser enviado (ack -> 0, nack -> 1, não responder -> -1)
retorno: ponteiro para mensagem recebida; NULL se não recebeu nenhuma mensagem
*/
mensagem_t *recebe_mensagem(gerenciador_t *gerenciador, int *resposta) {
    // le protocolo da rede
    uchar_t buffer[PROTOCOLO_TAM_MAX];
    recv(gerenciador->socket, buffer, PROTOCOLO_TAM_MAX, 0);
    
    // cria mensagem (se válida)
    int erro;
    mensagem_t *nova_mensagem = obtem_mensagem(buffer, &erro);
    // se mensagem não for válida, faz nada
    if (!nova_mensagem) {
        // se não tem marcador de início, não era mensagem
        if (erro == MSG_ERRO_MARCADOR_INICIO)
            *resposta = -1;
        // se era erro no checksum, enviar um nack
        else if (erro == MSG_ERRO_CHECKSUM)
            *resposta = 1;  
        return NULL;
    }

    // verifica se sequencia ta correta
    // se for a mesma sequencia da última, não processa e devolta ack
    if (gerenciador->ultima_recebida && gerenciador->ultima_recebida->sequencia == nova_mensagem->sequencia) {
        *resposta = 0;
        return NULL;
    }
    // se for primeira mensagem e sequencia = 0 ou sequencia = sequencia_anterior + 1
    uchar_t sequencia_correta;
    if (gerenciador->ultima_recebida)
        sequencia_correta = (gerenciador->ultima_recebida->sequencia + 1) % TAM_SEQUENCIA;
    else
        sequencia_correta = 0;
    if (nova_mensagem->sequencia != sequencia_correta) {
        // sequência incorreta -> enviar nack
        *resposta = 1;
        return NULL;
    }

    // libera antiga mensagem e guarda nova
    if (gerenciador->ultima_recebida)
        libera_mensagem(gerenciador->ultima_recebida);
    gerenciador->ultima_recebida = nova_mensagem;

    // mensagem recebida e será processada -> enviar ack
    *resposta = 0;
    return nova_mensagem;
}

/*
espera_ack: espera a outra máquina avisar que recebeu (ou não) uma mensagem
parâmetros:
    gerenciador: ponteiro para o gerenciador
    mensagem: ponteiro para mensagem recebida (pssagem por referência)
retorno: 0 (ack), 1 (nack) ou -1 (timeout)
*/
int espera_ack(gerenciador_t *gerenciador, mensagem_t **mensagem_ptr) {
    long long comeco = timestamp_seg();
    uchar_t buffer[PROTOCOLO_TAM_MAX];
    mensagem_t *mensagem;
    int tipo = -1;
    // setando timeout do socket
    struct timeval timeout = { .tv_sec = TIMEOUT, .tv_usec = 0};
    setsockopt(gerenciador->socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    do {
        recv(gerenciador->socket, buffer, PROTOCOLO_TAM_MAX, 0);
        int _;
        mensagem = obtem_mensagem(buffer, &_);  // não precisamos do erro aqui
        // se encontrou mensagem do tipo ack ou nack, acabou
        if (mensagem) {
            if (eh_ack(mensagem))
                tipo = 0;
            else if (eh_nack(mensagem))
                tipo = 1;
            // se não for nem um nem outro, há problema de sincronização
            else {
                libera_mensagem(mensagem);
                fprintf(stderr, "ERRO: esperava ACK ou NACK\n");
                exit(-1);
            }
        }
    } while(tipo == -1 && timestamp_seg() - comeco <= TIMEOUT);
    *mensagem_ptr = mensagem;
    return tipo; 
}

/*
reenvia: envia novamente a última mensagem enviada
parâmetros:
    gerenciador: ponteiro para o gerenciador
    retorno: 0 se houve sucesso; != 0 se houve erro
*/
int reenvia(gerenciador_t *gerenciador) {
    // se não tiver última, retorna erro
    mensagem_t *msg = gerenciador->ultima_enviada;
    if (!msg)
        return 1;
    // cria e envia o protocolo da última mensagem
    protocolo_t *novo_protocolo = cria_protocolo(msg->tamanho, msg->sequencia, msg->tipo, msg->dados);
    send(gerenciador->socket, novo_protocolo, PROTOCOLO_TAM_MAX, 0);
    
    libera_protocolo(novo_protocolo);
    return 0;
}

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
