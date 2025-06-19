#include "utils.h"

size_t tamanho_arquivo(const char *caminho_arquivo) {
    struct stat st;
    if (stat(caminho_arquivo, &st) == 0) {
        return st.st_size;
    } else {
        perror("Erro ao obter tamanho do arquivo");
        return 0;
    }
}

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

// deixa apenas o nome do arquivo sem a extensão
char *sem_extensao(char *arquivo) {
    char *nome = malloc(64 * sizeof(char));
    char *ponto = strchr(arquivo, '.');

    if (ponto) {
        size_t tam = ponto - arquivo;
        strncpy(nome, arquivo, tam);
        nome[tam] = '\0';
        return nome;
    } else {
        return arquivo;
    }
}

// pega apenas a extensão do arquivo
const char* obter_extensao(const char *arquivo) {
    const char *extensao = strrchr(arquivo, '.');
    if (!extensao || extensao == arquivo) return "";
    return extensao + 1;
}

// recebe um comando da mensagem (pega pelo tipo) e devolve qual é o caracter que representa
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

// recebe um comando do teclado (input) e retorna o tipo que ela representa na mensagem
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

