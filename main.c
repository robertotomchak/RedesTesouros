/* 
    Testa o sistema com ack
    Faz isso lançando um arquivo de uma máquina para outra
*/

#include "cliente.h"
#include "servidor.h"

int main(int argc, char **argv) {
    if (argc != 2)
        exit(-1);

    char nome_rede[30];
    parametro_rede(nome_rede);

    if (argv[1][0] == '0')
        servidor(nome_rede);
    else
        cliente(nome_rede);
    return 0;
}
