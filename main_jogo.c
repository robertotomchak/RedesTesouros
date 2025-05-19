#include "caca_tesouro.h"

int main (){
    char **matriz = inicializa_tabuleiro();
    tesouro_t tesouros[TAM_MAX] = {0};
    sorteia_tesouros(matriz, tesouros);

    int pos_x = 0, pos_y = 0;
    char comando;
    unsigned short cont_tesouros = 0;
    while(cont_tesouros < TAM_MAX){
        exibe_tabuleiro(matriz, pos_x, pos_y);
        scanf(" %c", &comando);
        movimentacao(matriz, comando, &pos_x, &pos_y, &cont_tesouros, tesouros);
        printf("%d\n", cont_tesouros);
    }

    return 0;
}