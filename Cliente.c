#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define SIZE 9

void string_para_matriz(char *tabuleiro_str, int matriz[SIZE][SIZE]) {
    int i = 0, j = 0;
    for (int k = 0; k < strlen(tabuleiro_str); k++) {
        if (tabuleiro_str[k] >= '1' && tabuleiro_str[k] <= '9') {
            matriz[i][j] = tabuleiro_str[k] - '0';
        } else if (tabuleiro_str[k] == '_') {
            matriz[i][j] = 0;
        } else {
            continue;
        }

        j++;
        if (j == SIZE) {
            j = 0;
            i++;
        }

        if (i == SIZE) {
            break;
        }
    }
}

void escrever_log_cliente(const char *mensagem) {
    FILE *f = fopen("./logs/cliente_log.txt", "a");
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro log.txt\n");
        return;
    }

    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = 0;

    fprintf(f, "%s | %s.\n", timestamp, mensagem);
    fclose(f);
}

void ler_configuracao_cliente(const char *config_path, int *porta, char *ip_server) {
    FILE *config = fopen(config_path, "r");
    if (config == NULL) {
        printf("Erro ao abrir o ficheiro de configuração.\n");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), config)) {
        char *token = strtok(linha, "=");

        if (strcmp(token, "porta") == 0) {
            token = strtok(NULL, "\n");
            *porta = atoi(token);
        } else if (strcmp(token, "ip_servidor") == 0) {
            token = strtok(NULL, "\n");
            strcpy(ip_server, token);
        }
    }
    fclose(config);
    escrever_log_cliente("Configuração lida com sucesso");
}

int get_new_user_id() {
    FILE *file = fopen("users.txt", "r+");
    if (file == NULL) {
        file = fopen("users.txt", "w+");
        if (file == NULL) {
            perror("Erro ao abrir o arquivo de ID");
            exit(EXIT_FAILURE);
        }
        fprintf(file, "1");
        fclose(file);
        return 1;
    }

    int id;
    fscanf(file, "%d", &id);
    id++;
    rewind(file);
    fprintf(file, "%d", id);
    fclose(file);
    return id;
}

int criar_socket_cliente() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erro ao criar socket");
        escrever_log_cliente("Erro no socket do cliente");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

void configurar_endereco_servidor(struct sockaddr_in *server_addr, int porta, const char *ip_server) {
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(porta);
    server_addr->sin_addr.s_addr = inet_addr(ip_server);
}

void conectar_servidor(int client_socket, struct sockaddr_in *server_addr) {
    if (connect(client_socket, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        escrever_log_cliente("Erro ao conectar ao servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
}

void enviar_id_cliente(int client_socket, int client_id) {
    if (send(client_socket, &client_id, sizeof(client_id), 0) < 0) {
        perror("Erro ao enviar ID do cliente");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
}
void imprima_matriz(int matriz[9][9]){
for(int i=0;i<9;i++){
    for (int j = 0; j < 9; j++)
    {
        printf("%d ", matriz[i][j]);
    }
    printf("\n");
}
}
int escolhe_celula_sem_nada_aleatoria(int matriz[9][9], int *linha, int *coluna) {
    // Lista para armazenar todas as células vazias
    int celulas_vazias[81][2]; // No máximo 81 células em uma matriz 9x9
    int total_vazias = 0;

    // Percorrer a matriz e registrar coordenadas de células vazias
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (matriz[i][j] == 0) {
                celulas_vazias[total_vazias][0] = i; // Linha
                celulas_vazias[total_vazias][1] = j; // Coluna
                total_vazias++;
            }
        }
    }

    // Se não houver células vazias, retorna 0 indicando falha
    if (total_vazias == 0) {
        return 0;
    }

    // Passo 2: Escolher uma célula vazia aleatoriamente
    srand(time(NULL)); // Inicializa o gerador de números aleatórios
    int indice_aleatorio = rand() % total_vazias;

    // Obter a célula vazia aleatória
    *linha = celulas_vazias[indice_aleatorio][0];
    *coluna = celulas_vazias[indice_aleatorio][1];

    return 1; // Retorna 1 para indicar sucesso
}
// Função para enviar uma tentativa para o servidor
void envia_tentativa(int client_socket, int num, int linha, int coluna) {
    int tentativa = (rand() % 9) + 1;  // Gera um número entre 1 e 9
    char buffer[BUFFER_SIZE];

    // Formata a mensagem para envio
    snprintf(buffer, BUFFER_SIZE, "%d %d %d %d", num, linha, coluna, tentativa);

    // Envia a tentativa para o servidor
    if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
        perror("Erro ao enviar tentativa para o servidor");
        return;
    }

}
void recebe_feed_back_tentativa(int client_socket){
 char buffer[BUFFER_SIZE];
    // Recebe a resposta do servidor
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Erro ao receber resposta do servidor");
        return;
    }
    buffer[bytes_received] = '\0';  // Garante que a mensagem recebida seja uma string válida

    printf("Resposta do servidor: %s\n", buffer);
    
}
void comunicar_servidor(int client_socket) {
    char buffer[BUFFER_SIZE];
    int matriz[SIZE][SIZE] = {0}; // Inicializa a matriz com zeros
    int id_tabuleiro;
    int linha_branca;
    int coluna_branca;
    if (recv(client_socket, &id_tabuleiro, sizeof(id_tabuleiro), 0) <= 0) {
        perror("Erro ao receber ID do tabuleiro");
        close(client_socket);
        return;
    }
    printf("Id recebido %d", id_tabuleiro);
    // Recebe o tabuleiro do servidor
    int bytes_received2 = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received2 <= 0) {
        printf("Servidor desconectado.\n");
        return;
    }
    buffer[bytes_received2] = '\0'; // Corrigido para usar buffer
    printf("\nTabuleiro enviado:\n%s\n", buffer);
    send(client_socket, "Tabuleiro recebido\0", 19, 0);

    // Converte a string para a matriz
    string_para_matriz(buffer, matriz);
    printf("Matriz transformada:\n");
    imprima_matriz(matriz);
    // Recebe o menu inicial do servidor
    printf("Recebe menu inicial\n");
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    printf("Depois de receber menu inicial\n");
    if (bytes_received <= 0) {
        printf("Servidor desconectado.\n");
        return;
    }
    printf("bytes_received: %d\n", bytes_received);
    buffer[bytes_received] = '\0';
    printf("Resposta do servidor:\n%s", buffer);

    // Inicia o loop para enviar e receber respostas
    printf("Antes de while do problema");
    while (1) {
        printf("Insira um número (ou 'sair' para encerrar): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Remove o newline que `fgets` deixa no buffer
        buffer[strcspn(buffer, "\n")] = 0;

        int resposta = atoi(buffer); // Converte a entrada para inteiro
        int bytes_received3;

        // Envia a opção para o servidor
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("Erro ao enviar dados");
            return;
        }

        switch (resposta) {
            case 1:
                // Recebe resposta para a opção 1
                bytes_received3 = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received3 <= 0) {
                    printf("Servidor desconectado.\n");
                    return;
                }
                buffer[bytes_received3] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
                if (escolhe_celula_sem_nada_aleatoria(matriz, &linha_branca, &coluna_branca)) {
                    printf("Posição vazia encontrada em: linha %d, coluna %d\n", linha_branca, coluna_branca);
                    printf("O id do tabuleiro é: %d\n",id_tabuleiro);
                    //envia_tentativa(client_socket,id_tabuleiro, linha_branca, coluna_branca);
                    //recebe_feed_back_tentativa(client_socket);
                } else {
                    printf("Nenhuma posição vazia encontrada.\n");
                    printf("Sodoku resolvido.\n");

                }
                break;
            case 2:
                // Recebe resposta para a opção 2
                bytes_received3 = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received3 <= 0) {
                    printf("Servidor desconectado.\n");
                    return;
                }
                buffer[bytes_received3] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
                break;

            case 3:
                // Recebe resposta para a opção 3
                bytes_received3 = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received3 <= 0) {
                    printf("Servidor desconectado.\n");
                    return;
                }
                buffer[bytes_received3] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
                break;
            case 4:
                // Recebe resposta para a opção 3
                bytes_received3 = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received3 <= 0) {
                    printf("Servidor desconectado.\n");
                    return;
                }
                buffer[bytes_received3] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
                break;
            case 5:
                // Recebe resposta para a opção 3
                bytes_received3 = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_received3 <= 0) {
                    printf("Servidor desconectado.\n");
                    return;
                }
                buffer[bytes_received3] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
                break;
            default:
                printf("Opção inválida! Tente novamente.\n");
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    int client_socket;
    int porta;
    char ip[256];

    if (argc < 2) {
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }

    int client_id = get_new_user_id();

    ler_configuracao_cliente(argv[1], &porta, ip);

    struct sockaddr_in server_addr;

    client_socket = criar_socket_cliente();

    configurar_endereco_servidor(&server_addr, porta, ip);

    conectar_servidor(client_socket, &server_addr);

    printf("Cliente %d conectado ao servidor\n", client_id);
    enviar_id_cliente(client_socket, client_id);

    comunicar_servidor(client_socket);

    close(client_socket);
    escrever_log_cliente("Cliente desconectado");
    return 0;
}