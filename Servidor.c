#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define TAMANHO 1024

int num_clients_sessao = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Função para registrar logs do servidor
void escrever_log(const char *mensagem) {
    pthread_mutex_lock(&log_mutex);
    FILE *f = fopen("./logs/log.txt", "a");
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro log.txt\n");
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = 0;  // Remove a nova linha do timestamp

    fprintf(f, "%s | %s\n", timestamp, mensagem);
    fclose(f);
    pthread_mutex_unlock(&log_mutex);
}

// Função para ler configurações do arquivo config.txt
void ler_configuracao(char *config_path, char *ficheiro_jogos, char *ficheiro_solucoes, int *porta) {
    FILE *config = fopen(config_path, "r");
    if (config == NULL) {
        printf("Erro ao abrir o ficheiro de configuração.\n");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), config)) {
        char *token = strtok(linha, "=");

        if (strcmp(token, "ficheiro_jogos") == 0) {
            token = strtok(NULL, "\n");
            strcpy(ficheiro_jogos, token);
        } else if (strcmp(token, "ficheiro_solucoes") == 0) {
            token = strtok(NULL, "\n");
            strcpy(ficheiro_solucoes, token);
        } else if (strcmp(token, "porta") == 0) {
            token = strtok(NULL, "\n");
            *porta = atoi(token);
        }
    }
    fclose(config);
    escrever_log("Inicio do servidor: Configuração lida com sucesso");
    printf("Configuração carregada com sucesso\n"); // Debug
}

// Função para enviar o menu para o cliente
void enviar_menu(int client_socket) {
    const char *menu =
        "---------- Menu de Sudoku ----------\n"
        "1. Resolver Tabuleiro.\n"
        "2. O Servidor revela a Solução.\n"
        "3. O Cliente resolve a Solução.\n"
        "4. Desistir.\n"
        "------------------------------------\n"
        "Escolha uma opção: ";
    printf("Enviando menu para o cliente\n"); // Debug
    send(client_socket, menu, strlen(menu), 0);
}

void formatar_tabuleiro(char *tabuleiro, char *formatted_board) {
    int index = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            formatted_board[index++] = tabuleiro[i * 9 + j];
            if ((j + 1) % 3 == 0 && j < 8) {
                formatted_board[index++] = ' ';
                formatted_board[index++] = '|';
                formatted_board[index++] = ' ';
            } else {
                formatted_board[index++] = ' ';
            }
        }
        formatted_board[index++] = '\n';
        if ((i + 1) % 3 == 0 && i < 8) {
            snprintf(formatted_board + index, 24, "------+-------+------\n");
            index += strlen("------+-------+------\n");
        }
    }
    formatted_board[index] = '\0';
}

// Função para escolher e enviar o tabuleiro para o cliente
void escolhe_tabuleiro(int client_socket, int num) {
    char buffer[BUFFER_SIZE];
    FILE *f = fopen("./jogos_solucoes/jogos.txt", "r");
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro dos jogos para leitura.\n");
        return;
    }

    printf("Escolhendo tabuleiro com ID %d\n", num); // Debug
    while (fgets(buffer, BUFFER_SIZE, f) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        int id = atoi(buffer);
        if (fgets(buffer, BUFFER_SIZE, f) == NULL) {
            break;
        }
        buffer[strcspn(buffer, "\n")] = 0;
        if (num == id) {
            char formatted_tabuleiro[BUFFER_SIZE];
            formatar_tabuleiro(buffer, formatted_tabuleiro);
            printf("Enviando tabuleiro para o cliente\n"); // Debug
            send(client_socket, formatted_tabuleiro, strlen(formatted_tabuleiro), 0);
            break;
        }
    }
    fclose(f);
}

// Função para enviar solução para o cliente
void envia_solucao(int client_socket, int num) {
    char buffer[BUFFER_SIZE];
    FILE *f = fopen("./jogos_solucoes/solucoes.txt", "r");
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro das soluções para leitura.\n");
        return;
    }

    printf("Enviando solução com ID %d para o cliente\n", num); // Debug
    while (fgets(buffer, BUFFER_SIZE, f) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        int id = atoi(buffer);
        if (fgets(buffer, BUFFER_SIZE, f) == NULL) {
            break;
        }
        buffer[strcspn(buffer, "\n")] = 0;
        if (num == id) {
            char formatted_tabuleiro[BUFFER_SIZE];
            formatar_tabuleiro(buffer, formatted_tabuleiro);
            send(client_socket, formatted_tabuleiro, strlen(formatted_tabuleiro), 0);
            break;
        }
    }
    fclose(f);
}

// Função para gerenciar cada cliente
void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);
    char buffer[BUFFER_SIZE];
    int opcao, client_id;
    int num = (rand() % 2) + 1;

    if (recv(sock, &client_id, sizeof(client_id), 0) <= 0) {
        perror("Erro ao receber ID do cliente");
        close(sock);
        return NULL;
    }
    printf("Novo cliente conectado com ID: %d\n", client_id); // Debug
    escrever_log("Novo cliente conectado");

    escolhe_tabuleiro(sock, num);
    enviar_menu(sock);

    int running = 1;
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Cliente %d desconectado\n", client_id);
            escrever_log("Cliente desconectado");
            break;
        }

        buffer[bytes_received] = '\0';
        opcao = atoi(buffer);
        printf("Cliente %d selecionou a opção %d\n", client_id, opcao); // Debug

        switch (opcao) {
            case 1:
                printf("Cliente %d selecionou inserir um valor no Sudoku\n", client_id);
                strcpy(buffer, "Opção 1: Valor inserido.\n");
                break;
            case 2:
                printf("Cliente %d pediu para revelar a solução.\n", client_id);
                envia_solucao(sock, num);
                strcpy(buffer, "Opção 2: Solução revelada.\n");
                break;
            case 3:
                printf("Cliente %d resolveu a solução localmente.\n", client_id);
                strcpy(buffer, "Opção 3: Solução resolvida pelo cliente.\n");
                break;
            case 4:
                printf("Cliente %d desistiu do jogo.\n", client_id);
                strcpy(buffer, "Opção 4: Saindo do jogo.\n");
                running = 0;
                break;
            default:
                printf("Cliente %d selecionou uma opção inválida.\n", client_id);
                strcpy(buffer, "Opção inválida! Tente novamente.\n");
                break;
        }

        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    pthread_mutex_lock(&clients_mutex);
    num_clients_sessao--;
    pthread_mutex_unlock(&clients_mutex);

    printf("Clientes atuais: %d\n", num_clients_sessao);
    printf("Conexão com cliente %d encerrada\n", client_id); // Debug
    escrever_log("Conexão com cliente encerrada");
    return NULL;
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket, porta;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if (argc < 2) {
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }

    char ficheiro_jogos[100], ficheiro_solucoes[100];
    ler_configuracao(argv[1], ficheiro_jogos, ficheiro_solucoes, &porta);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erro ao criar socket do servidor");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(porta);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        perror("Erro ao ouvir no socket");
        close(server_socket);
        return 1;
    }

    printf("Servidor iniciado na porta %d\n", porta); // Debug
    srand(time(NULL));

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Erro ao aceitar conexão");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (num_clients_sessao >= 10) {
            close(client_socket);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        num_clients_sessao++;
        pthread_mutex_unlock(&clients_mutex);

        int *new_sock = malloc(sizeof(int));
        if (new_sock == NULL) {
            perror("Erro ao alocar memória para new_sock");
            close(client_socket);
            continue;
        }
        *new_sock = client_socket;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)new_sock) != 0) {
            perror("Erro ao criar thread para o cliente");
            free(new_sock);
            close(client_socket);
            continue;
        }

        pthread_detach(tid);
    }

    close(server_socket);
    return 0;
}
