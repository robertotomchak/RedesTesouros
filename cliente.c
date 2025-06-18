#include "cliente.h"

// define se valor deve ser imprimido em termos de B, KB, MB ou GB
char grandeza_(size_t valor) {
    if (valor < 1024)
        return ' ';
    else if (valor < 1024*1024)
        return 'K';
    else if (valor < 1024*1024*1024)
        return 'M';
    return 'G';
}

// função auxiliar para imprimir envio do arquivo bonitinho
void imprime_progresso_envio_(const char *nome_arquivo, size_t atual, size_t total) {
    fflush(stdout);
    printf("\r%s: ", nome_arquivo);
    size_t atual_reduzido = atual;
    while (atual_reduzido > 1024) atual_reduzido /= 1024;
    printf("%ld%cB / ", atual_reduzido, grandeza_(atual));
    size_t total_reduzido = total;
    while (total_reduzido > 1024) total_reduzido /= 1024;
    printf("%ld%cB ", total_reduzido, grandeza_(total));
    double porcent = 100 * ((double) atual) / total;
    printf("(%.2f%%)", porcent);
    // uns espaço a mais só para evitar problemas no print
    printf("                          ");
}

void abrir_arquivo(const char *arquivo){
    // descobrindo quem foi o usuário que executou o código em modo sudo
    const char *usuario = getenv("SUDO_USER");
    if (!usuario) {
        fprintf(stderr, "Erro: SUDO_USER não encontrado. O programa deve ser executado com sudo.\n");
        return;
    }
    struct passwd *pw = getpwnam(usuario);
    if (!pw) {
        fprintf(stderr, "Erro: Não foi possível obter informações do usuário %s.\n", usuario);
        return;
    }

    // Mudar o dono do arquivo
    if (chown(arquivo, pw->pw_uid, pw->pw_gid) != 0) {
        perror("Erro ao mudar o dono do arquivo");
    }

    // Capturar variáveis de ambiente da sessão do usuário
    const char *display = getenv("DISPLAY");
    const char *dbus = getenv("DBUS_SESSION_BUS_ADDRESS");
    const char *runtime = getenv("XDG_RUNTIME_DIR");

    if (!display || !dbus || !runtime) {
        fprintf(stderr, "Erro: Ambiente gráfico do usuário não encontrado corretamente (DISPLAY, DBUS ou XDG_RUNTIME_DIR).\n");
        return;
    }

    // executa o comando para abrir o arquivo, com as permissões necessárias
    char comando[512];
    snprintf(comando, sizeof(comando),
        "sudo -u %s DISPLAY='%s' DBUS_SESSION_BUS_ADDRESS='%s' XDG_RUNTIME_DIR='%s' xdg-open '%s' >/dev/null 2>&1 &",
        usuario, display, dbus, runtime, arquivo);
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
    mensagem_t *msg_recebida = NULL;
    int resposta;

    // primeiro, receber tamanho do arquivo e ver se cabe no disco
    size_t tamanho_arq;
    do {
        msg_recebida = recebe_mensagem(gerenciador, &resposta);
        if (resposta == -1)
            continue;

        if (msg_recebida && msg_recebida->tipo == TIPO_TAMANHO)
            tamanho_arq = *(size_t *) msg_recebida->dados;
        
        // enviar ack
        if (resposta == 0) {
            envia_mensagem(gerenciador, 0, TIPO_ACK, (uchar_t *) 1);
        }
        // enviar nack
        else if (resposta == 1) {
            envia_mensagem(gerenciador, 0, TIPO_NACK, (uchar_t *) 1);
        }
    } while(!msg_recebida);


    // agora, receber arquivo aos poucos
    // até receber o fim de arquivo
    size_t bytes_recebidos = 0;
    do {
        msg_recebida = recebe_mensagem(gerenciador, &resposta);
        if (resposta == -1)
            continue;
        // processar mensagem
        if (msg_recebida && msg_recebida->tipo == TIPO_DADOS) {
            bytes_recebidos += msg_recebida->tamanho;
            fwrite(msg_recebida->dados, 1, msg_recebida->tamanho, f);
            imprime_progresso_envio_(nome_arquivo, bytes_recebidos, tamanho_arq);
        }
        // enviar ack
        if (resposta == 0) {
            envia_mensagem(gerenciador, 0, TIPO_ACK, (uchar_t *) 1);
        }
        // enviar nack
        else if (resposta == 1) {
            envia_mensagem(gerenciador, 0, TIPO_NACK, (uchar_t *) 1);
        }
    } while(!msg_recebida || msg_recebida->tipo != TIPO_FIM_ARQUIVO);

    printf("\n");
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
        printf("TESOUROS ENCONTRADOS: %d\n", tabuleiro->cont_tesouros);
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
                    tabuleiro->cont_tesouros++;
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
