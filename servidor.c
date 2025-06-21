#include "servidor.h"

// função que faz o envio do arquivo para o cliente
void envia(const char *nome_arquivo, gerenciador_t *gerenciador) {
    char caminho_arquivo[100];
    snprintf(caminho_arquivo, sizeof(caminho_arquivo), "objetos/%s", nome_arquivo);
    FILE *f = fopen(caminho_arquivo, "rb");

    mensagem_t *msg_ack;
    int erro;

    // se não conseguir abrir o arquivo, avisar cliente e não mandar arquivo
    if (!f) {
        printf("Infelizmente, não tenho permissão para abrir o arquivo :(\n");
        int dados_erro = ERRO_PERMISSAO;
        envia_mensagem(gerenciador, sizeof(int), TIPO_ERRO, (uchar_t *) &dados_erro);
        erro = espera_ack(gerenciador, &msg_ack);
        while (erro) {
            reenvia(gerenciador);
            erro = espera_ack(gerenciador, &msg_ack);
    }
        return;
    }

    // primeiro, enviar tamanho do arquivo
    size_t tamanho_arq = tamanho_arquivo(caminho_arquivo);
    envia_mensagem(gerenciador, sizeof(size_t), TIPO_TAMANHO, (uchar_t *) &tamanho_arq);
    erro = espera_ack(gerenciador, &msg_ack);
    while (erro) {
        reenvia(gerenciador);
        erro = espera_ack(gerenciador, &msg_ack);
    }
    // se mensagem recebida for de erro, é porque não tinha espaço suficiente
    if (msg_ack->tipo == TIPO_ERRO && * (int *) msg_ack->dados == ERRO_ESPACO) {
        printf("CLIENTE AVISOU QUE ARQUIVO NÃO CABE NO SEU DISCO! Abortar envio do arquivo.\n");
        return;
    }

    printf("Enviando arquivo %s para cliente.\n", nome_arquivo);
    char buffer[BUFFER_SIZE];
    size_t bytes_lidos;
    size_t bytes_enviados = 0;
    while ((bytes_lidos = fread(buffer, 1, BUFFER_SIZE, f)) > 0) {
        envia_mensagem(gerenciador, bytes_lidos, TIPO_DADOS, (uchar_t *) buffer);
        erro = espera_ack(gerenciador, &msg_ack);
        while (erro) {
            reenvia(gerenciador);
            erro = espera_ack(gerenciador, &msg_ack);
            if (erro == 1)
                libera_mensagem(msg_ack);
        }
        libera_mensagem(msg_ack);
        bytes_enviados += bytes_lidos;
        imprime_progresso_envio(nome_arquivo, bytes_enviados, tamanho_arq);
    }
    // última mensagem para dizer que acabou
    envia_mensagem(gerenciador, 0, TIPO_FIM_ARQUIVO, (uchar_t *) buffer);
    erro = espera_ack(gerenciador, &msg_ack);
    while (erro) {
        reenvia(gerenciador);
        erro = espera_ack(gerenciador, &msg_ack);
        if (erro == 1)
            libera_mensagem(msg_ack);
    }
    libera_mensagem(msg_ack);
    fclose(f);
    printf("\r%s enviado com sucesso       \n", nome_arquivo);
}

// função geral que comanda o lado servidor
void servidor(){
    tabuleiro_t *tabuleiro = inicializa_tabuleiro();
    sorteia_tesouros(tabuleiro);
    
    gerenciador_t *gerenciador = malloc(sizeof(gerenciador_t));
    inicia_gerenciador(gerenciador, REDE_ENVIA);

    mensagem_t *msg_recebida, *msg_ack;
    int resposta, erro;

    // laço principal do jogo
    while (tabuleiro->cont_tesouros < 8) {
        do {
            msg_recebida = recebe_mensagem(gerenciador, &resposta);
            // mensagem recebida com erro, enviar nack
            if (resposta == 1)
                envia_mensagem(gerenciador, 0, TIPO_NACK, NULL);
        } while (!msg_recebida || resposta == -1);

        // pega o tipo do comando que o cliente enviou
        const char comando = tipo_do_comando (msg_recebida->tipo);

        /* recebe o tipo de movimento com base no comando passado pelo cliente, os quais sao:
           "aceito", "invalido" ou o nome completo do arquivo*/
        const char *movimento = movimentacao(tabuleiro, comando);
        
        // movimento em que bateu na parede devolve ack dizendo que receber, mas que nao movimentou
        if (strcmp(movimento, MOVIMENTO_INVALIDO) == 0) {
            envia_mensagem(gerenciador, 0, TIPO_ACK, NULL);
            exibe_tabuleiro(tabuleiro);
            continue;
        
        // o movimento do jogador foi aceito, ou seja so vai andar e nao encotrou um tesouro
        } else if (strcmp(movimento, MOVIMENTO_ACEITO) == 0) {
            // quando permitiu a movimentação o jogador/cliente precisa saber dessa condição
            envia_mensagem(gerenciador, 0, TIPO_OK_ACK, NULL);
            exibe_tabuleiro(tabuleiro);
            continue;
        }

        // movimento resultou no encontro de um tesouro e agora com base no nome do 
        // arquivo devolvido irá devolver o tipo na mensagem para o cliente 
        else {
            int tipo_ack;
            const char *extensao = obter_extensao(movimento);
            if (strcmp(extensao, "txt") == 0) {
                tipo_ack = TIPO_TEXTO_ACK;
            } else if (strcmp(extensao, "jpg") == 0) {
                tipo_ack = TIPO_IMAGEM_ACK;
            } else if (strcmp(extensao, "mp4") == 0) {
                tipo_ack = TIPO_VIDEO_ACK;
            } else {
                tipo_ack = TIPO_ERRO;
            }

            // envia mensagem com o tipo encontrado no if o nome do arquivo como dados da mensagem 
            envia_mensagem(gerenciador, strlen(movimento) + 1, tipo_ack, (uchar_t *) movimento);
            
            // espera o ack do cliente que recebeu a mensagem
            erro = espera_ack(gerenciador, &msg_ack);
            while (erro) {
                reenvia(gerenciador);
                erro = espera_ack(gerenciador, &msg_ack);
            }

            // envia o arquivo para o cliente
            envia(movimento, gerenciador);

            exibe_tabuleiro(tabuleiro);
        }

    }

    libera_tabuleiro(tabuleiro);
    libera_gerenciador(gerenciador);
}
