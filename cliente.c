#include "cliente.h"

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

// função apenas para enviar comando para o servidor
void envia_comando (gerenciador_t *gerenciador, int movimento){
    envia_mensagem(gerenciador, 0, movimento, (uchar_t *) 1);
}

// função que serve para receber o arquivo do servidor
void receba(const char *nome_arquivo, gerenciador_t *gerenciador) {
    // tempo de início do recebimento
    struct timeval start, end;
    long seconds, useconds;
    double total_time;
    gettimeofday(&start, NULL);


    FILE *f = fopen(nome_arquivo, "wb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return;
    }
    mensagem_t *msg_recebida = NULL;
    int resposta;

    // primeiro, receber tamanho do arquivo e ver se cabe no disco
    // também é possível receber aviso de que não tem permissão para abrir
    size_t tamanho_arq;
    do {
        msg_recebida = recebe_mensagem(gerenciador, &resposta);
        if (resposta == -1)
            continue;

        if (msg_recebida) {
            if (msg_recebida->tipo == TIPO_TAMANHO)
                tamanho_arq = *(size_t *) msg_recebida->dados;
            else if (msg_recebida->tipo == TIPO_ERRO && *(int *) msg_recebida->dados == ERRO_PERMISSAO) {
                printf("SERVIDOR AVISOU QUE NÃO TEM PERMISSÃO PARA ABRIR ARQUIVO! Continuando jogo.\n");
                return;
            }
        }
        
        // enviar ack
        if (resposta == 0) {
            envia_mensagem(gerenciador, 0, TIPO_ACK, (uchar_t *) 1);
        }
        // enviar nack
        else if (resposta == 1) {
            envia_mensagem(gerenciador, 0, TIPO_NACK, (uchar_t *) 1);
        }
    } while(!msg_recebida);

    // verificando se cabe no disco
    struct statvfs st;
        if (statvfs(".", &st) != 0) {
        perror("ERRO: não foi possível ver o espaço do disco.\n");
        return;
    }

    size_t espaco_livre = st.f_bsize * st.f_bavail;
    // se arquivo não cabe no disco, avisar e sair
    if (tamanho_arq > espaco_livre) {
        // avisando servidor
        int dados_erro = ERRO_ESPACO;
        envia_mensagem(gerenciador, sizeof(int), TIPO_ERRO, (uchar_t *) &dados_erro);
        printf("Infelizmente, o arquivo não cabe no disco :( Vamos contunuar o jogo\n");
        return;
    }

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
            imprime_progresso_envio(nome_arquivo, bytes_recebidos, tamanho_arq);
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

    // tempo de fim do recebimento
    gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    total_time = seconds + useconds/1e6;

    // resultado do tempo e taxa de transmissão
    printf("Tempo total: %f segundos\n", total_time);
    printf("Taxa média de transmissão: %.2f MB/s\n", tamanho_arq / total_time / (1024*1024));
}

// função principal do cliente 
void cliente(char *rede_recebe){
    tabuleiro_t *tabuleiro = inicializa_tabuleiro();
    gerenciador_t *gerenciador = malloc(sizeof(gerenciador_t));
    inicia_gerenciador(gerenciador, rede_recebe);

    char comando;
    mensagem_t *msg_recebida, *msg_ack;
    int erro;
    char nome_arquivo[64];

    char linha[10];

    while (tabuleiro->cont_tesouros < NUM_TESOUROS) {
        printf("TESOUROS ENCONTRADOS: %d\n", tabuleiro->cont_tesouros);
        exibe_tabuleiro(tabuleiro);
        printf("Digite comando (w/a/s/d): ");

        if (fgets(linha, sizeof(linha), stdin) != NULL) {
            comando = linha[0]; // lê o primeiro caractere digitado
        }
        while (comando != 'a' && comando != 'w' && comando != 'd' && comando != 's') {
            printf("Comando inválido. \nDigite apenas w, a, s ou d:");
            if (fgets(linha, sizeof(linha), stdin) != NULL) {
                comando = linha[0];
            }
        }
        printf("\n");
        
        // traduz o comando do input em tipo para a mensagem
        int tipo_comando = tipo_de_movimento(comando);
        
        // envia o comando para o servidor 
        envia_comando(gerenciador, tipo_comando);
        erro = espera_ack(gerenciador, &msg_ack);
        while (erro) {
            reenvia(gerenciador);
            erro = espera_ack(gerenciador, &msg_ack);
            if (erro == 1)
                libera_mensagem(msg_ack);
        }
        msg_recebida = msg_ack;
        int sucesso_nack = 0;
        while (!sucesso_nack) {
            switch (msg_recebida->tipo) {
                // movimento aceito, ou seja só andou no tabuleiro
                case TIPO_OK_ACK:
                    movimentacao(tabuleiro, comando);
                    sucesso_nack = 1;
                    break;
                // caiu num tesouro
                case TIPO_IMAGEM_ACK:
                case TIPO_VIDEO_ACK:
                case TIPO_TEXTO_ACK:
                    tabuleiro->cont_tesouros++;
                    // envia ack para o servidor dizendo que recebeu o comandod
                    
                    envia_mensagem(gerenciador, 0, TIPO_ACK, NULL);
                    // permite movimentar no tabuleiro
                    movimentacao(tabuleiro, comando);
                    exibe_tabuleiro(tabuleiro);
                    memcpy(nome_arquivo, msg_recebida->dados, msg_recebida->tamanho);
                    nome_arquivo[msg_recebida->tamanho] = '\0';
                    printf("Parabéns! Você encontrou o tesouro %s em (%d,%d)!\n", 
                        nome_arquivo, tabuleiro->pos_x, tabuleiro->pos_y);
                    receba(nome_arquivo, gerenciador);
                    abrir_arquivo(nome_arquivo);
                    sucesso_nack = 1;
                    break;
                // movimento não válido (saiu do tabuleiro, etc)
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
        libera_mensagem(msg_ack);
    }
    
    libera_tabuleiro(tabuleiro);
    libera_gerenciador(gerenciador);
}
