#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg){ //manejo de errores
    perror(msg);
    printf("El servidor fue cerrado o tu oponente se desconectó.\nSe acabó el juego :(\n");
    exit(0);
}

void recv_msg(int sockfd, char *msg){ //Recibe un mensaje del socket del servidor y lo almacena en el buffer msg.
    memset(msg, 0, 4);
    int n = read(sockfd, msg, 3);

    if (n < 0 || n != 3)
        error("ERROR al leer el mensaje del socket del servidor.");
}

int recv_int(int sockfd){ //Recibe un entero del socket del servidor.
    int msg = 0;
    int n = read(sockfd, &msg, sizeof(int));

    if (n < 0 || n != sizeof(int))
        error("ERROR al leer int desde el socket del servidor");

    return msg;
}

void write_server_int(int sockfd, int msg){ //Esta función escribe un entero en el socket del servidor.
    int n = write(sockfd, &msg, sizeof(int));
    if (n < 0)
        error("ERROR al escribir int en el socket del servidor");
}

int connect_to_server(int portno){ //Esta función establece una conexión con el servidor utilizando el puerto 8080.
    struct sockaddr_in serv_addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("Error al abrir el socket del servidor");

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // IP 127.0.0.1
    serv_addr.sin_port = htons(portno); // Puerto 8080

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Error al conectarse al servidor");

    return sockfd;
}

void draw_board(char board[][3]){ //Esta función imprime el tablero de juego en la consola.
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
}

void take_turn(int sockfd){ //Esta función permite al jugador realizar un movimiento, leyendo la entrada del usuario desde la consola.
    char buffer[10];

    while (1)
    {
        printf("Ingresa 0-8 para realizar un movimiento, o 9 para ver la cantidad de jugadores activos: ");
        fgets(buffer, 10, stdin);
        int move = buffer[0] - '0';
        if (move <= 9 && move >= 0)
        {
            printf("\n");
            write_server_int(sockfd, move);
            break;
        }
        else
            printf("\nEntrada inválida. Inténtalo otra vez.\n");
    }
}

void get_update(int sockfd, char board[][3]){ //Esta función actualiza el tablero de juego basándose en la información recibida del servidor.
    int player_id = recv_int(sockfd);
    int move = recv_int(sockfd);
    board[move / 3][move % 3] = player_id ? 'X' : 'O';
}

int main(int argc, char *argv[])
{
    int sockfd = connect_to_server(8080); // Conexión a 127.0.0.1:8080

    int id = recv_int(sockfd);

    char msg[4];
    char board[3][3] = {{' ', ' ', ' '},
                        {' ', ' ', ' '},
                        {' ', ' ', ' '}};

    printf("Gato\n------------\n");

    do
    {
        recv_msg(sockfd, msg);
        if (!strcmp(msg, "HLD"))
            printf("Esperando un segundo jugador ...\n");
    } while (strcmp(msg, "SRT"));

    /* El juego ha comenzado. */
    printf("¡EL JUEGO HA EMPEZADO!\n");
    printf("Eres %c\n", id ? 'X' : 'O');

    draw_board(board);

    while (1)
    {
        recv_msg(sockfd, msg);

        if (!strcmp(msg, "TRN"))
        {
            printf("Es tu turno...\n");
            take_turn(sockfd);
        }
        else if (!strcmp(msg, "INV"))
        {
            printf("Esa posición ya ha sido jugada. Inténtalo de nuevo.\n");
        }
        else if (!strcmp(msg, "CNT"))
        {
            int num_players = recv_int(sockfd);
            printf("Actualmente hay %d jugadores activos.\n", num_players);
        }
        else if (!strcmp(msg, "UPD"))
        {
            get_update(sockfd, board);
            draw_board(board);
        }
        else if (!strcmp(msg, "WAT"))
        {
            printf("Esperando el movimiento del oponente...\n");
        }
        else if (!strcmp(msg, "WIN"))
        {
            printf("¡Ganaste!\n");
            break;
        }
        else if (!strcmp(msg, "LSE"))
        {
            printf("Perdiste.\n");
            break;
        }
        else if (!strcmp(msg, "DRW"))
        {
            printf("Empate.\n");
            break;
        }
        else
            error("Mensaje desconocido.");
    }

    printf("Juego terminado.\n");
    close(sockfd);
    return 0;
}
