/*
    Define a mensagem, bem como seu protocolo
    Define também funções úteis de verificação e busca de mensagem
*/
#ifndef MENSAGEM_H
#define MENSAGEM_H 

typedef unsigned char uchar_t;

/*
Estrutura do protocolo
Permite ler/escrever uma mensagem na rede
*/
typedef struct {
    uchar_t marcador_inicio: 8;
    uchar_t tamanho: 7;
    uchar_t sequencia: 5;
    uchar_t tipo: 4;
    uchar_t checksum: 8;
    uchar_t dados[];  // mensagem de tamanho variável
} __attribute__((packed)) protocolo_t;

/*
Estrutura da mensagem de fato
Guarda apenas os dados e alguns metados úteis
Tira informações de checagem do protocolo
*/
typedef struct {
    uchar_t tamanho;
    uchar_t sequencia;  // necessário para outras verificações
    uchar_t tipo;
    uchar_t dados[];  // sizeof(dados) = tamanho
} mensagem_t;

// metadados + dados de 2^7 bytes;
#define PROTOCOLO_TAM_MAX sizeof(protocolo_t) + (1 << 7)

// marcador de início de mensagem
#define MARCADOR_INICIO 0b01111110


/*
cria_mensagem: cria mensagem a partir do protocolo
parâmetros:
    protocolo: ponteiro para protocolo a ser interpretado
retorno: ponteiro para mensagem criada (NULL se não houver mensagem)
obs: memória é alocada, deve ser liberada após uso com libera_mensagem()
*/
mensagem_t *cria_mensagem(protocolo_t *protocolo);

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
protocolo_t *cria_protocolo(uchar_t tamanho, uchar_t sequencia, uchar_t tipo, uchar_t *dados);

/*
obtem_mensagem: cria mensagem a partir de um buffer
parâmetros:
    buffer: vetor de bytes (deve ter tamanho PROTOCOLO_TAM_MAX) que contém a mensagem
retorno: ponteiro para mensagem criada
obs: memória é alocada, deve ser liberada após uso com libera_mensagem (NULL se não houver mensagem)
*/
mensagem_t *obtem_mensagem(uchar_t *buffer);

/*
libera_mensagem: libera memória alocada por mensagem
parâmetros:
    mensagem: ponteiro para mensagem a ser liberada
retorno: void
*/
void libera_mensagem(mensagem_t *mensagem);

/*
libera_protocolo: libera memória alocada por protocolo
parâmetros:
    protocolo: ponteiro para protocolo a ser liberado
retorno: void
*/
void libera_protocolo(protocolo_t *protocolo);

#endif