#include "caca_tesouro.h"

tabuleiro_t *inicializa_tabuleiro (){
    tabuleiro_t *tabuleiro = malloc(sizeof(tabuleiro_t));
    tabuleiro->matriz = malloc(TAM_MAX * sizeof(char*));
    tabuleiro->deslocamento = malloc(TAM_MAX * sizeof(char*));

    for (int i = 0; i < TAM_MAX; i++) {
        tabuleiro->matriz[i] = malloc(TAM_MAX * sizeof(char));
        tabuleiro->deslocamento[i] = malloc(TAM_MAX * sizeof(char));
    }
    
    for (int i = 0; i < TAM_MAX; i++) {
        for (int j = 0; j < TAM_MAX; j++) {
            tabuleiro->matriz[i][j] = '.';
            tabuleiro->deslocamento[i][j] = 0;
        }
    }

    tabuleiro->pos_x = 0;
    tabuleiro->pos_y = 0;
    memset(tabuleiro->tesouros, 0, sizeof(tabuleiro->tesouros));
    tabuleiro->cont_tesouros = 0;

    return tabuleiro;
}

void sorteia_tesouros (tabuleiro_t *tabuleiro){
    DIR *dir;
    struct dirent *entrada;

    dir = opendir("objetos/");
    if ( !dir )
   {
      perror ("Não foi possível abrir o diretório.\n");
      exit (1) ;
   }
   
   // armazena todos os arquivos em um vetor
    char arquivos[TAM_MAX][64];
    int total_arquivos = 0;

    while ((entrada = readdir(dir)) != NULL) {
        if (entrada->d_type == DT_REG && total_arquivos < TAM_MAX) {
            strcpy(arquivos[total_arquivos], entrada->d_name);
            total_arquivos++;
        }
    }

   // sortear com srand no tempo atual
   int lin, col;
   srand(time(NULL));
   int i = 0;
    while (i < TAM_MAX){
        lin = rand() % TAM_MAX;
        col = rand() % TAM_MAX;
        if (tabuleiro->matriz[lin][col] != 'T' && !(lin == 0 && col == 0)) {
            tabuleiro->matriz[lin][col] = 'T';
            tabuleiro->tesouros[i].posicao = lin * TAM_MAX + col;
            // garante que o espaço esteja se lixo de memória
            memset(tabuleiro->tesouros[i].arquivo, 0, sizeof(tabuleiro->tesouros[i].arquivo));
            strcpy(tabuleiro->tesouros[i].arquivo, arquivos[i]);
            printf("%s\n",tabuleiro->tesouros[i].arquivo);
            i++;
        }
    }

    exibe_tabuleiro(tabuleiro, SERVIDOR);

    closedir(dir);
}

void exibe_tabuleiro (tabuleiro_t *tabuleiro, int tipo){
    printf("   ");
    for (int i = 0; i < TAM_MAX; i++) {
        printf("%d ", i);
    }
    printf("\n");

    for (int i = TAM_MAX - 1; i >= 0; i--) {
        printf("%d  ", i);
        for (int j = 0; j < TAM_MAX; j++) {
            if(i == tabuleiro->pos_y && j == tabuleiro->pos_x)
                printf("J ");
            else if (tabuleiro->deslocamento[i][j])
                printf("* ");
            else if (tabuleiro->matriz[i][j] == 'T')
                if (tipo == SERVIDOR)
                    printf("T ");
                else
                    printf(". "); 
            else
                printf("%c ", tabuleiro->matriz[i][j]);
        }
        printf("\n");
    }
}

const char* movimentacao(tabuleiro_t *tabuleiro, const char comando) {
    int x = tabuleiro->pos_x;
    int y = tabuleiro->pos_y;

    if ((comando == 'a') && (x > 0)) x--;         // esquerda
    else if ((comando == 'w') && (y < 7)) y++;    // cima
    else if ((comando == 'd') && (x < 7)) x++;    // direita
    else if ((comando == 's') && (y > 0)) y--;    // baixo
    else {
        printf("Movimento inválido, bateu na parede.\n");
        return MOVIMENTO_INVALIDO;
    }

    // Marca deslocamento na posição anterior
    tabuleiro->deslocamento[tabuleiro->pos_y][tabuleiro->pos_x] = 1;

    // Atualiza posição
    tabuleiro->pos_x = x;
    tabuleiro->pos_y = y;

    // Verifica se encontrou um tesouro e se ja nao foi visitado
    if (tabuleiro->matriz[y][x] == 'T' && tabuleiro->deslocamento[y][x] == 0) {
        tabuleiro->cont_tesouros++;

        for (int i = 0; i < TAM_MAX; i++) {
            if (tabuleiro->tesouros[i].posicao == y * TAM_MAX + x) {
                return tabuleiro->tesouros[i].arquivo;
            }
        }
    }
    return MOVIMENTO_ACEITO;
}

void libera_tabuleiro(tabuleiro_t *tabuleiro) {
    for (int i = 0; i < TAM_MAX; i++) {
        free(tabuleiro->matriz[i]);
    }
    free(tabuleiro->matriz);
    free(tabuleiro);
}