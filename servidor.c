#include "servidor.h"

// define se valor deve ser imprimido em termos de B, KB, MB ou GB
char grandeza(size_t valor) {
    if (valor < 1024)
        return ' ';
    else if (valor < 1024*1024)
        return 'K';
    else if (valor < 1024*1024*1024)
        return 'M';
    return 'G';
}

// função auxiliar para imprimir envio do arquivo bonitinho
void imprime_progresso_envio(const char *nome_arquivo, size_t atual, size_t total) {
    fflush(stdout);
    printf("\r%s: ", nome_arquivo);
    size_t atual_reduzido = atual;
    while (atual_reduzido > 1024) atual_reduzido /= 1024;
    printf("%ld%cB / ", atual_reduzido, grandeza(atual));
    size_t total_reduzido = total;
    while (total_reduzido > 1024) total_reduzido /= 1024;
    printf("%ld%cB ", total_reduzido, grandeza(total));
    double porcent = 100 * ((double) atual) / total;
    printf("(%.2f%%)", porcent);
    // uns espaço a mais só para evitar problemas no print
    printf("                          ");
}

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

    mensagem_t *msg_ack;
    int erro;

    // primeiro, enviar tamanho do arquivo
    struct stat st;
    stat(caminho_arquivo, &st);
    size_t tamanho_arq = st.st_size;
    envia_mensagem(gerenciador, sizeof(unsigned int), TIPO_TAMANHO, (uchar_t *) &tamanho_arq);
    erro = espera_ack(gerenciador, &msg_ack);
    while (erro) {
        reenvia(gerenciador);
        erro = espera_ack(gerenciador, &msg_ack);
    }
    // TODO: usar dados do ack para saber se deu tudo certo

    char buffer[BUFFER_SIZE];
    size_t bytes_lidos;
    size_t bytes_enviados = 0;
    while ((bytes_lidos = fread(buffer, 1, BUFFER_SIZE, f)) > 0) {
        envia_mensagem(gerenciador, bytes_lidos, TIPO_DADOS, (uchar_t *) buffer);
        erro = espera_ack(gerenciador, &msg_ack);
        while (erro) {
            reenvia(gerenciador);
            erro = espera_ack(gerenciador, &msg_ack);
        }
        bytes_enviados += bytes_lidos;
        imprime_progresso_envio(nome_arquivo, bytes_enviados, tamanho_arq);
    }
    // última mensagem para dizer que acabou
    envia_mensagem(gerenciador, 0, TIPO_FIM_ARQUIVO, (uchar_t *) buffer);
    erro = espera_ack(gerenciador, &msg_ack);
    while (erro) {
        reenvia(gerenciador);
        erro = espera_ack(gerenciador, &msg_ack);
    }
    fclose(f);
    printf("\r%s enviado com sucesso       \n", nome_arquivo);
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
        printf("TIPO DA MENSAGEM RECEBIDA: %d\n", msg_recebida->tipo);
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
            envia(movimento, gerenciador); // envia o arquivo

            exibe_tabuleiro(tabuleiro, SERVIDOR);
        }

    }

    libera_tabuleiro(tabuleiro);
    libera_gerenciador(gerenciador);
}