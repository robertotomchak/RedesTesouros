#include "cliente.h"

void abrir_arquivo(const char *arquivo){
    printf("Abrindo o arquivo %s...\n", arquivo);
    char comando[128];
    snprintf(comando, sizeof(comando), "xdg-open '%s' 2>/dev/null >/dev/null &", arquivo);
    system(comando);
}

int tipo_de_movimento (char comando){
    switch (comando) {
        case 'a':
            return TIPO_ESQUERDA;
        case 'w':
            return TIPO_CIMA;
        case 'd':
            return TIPO_DIREITA;
        case 's':
            return TIPO_BAIXO;
        default:
            printf("Comando inválido.\n");
            return TIPO_ERRO;
    }
}

void envia_comando (gerenciador_t *gerenciador, int movimento){
    envia_mensagem(gerenciador, 0, movimento, (uchar_t *) 1);
}

void receba(const char *nome_arquivo, gerenciador_t *gerenciador) {
    FILE *f = fopen(nome_arquivo, "wb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return;
    }

    // TODO: aqui não tava inicializando msg_recebida com NULL
    // talvez isso fizesse ele sair do while, mas não tenho certeza
    mensagem_t *msg_recebida = NULL;
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
        else if (resposta == 1) {
            printf("ENVIANDO NACK...\n");
            envia_mensagem(gerenciador, 0, TIPO_NACK, (uchar_t *) 1);
        }
    } while(!msg_recebida || msg_recebida->tipo != TIPO_FIM_ARQUIVO);
    
    envia_mensagem(gerenciador, 0, TIPO_ACK, (uchar_t *) 1);

    fclose(f);
}

void cliente(){
    tabuleiro_t *tabuleiro = inicializa_tabuleiro();
    gerenciador_t *gerenciador = malloc(sizeof(gerenciador_t));
    inicia_gerenciador(gerenciador, REDE_RECEBE);

    char comando;
    mensagem_t *msg_recebida, *msg_ack;
    int erro;
    char nome_arquivo[64];

    char linha[10];

    while (tabuleiro->cont_tesouros < 8) {
        exibe_tabuleiro(tabuleiro, CLIENTE);
        printf("Digite comando (w/a/s/d): ");

        if (fgets(linha, sizeof(linha), stdin) != NULL) {
            comando = linha[0]; // lê o primeiro caractere digitado
        }
        while (comando != 'a' && comando != 'w' && comando != 'd' && comando != 's') {
            printf("Comando inválido. Use apenas w, a, s ou d.\n");
            if (fgets(linha, sizeof(linha), stdin) != NULL) {
                comando = linha[0];
            }
        }

        int tipo_comando = tipo_de_movimento(comando);
        int sucesso_nack = 0;

        envia_comando(gerenciador, tipo_comando);
        erro = espera_ack(gerenciador, &msg_ack);
        while (erro) {
            reenvia(gerenciador);
            erro = espera_ack(gerenciador, &msg_ack);
        }
        msg_recebida = msg_ack;
        while (!sucesso_nack) {
            switch (msg_recebida->tipo) {
                // movimento aceito
                case TIPO_OK_ACK:
                    movimentacao(tabuleiro, comando);
                    exibe_tabuleiro(tabuleiro, CLIENTE);
                    sucesso_nack = 1;  // sai do while
                    break;
                // caiu num tesouro
                case TIPO_IMAGEM_ACK:
                case TIPO_VIDEO_ACK:
                case TIPO_TEXTO_ACK:
                    // TODO: acho que envia_mensagem tá ok. Só talvez tenha que adicionar lógica
                    // para reenviar mensagem se servidor não receber o ack
                    envia_mensagem(gerenciador, 0, TIPO_ACK, NULL);
                    while (erro) {
                        reenvia(gerenciador);
                        erro = espera_ack(gerenciador, &msg_ack);
                    }
                    movimentacao(tabuleiro, comando);
                    exibe_tabuleiro(tabuleiro, CLIENTE);
                    memcpy(nome_arquivo, msg_recebida->dados, msg_recebida->tamanho);
                    nome_arquivo[msg_recebida->tamanho] = '\0';
                    receba(nome_arquivo, gerenciador);
                    abrir_arquivo(nome_arquivo);
                    sucesso_nack = 1;
                    break;
                // TODO: esse caso pode acontecer?
                case TIPO_NACK:
                    printf("Servidor rejeitou o comando. Reenviando...\n");
                    // Reenvia até ter sucesso
                    while (reenvia(gerenciador)) {
                        printf("Erro ao reenviar. Tentando novamente...\n");
                    }
                    break;
                // movimento não válido (saiu do tabuleiro, etc)
                case TIPO_ACK:
                    printf("Movimento inválido!\n");
                    sucesso_nack = 1; 
                    break;
                case TIPO_ERRO:
                    printf("Comando invalido!\n");
                    sucesso_nack = 1; 
                    break;
                default:
                    printf("Mensagem inesperada! Tipo %d\n", msg_recebida->tipo);
                    sucesso_nack = 1;  // sai do loop em caso de erro
            }
        }

    }
    
    libera_tabuleiro(tabuleiro);
    libera_gerenciador(gerenciador);
}
