#include "caca_tesouro.h"


char **inicializa_tabuleiro (){
    char **matriz = malloc(TAM_MAX * sizeof(char*));

    for (int i = 0; i < TAM_MAX; i++) {
        matriz[i] = malloc(TAM_MAX * sizeof(char));
    }
    
    for (int i = 0; i < TAM_MAX; i++) {
        for (int j = 0; j < TAM_MAX; j++) {
            matriz[i][j] = '.';
        }
    }

    return matriz;
}

void sorteia_tesouros (char **matriz, tesouro_t *tesouros){
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
        if (matriz[lin][col] != 'T' && !(lin == 0 && col == 0)) {
            matriz[lin][col] = 'T';
            tesouros[i].posicao = lin * TAM_MAX + col;
            // garante que o espaço esteja se lixo de memória
            memset(tesouros[i].arquivo, 0, sizeof(tesouros[i].arquivo));
            strcpy(tesouros[i].arquivo, arquivos[i]);
            printf("%s\n",tesouros[i].arquivo);
            i++;
        }
    }
    closedir(dir);
}

void exibe_tabuleiro (char **matriz, int pos_x, int pos_y){
    printf("   ");
    for (int i = 0; i < TAM_MAX; i++) {
        printf("%d ", i);
    }
    printf("\n");

    for (int i = 0; i < TAM_MAX; i++) {
        printf("%d  ", i);
        for (int j = 0; j < TAM_MAX; j++) {
            if(i == pos_y && j == pos_x)
                printf("J ");
            else if (matriz[i][j] == 'T')
                printf(". ");
            else
                printf("%c ", matriz[i][j]);
        }
        printf("\n");
    }
}

void abrir_arquivo(char *arquivo){
    printf("Abrindo o arquivo %s...\n", arquivo);
    char comando[128];
    snprintf(comando, sizeof(comando), "xdg-open './objetos/%s' 2>/dev/null >/dev/null &", arquivo);
    system(comando);
}

void movimentacao (char **matriz, char comando, int *pos_x, int *pos_y, unsigned short *cont_tesouros, tesouro_t *tesouros){
    int x, y;
    x = *pos_x; y = *pos_y;
    if (comando != 'a' && comando != 'w' && comando != 'd' && comando != 's') {
        printf("Comando inválido. Use apenas w, a, s ou d.\n");
        return;
    }

    if ((comando == 'a') && (x > 0)) x--; // esquerda
    else if((comando == 'w') && (y > 0)) y--; // cima
    else if((comando == 'd') && (x <  7)) x++; // direita
    else if((comando == 's') && (y < 7)) y++; // baixo
    else{ 
        printf("Movimento inválido, bateu na parede.\n");
        return;
    }
    matriz[*pos_y][*pos_x] = '*';
    
    // arruma as posições    
    *pos_x = x; *pos_y = y;
    
    if (matriz[*pos_y][*pos_x] == 'T') {
        (*cont_tesouros)++;
        printf("Parabéns! Você encontrou um tesouro em (%d,%d)!\n", *pos_x, *pos_y);
        for (int i = 0; i < TAM_MAX; i++) {
            if (tesouros[i].posicao == *pos_y * TAM_MAX + *pos_x) {
                abrir_arquivo(tesouros[i].arquivo);
            }
        }
    }
}