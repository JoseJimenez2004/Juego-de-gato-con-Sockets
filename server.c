#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int player_count = 0; //contador para los jugadores
pthread_mutex_t mutexcount;

void error(const char *msg){ //manejo de errores (imprime un mensaje de error y finaliza el hilo)

    perror(msg);
    pthread_exit(NULL);
}

void write_client_int(int cli_sockfd, int msg){ //Escribe un entero en el socket del cliente especificado
    int n = write(cli_sockfd, &msg, sizeof(int));
    if (n < 0)
        error("ERROR al escribir un entero en el socket del cliente");
}

void write_clients_msg(int *cli_sockfd, char *msg){ //Envia un mensaje a ambos clientes en el hilo
    write_client_msg(cli_sockfd[0], msg);
    write_client_msg(cli_sockfd[1], msg);
}

void write_clients_int(int *cli_sockfd, int msg){ //Envia un mensaje a ambos clientes en el hilo
    write_client_int(cli_sockfd[0], msg);
    write_client_int(cli_sockfd[1], msg);
}

int setup_listener(int portno){ //configuramos el socket en el puerto 8080 y lo vinculamos a la direccion local
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR al abrir el socket del oyente.");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR al enlazar el socket del oyente.");

    return sockfd;
}

int recv_int(int cli_sockfd){ //Recibimos un entero del cliente especificado
    int msg = 0;
    int n = read(cli_sockfd, &msg, sizeof(int));

    if (n < 0 || n != sizeof(int))
        return -1;

    return msg;
}

void write_client_msg(int cli_sockfd, char *msg){ //Envia un mensaje al cliente especificado

    int n = write(cli_sockfd, msg, strlen(msg));
    if (n < 0)
        error("ERROR al escribir un mensaje en el socket del cliente");
}

void get_clients(int lis_sockfd, int *cli_sockfd){ //Acepta conexiones de clientes y las almacena en el arreglo cli_sockfd. Espera nuevas conexiones mientras haya menos de dos clientes.

    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    int num_conn = 0;
    while(num_conn < 2)
    {
        listen(lis_sockfd, 253 - player_count);

        memset(&cli_addr, 0, sizeof(cli_addr));

        clilen = sizeof(cli_addr);

        cli_sockfd[num_conn] = accept(lis_sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (cli_sockfd[num_conn] < 0)
            error("ERROR al aceptar una conexión de un cliente.");

        write(cli_sockfd[num_conn], &num_conn, sizeof(int));

        pthread_mutex_lock(&mutexcount);
        player_count++;
        printf("Número de jugadores ahora es %d.\n", player_count);
        pthread_mutex_unlock(&mutexcount);

        if (num_conn == 0) {
            write_client_msg(cli_sockfd[0], "HLD");
        }

        num_conn++;
    }
}

int get_player_move(int cli_sockfd){//Solicita al cliente un movimiento y espera recibirlo.
    write_client_msg(cli_sockfd, "TRN");
    return recv_int(cli_sockfd);
}

int check_move(char board[][3], int move, int player_id){//Verifica si un movimiento es válido en el tablero para un jugador específico.
    if ((move == 9) || (board[move/3][move%3] == ' ')) {
        return 1;
   }
   else {
       return 0;
   }
}

void update_board(char board[][3], int move, int player_id){ //Actualiza el tablero con el movimiento del jugador.
    board[move/3][move%3] = player_id ? 'X' : 'O';
}

void draw_board(char board[][3]){//Imprime el tablero en la consola.
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
}

void send_update(int *cli_sockfd, int move, int player_id){ //Envía una actualización a ambos clientes después de que se realiza un movimiento.
    write_clients_msg(cli_sockfd, "UPD");
    write_clients_int(cli_sockfd, player_id);
    write_clients_int(cli_sockfd, move);
}

void send_player_count(int cli_sockfd){ //Envía la cantidad de jugadores a un cliente específico.
    write_client_msg(cli_sockfd, "CNT");
    write_client_int(cli_sockfd, player_count);
    //printf("[DEBUG] Cantidad de jugadores enviada.\n");
}

int check_board(char board[][3], int last_move){ //Verifica si el juego ha terminado después de un movimiento.
    int row = last_move/3;
    int col = last_move%3;

    if (board[row][0] == board[row][1] && board[row][1] == board[row][2]) {
        return 1;
    }
    else if (board[0][col] == board[1][col] && board[1][col] == board[2][col]) {
        return 1;
    }
    else if (!(last_move % 2)) {
        if ((last_move == 0 || last_move == 4 || last_move == 8) && (board[1][1] == board[0][0] && board[1][1] == board[2][2])) {
            return 1;
        }
        if ((last_move == 2 || last_move == 4 || last_move == 6) && (board[1][1] == board[0][2] && board[1][1] == board[2][0])) {
            return 1;
        }
    }

    return 0;
}

void *run_game(void *thread_data){ //logica del juego
    int *cli_sockfd = (int*)thread_data;
    char board[3][3] = { {' ', ' ', ' '},
                         {' ', ' ', ' '},
                         {' ', ' ', ' '} };

    printf("¡Juego en marcha!\n");
    write_clients_msg(cli_sockfd, "SRT");
    //printf("[DEBUG] Mensaje de inicio enviado.\n");
    draw_board(board);

    int prev_player_turn = 1;
    int player_turn = 0;
    int game_over = 0;
    int turn_count = 0;
    while(!game_over) {
        if (prev_player_turn != player_turn)
            write_client_msg(cli_sockfd[(player_turn + 1) % 2], "WAT");

        int valid = 0;
        int move = 0;
        while(!valid) {
            move = get_player_move(cli_sockfd[player_turn]);
            if (move == -1)
                break;
            printf("El jugador %d jugó en la posición %d\n", player_turn, move);

            valid = check_move(board, move, player_turn);
            if (!valid) {
                printf("El movimiento fue inválido. Intentemos de nuevo...\n");
                write_client_msg(cli_sockfd[player_turn], "INV");
            }
        }

        if (move == -1) {
            printf("Jugador desconectado.\n");
            break;
        }
        else if (move == 9) {
            prev_player_turn = player_turn;
            send_player_count(cli_sockfd[player_turn]);
        }
        else {
            update_board(board, move, player_turn);
            send_update(cli_sockfd, move, player_turn);
            draw_board(board);
            game_over = check_board(board, move);
            if (game_over == 1) {
                write_client_msg(cli_sockfd[player_turn], "WIN");
                write_client_msg(cli_sockfd[(player_turn + 1) % 2], "LSE");
                printf("El jugador %d ganó.\n", player_turn);
            }
            else if (turn_count == 8) {
                printf("Empate.\n");
                write_clients_msg(cli_sockfd, "DRW");
                game_over = 1;
            }

            prev_player_turn = player_turn;
            player_turn = (player_turn + 1) % 2;
            turn_count++;
        }
    }

    printf("Juego terminado.\n");

    close(cli_sockfd[0]);
    close(cli_sockfd[1]);

    pthread_mutex_lock(&mutexcount);
    player_count--;
    printf("Número de jugadores ahora es %d.", player_count);
    player_count--;
    printf("Número de jugadores ahora es %d.", player_count);
    pthread_mutex_unlock(&mutexcount);

    free(cli_sockfd);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int portno = 8080; // Puerto predeterminado

    int lis_sockfd = setup_listener(portno);
    pthread_mutex_init(&mutexcount, NULL);

    while (1) {
        if (player_count <= 252) {
            int *cli_sockfd = (int*)malloc(2*sizeof(int));
            memset(cli_sockfd, 0, 2*sizeof(int));

            get_clients(lis_sockfd, cli_sockfd);

            pthread_t thread;
            int result = pthread_create(&thread, NULL, run_game, (void *)cli_sockfd);
            if (result){
                printf("La creación del hilo falló con el código de retorno %d\n", result);
                exit(-1);
            }
        }
    }

    close(lis_sockfd);

    pthread_mutex_destroy(&mutexcount);
    pthread_exit(NULL);
}

