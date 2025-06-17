/*
    Define os tipos de mensagens
*/

#define TIPO_ACK 0               // ack normal
#define TIPO_NACK 1              // nack normal
#define TIPO_OK_ACK 2            // movimento aceito + ack
// tipo = 3 livre
#define TIPO_TAMANHO 4           // tamanho do arquivo a ser enviado
#define TIPO_DADOS 5             // dados do arquivo
#define TIPO_TEXTO_ACK 6         // arquivo é texto + nome do arquivo + ack
#define TIPO_VIDEO_ACK 7         // arquivo é vídeo + nome do arquivo + ack
#define TIPO_IMAGEM_ACK 8        // arquivo é imagem + nome do arquivo + ack
#define TIPO_FIM_ARQUIVO 9       // terminou de enviar arquivo
#define TIPO_DIREITA 10          // movimente para direita
#define TIPO_CIMA 11             // movimente para cima
#define TIPO_BAIXO 12            // movimente para baixo
#define TIPO_ESQUERDA 13         // movimente para esquerda
// tipo = 14 livre
#define TIPO_ERRO 15             // envia uma informação de erro