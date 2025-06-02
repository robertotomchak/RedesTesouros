#include "servidor.h"

const char tipo_do_comando(int comando){
    switch (comando) {
        case TIPO_ESQUERDA:
            return 'a';
        case TIPO_CIMA:
            return 'w';
        case TIPO_DIREITA:
            return 'd';
        case TIPO_BAIXO:
            return 's';
        default:
            printf("Comando inválido.\n");
            return '\0';
    }
}

void envia(const char *nome_arquivo, gerenciador_t *gerenciador) {
    char caminho_arquivo[100];
    snprintf(caminho_arquivo, sizeof(caminho_arquivo), "objetos/%s", nome_arquivo);
    FILE *f = fopen(caminho_arquivo, "rb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return;
    }

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
    // TODO: talvez aqui precise do while (erro) ...
    // pra saber que cliente recebeu o FIM_ARQUIVO
    envia_mensagem(gerenciador, 0, TIPO_FIM_ARQUIVO, (uchar_t *) buffer);
    erro = espera_ack(gerenciador, &msg_ack);
    while (erro) {
        reenvia(gerenciador);
        erro = espera_ack(gerenciador, &msg_ack);
    }
    fclose(f);
    printf("TERMINOU DE ENVIAR ARQUIVO\n");
}

const char* obter_extensao(const char *arquivo) {
    const char *extensao = strrchr(arquivo, '.');
    if (!extensao || extensao == arquivo) return "";
    return extensao + 1;
}

void servidor(){
    tabuleiro_t *tabuleiro = inicializa_tabuleiro();
    sorteia_tesouros(tabuleiro);
    
    gerenciador_t *gerenciador = malloc(sizeof(gerenciador_t));
    inicia_gerenciador(gerenciador, REDE_ENVIA);

    mensagem_t *msg_recebida, *msg_ack;
    int resposta, erro;

    while (tabuleiro->cont_tesouros < 8) {
        do {
            msg_recebida = recebe_mensagem(gerenciador, &resposta);
        } while (!msg_recebida || resposta == -1);

        // Pega o tipo do comando
        const char comando = tipo_do_comando (msg_recebida->tipo);
        const char *movimento = movimentacao(tabuleiro, comando);
        
        if (strcmp(movimento, MOVIMENTO_INVALIDO) == 0) {
            envia_mensagem(gerenciador, 0, TIPO_ACK, NULL);
            exibe_tabuleiro(tabuleiro, SERVIDOR);
            continue;
        } else if (strcmp(movimento, MOVIMENTO_ACEITO) == 0) {
            // quando permitiu a movimentação o jogador/cliente precisa saber dessa condição
            envia_mensagem(gerenciador, 0, TIPO_OK_ACK, NULL);
            exibe_tabuleiro(tabuleiro, SERVIDOR);
            continue;
        }
        else {
            int tipo_ack;
            if (strcmp(obter_extensao(movimento), "txt") == 0) {
                printf("%s\n",obter_extensao(movimento));
                tipo_ack = TIPO_TEXTO_ACK;
            } else if (strcmp(obter_extensao(movimento), "jpg") == 0) {
                printf("%s\n",obter_extensao(movimento));
                tipo_ack = TIPO_IMAGEM_ACK;
            } else if (strcmp(obter_extensao(movimento), "mp4") == 0) {
                tipo_ack = TIPO_VIDEO_ACK;
            } else {
                tipo_ack = TIPO_ERRO;
            }

            envia_mensagem(gerenciador, strlen(movimento) + 1, tipo_ack, (uchar_t *) movimento);
            erro = espera_ack(gerenciador, &msg_ack);
            while (erro) {
                reenvia(gerenciador);
                erro = espera_ack(gerenciador, &msg_ack);
            }
            printf("MENSAGEM ENVIADA COM SUCESSO DO TIPO DE ARQUIVO\n");
            envia(movimento, gerenciador); // envia o arquivo

            exibe_tabuleiro(tabuleiro, SERVIDOR);
        }

    }

    libera_tabuleiro(tabuleiro);
    libera_gerenciador(gerenciador);
}