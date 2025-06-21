#include "caca_tesouro.h"

// função que inicializa o tabuleiro
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

// servidor usa a função para sortear posições dos tesouros no tabuleiro
void sorteia_tesouros (tabuleiro_t *tabuleiro){
    DIR *dir;
    struct dirent *entrada;

    // abre o diretorio onde estão os arquivos
    dir = opendir("objetos/");
    if ( !dir )
   {
      perror ("Não foi possível abrir o diretório.\n");
      exit (1) ;
   }
   
   // armazena todos os nomes dos arquivos em um vetor
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
   printf("Tesouros:\n");
   int i = 0;
    while (i < TAM_MAX){
        lin = rand() % TAM_MAX;
        col = rand() % TAM_MAX;

        // confere se no lugar sorteado ja foi encontrado um tesouro ou se eh diferente da posição (0,0)
        if (tabuleiro->matriz[lin][col] != 'T' && !(lin == 0 && col == 0)) {
            tabuleiro->matriz[lin][col] = 'T';

            // calcula a posição considerando que existe 64 (0-63)
            tabuleiro->tesouros[i].posicao = lin * TAM_MAX + col;

            // garante que o espaço esteja se lixo de memória
            memset(tabuleiro->tesouros[i].arquivo, 0, sizeof(tabuleiro->tesouros[i].arquivo));
            strcpy(tabuleiro->tesouros[i].arquivo, arquivos[i]);
            
            char caminho[128];
            snprintf(caminho, sizeof(caminho), "objetos/%s", tabuleiro->tesouros[i].arquivo);
            
            // Reduz o calculo do tamanho do arquivo
            size_t tamanho_arq = tamanho_arquivo(caminho);
            size_t reduzido = tamanho_arq;
            while (reduzido >= 1024) reduzido /= 1024;
            
            // print para informações dos tesouros, a posição, nome e tamanho
            printf(" • (%d,%d) -> %s: %ld%c\n", 
                        lin, col, tabuleiro->tesouros[i].arquivo,
                        reduzido, grandeza(tamanho_arq));
            i++;
        }
    }

    exibe_tabuleiro(tabuleiro);

    closedir(dir);
}

// mostra o tabuleiro conforme se é servidor ou cliente
void exibe_tabuleiro(tabuleiro_t *tabuleiro){
    printf("   ");
    for (int i = 0; i < TAM_MAX; i++) {
        printf("%d ", i);
    }
    printf("\n");

    for (int i = TAM_MAX - 1; i >= 0; i--) {
        printf("%d  ", i);
        for (int j = 0; j < TAM_MAX; j++) {
            // confere se é a mesma posição do jogador
            if(i == tabuleiro->pos_y && j == tabuleiro->pos_x)
                printf("J ");

            // confere se ja foi passado no local 
            else if (tabuleiro->deslocamento[i][j])
                printf("* ");

            // vê se é tesouro e então encontra o seu respectivo tesouro
            else if ((tabuleiro->matriz[i][j] == 'T')){
                for (int k = 0; k < TAM_MAX; k++) {
                    if (tabuleiro->tesouros[k].posicao == i * TAM_MAX + j) {
                        printf("%s ", sem_extensao(tabuleiro->tesouros[k].arquivo));
                    }
                }
            }
            else
                printf("%c ", tabuleiro->matriz[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// realiza a movimentação do jogador na tela
const char* movimentacao(tabuleiro_t *tabuleiro, const char comando) {
    
    // pega a posição atual do jogador
    int x = tabuleiro->pos_x;
    int y = tabuleiro->pos_y;

    if ((comando == 'a') && (x > 0)) x--;         // esquerda
    else if ((comando == 'w') && (y < 7)) y++;    // cima
    else if ((comando == 'd') && (x < 7)) x++;    // direita
    else if ((comando == 's') && (y > 0)) y--;    // baixo
    else {
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

        // busca qual é o tesouro com base em sua posição em relação ao local encontrado
        for (int i = 0; i < TAM_MAX; i++) {
            if (tabuleiro->tesouros[i].posicao == y * TAM_MAX + x) {
                return tabuleiro->tesouros[i].arquivo;
            }
        }
    }

    // so movimenta o jogador
    return MOVIMENTO_ACEITO;
}

void libera_tabuleiro(tabuleiro_t *tabuleiro) {
    for (int i = 0; i < TAM_MAX; i++) {
        free(tabuleiro->matriz[i]);
    }
    free(tabuleiro->matriz);
    for (int i = 0; i < TAM_MAX; i++) {
        free(tabuleiro->deslocamento[i]);
    }
    free(tabuleiro->deslocamento);
    free(tabuleiro);
}