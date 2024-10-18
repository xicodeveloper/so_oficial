#include "header.h"
#define TAMANHO 9


// Função de jogo
void jogar_sudoku(int tabuleiro[TAMANHO][TAMANHO]) {
    int linha, col, num;
    char buffer[20];
    int erros = 0;

    while (1) {
        imprimir_tabuleiro(tabuleiro);

        // Verificar vitória
        if (verificar_vitoria(tabuleiro)) {
            printf("Parabéns! Completou o Sudoku corretamente!\n");
            FILE *fe = fopen("log.txt", "a");
            if (fe != NULL) {
                fprintf(fe, "Parabéns! Completou o Sudoku corretamente!\n");
                fclose(fe);
            }
            break;
        }

        printf("Insira a linha (1-9), coluna (1-9) e o número (1-9) ou 0 0 0 para sair: ");
        

        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // Intentar extraer los tres números
            int result = sscanf(buffer, "%d %d %d", &linha, &col, &num);
            // Verificar si se leyeron los tres números
            if (result != 3) {
                printf("Erro esperava-se 3 números inteiros.\n");
                continue; // Pedir entrada nuevamente
            }

            // Si los números son válidos, salir del bucle
        } else {
            printf("Erro ao ler a entrada.\n");
            continue; // Pedir entrada nuevamente
        }
            

        if (linha == 0 && col == 0 && num == 0) {
            printf("Jogo terminado. Até à próxima!\n");
            FILE *fe = fopen("log.txt", "a");
            if (fe != NULL) {
                fprintf(fe, "Jogo terminado. Até à próxima!\n");
                fclose(fe);
            }
            break;
        }

        linha--; // Ajustar  índice 0
        col--;   // Ajustar  índice 0

        // Verifica se a posição está vazia e o número é válido
        if (tabuleiro[linha][col] == 0 && pode_colocar(tabuleiro, linha, col, num)) {
            tabuleiro[linha][col] = num;

             FILE *fe = fopen("log.txt", "a");
                if (fe != NULL) {
                    fprintf(fe,"Cliente inseriu um número no soduku!\n");
                }
                fclose(fe);
        } else {
             FILE *fe = fopen("log.txt", "a");
            fprintf(fe,"Movimento inválido!\n");
            fclose(fe);
            printf("Movimento inválido!\n");
            erros++;
            if (erros >= 3) {
                printf("Perdeu o jogo. Excedeu as 3 tentativas.\n");
                FILE *fe = fopen("log.txt", "a");
                if (fe != NULL) {
                    fprintf(fe, "Perdeu o jogo. Excedeu as 3 tentativas.\n");
                    fclose(fe);
                }
                break;
            }
        }
    }
}

/*
void connect_to_server() {
    struct sockaddr_un addr;
    int i;
    int ret;
    int data_socket;
    char buffer[BUFFER_SIZE];

    // Cria o socket de dados
    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Limpa a estrutura de endereço
    memset(&addr, 0, sizeof(struct sockaddr_un));

    // Configura o endereço do socket
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    // Conecta ao servidor
    ret = connect(data_socket, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un));
    if (ret == -1) {
        fprintf(stderr, "O servidor está indisponível.\n");
        close(data_socket);
        exit(EXIT_FAILURE);
    }

    printf("Conectado ao servidor.\n");

    // Enviar números ao servidor
    do {
        printf("Digite um número para enviar ao servidor (0 para sair):\n");
        scanf("%d", &i);

        // Enviar o número
        ret = write(data_socket, &i, sizeof(int));
        if (ret == -1) {
            perror("Erro ao enviar dados");
            break;
        }
        printf("Número enviado: %d\n", i);
    } while (i != 0);

    // Solicitar resultado
    memset(buffer, 0, BUFFER_SIZE);
    strncpy(buffer, "RES", sizeof(buffer) - 1);
    ret = write(data_socket, buffer, strlen(buffer));
    if (ret == -1) {
        perror("Erro ao solicitar resultado");
        close(data_socket);
        exit(EXIT_FAILURE);
    }

    // Receber o resultado
    memset(buffer, 0, BUFFER_SIZE);
    ret = read(data_socket, buffer, BUFFER_SIZE);
    if (ret == -1) {
        perror("Erro ao receber resultado");
        close(data_socket);
        exit(EXIT_FAILURE);
    }

    // Mostrar o resultado
    printf("Resultado recebido do servidor: %s\n", buffer);

    // Fechar o socket
    close(data_socket);
    printf("Conexão encerrada.\n");

    exit(EXIT_SUCCESS);
}
    void main() {
    printf("Iniciando cliente...\n");
    connect_to_server();  // Inicia a conexão com o servidor
    return 0;
}*/

