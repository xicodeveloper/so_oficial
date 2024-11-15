#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define TAMANHO 1024
#define SIZE 4
#define LC 9

int matriz_of[SIZE][LC][LC] = {{{0}}};
int matriz_solucao[SIZE][LC][LC] = {{{0}}};

int num_clients_sessao = 0;
int server_socket, client_socket, porta;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clients_mutex_board = PTHREAD_MUTEX_INITIALIZER;
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
void transforma_matriz_solucao(int matriz_of[SIZE][LC][LC]) {
    char buffer[BUFFER_SIZE];
    FILE *f = fopen("./jogos_solucoes/solucoes.txt", "r");
    
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro dos jogos para leitura.\n");
        return;
    }

    int jogo_index = 0;
    while (jogo_index < SIZE) {
        // Ignora a linha do índice do jogo (1, 2, 3, etc.)
        if (fgets(buffer, BUFFER_SIZE, f) == NULL) {
            break; // Encerra o loop se não houver mais linhas
        }

        // Lê a linha contendo a solução do jogo (81 caracteres)
        if (fgets(buffer, BUFFER_SIZE, f) == NULL) {
            break; // Encerra o loop se não houver linha de solução
        }
        buffer[strcspn(buffer, "\n")] = 0; // Remove o '\n' no final da linha, se houver

        // Verifica se a linha de solução tem o tamanho correto (81 caracteres)
        if (strlen(buffer) != 81) {
            printf("Linha de solução do jogo %d tem tamanho incorreto.\n", jogo_index + 1);
            continue; // Passa para o próximo jogo se o tamanho for incorreto
        }

        // Preenche a matriz 9x9 com os valores do buffer
        int k = 0; // Índice do caractere no buffer
        for (int i = 0; i < LC; i++) {
            for (int j = 0; j < LC; j++) {
                matriz_of[jogo_index][i][j] = buffer[k] - '0'; // Converte caractere para inteiro
                k++; // Avança para o próximo caractere do buffer
            }
        }
        
        jogo_index++; // Próximo jogo
    }
    
    fclose(f);
}


void transforma_matriz(int matriz_of[SIZE][LC][LC]) {
    char buffer[BUFFER_SIZE];
    FILE *f = fopen("./jogos_solucoes/jogos.txt", "r");
    
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro dos jogos para leitura.\n");
        return;
    }

    int jogo_index = 0;
    while (fgets(buffer, BUFFER_SIZE, f) != NULL && jogo_index < SIZE) {
        buffer[strcspn(buffer, "\n")] = 0; // Remover o '\n'        
        if (fgets(buffer, BUFFER_SIZE, f) == NULL) {
            break; // Se não houver outra linha, encerra o loop
        }
        buffer[strcspn(buffer, "\n")] = 0; // Remover o '\n'

        // Preencher a matriz 9x9 com os valores do buffer
        int k = 0; // Índice do caractere no buffer
        for (int i = 0; i < LC; i++) {
            for (int j = 0; j < LC; j++) {
                if (buffer[k] == '_') {
                    matriz_of[jogo_index][i][j] = 0; // Usar 0 para posições desconhecidas
                } else {
                    matriz_of[jogo_index][i][j] = buffer[k] - '0'; // Converter caractere para inteiro
                }
                k++; // Avançar para o próximo caractere do buffer
            }
        }
        jogo_index++; // Próximo jogo
    }
    
    fclose(f);
    return; // Retornar a quantidade de jogos lidos
}

void ler_matrizes(int matriz_of[SIZE][LC][LC]) {
    for (int i = 0; i < SIZE; i++) {
        printf("Jogo %d:\n", i + 1);
        for (int j = 0; j < LC; j++) {
            for (int l = 0; l < LC; l++) {
                printf("%d ", matriz_of[i][j][l]);
            }
            printf("\n");  // Nova linha após cada linha da matriz
        }
        printf("\n");  // Linha em branco entre jogos
    }
}


void handle_sigint(int sig) {
    printf("\nSIGINT received. Closing server socket...\n");
    close(server_socket);
    exit(0);
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
        "1. Resolver Tabuleiro Total.\n"
        "2. Resolver Tabuleiro Parcial.\n"
        "3. O Servidor revela a Solução.\n"
        "4. O Servidor revela a Solução Parcial.\n"
        "5. Desistir.\n"
        "------------------------------------\n";
    printf("Enviando menu para o cliente\n"); // Debug
    send(client_socket, menu, strlen(menu), 0);
}


// Função auxiliar para converter o tabuleiro 2D em uma string unidimensional
void converter_tabuleiro_para_string(int tabuleiro[LC][LC], char *tabuleiro_str) {
    int index = 0;
    for (int i = 0; i < LC; i++) {
        for (int j = 0; j < LC; j++) {
            if (tabuleiro[i][j] == 0) {
                tabuleiro_str[index++] = '_'; // Representação para valores desconhecidos
            } else {
                tabuleiro_str[index++] = tabuleiro[i][j] + '0'; // Converte para caractere
            }
        }
    }
    tabuleiro_str[index] = '\0'; // Termina a string
}

// Formata a string do tabuleiro com as linhas e colunas do jogo
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
void escolhe_tabuleiro(int client_socket, int num, int matriz[SIZE][LC][LC]) {
    char tabuleiro_str[LC * LC + 1];  // String temporária para armazenar o tabuleiro unidimensional
    char formatted_tabuleiro[BUFFER_SIZE];
    char received_message[BUFFER_SIZE];

    if (num < 1 || num > SIZE) {
        printf("ID do tabuleiro inválido: %d\n", num);
        return;
    }

    printf("Escolhendo tabuleiro com ID %d\n", num); // Debug
    converter_tabuleiro_para_string(matriz[num - 1], tabuleiro_str); // Converte para string unidimensional
    formatar_tabuleiro(tabuleiro_str, formatted_tabuleiro); // Formata o tabuleiro

    printf("Enviando tabuleiro para o cliente\n"); // Debug
    send(client_socket, formatted_tabuleiro, strlen(formatted_tabuleiro), 0);

    // Recebe a confirmação de que o cliente recebeu o tabuleiro
    if (recv(client_socket, received_message, BUFFER_SIZE - 1, 0) > 0) {
        received_message[BUFFER_SIZE - 1] = '\0'; // Assegura-se de que a mensagem recebida seja terminada com '\0'
        if (strcmp(received_message, "Tabuleiro recebido") == 0) {
            printf("Tabuleiro recebido pelo cliente\n");
        } else {
            printf("Erro ao receber tabuleiro: %s\n", received_message);
        }
    } else {
        printf("Erro na recepção da confirmação do cliente\n");
    }
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
void enviar_id_tabuleiro(int client_socket, int num) {
    if (send(client_socket, &num, sizeof(num), 0) < 0) {
        perror("Erro ao enviar ID do tabuleiro");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
}
void recebe_tentativa_e_envia_feedback(int client_socket, int matriz_of[4][9][9]) {
    char buffer[BUFFER_SIZE];
    int num, linha, coluna, tentativa;

    printf("[DEBUG] Aguardando mensagem do cliente para receber tentativa...\n");
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("[ERRO] Falha no recv");
        return;
    }

    buffer[bytes_received] = '\0'; // Garante que a mensagem recebida é válida
    printf("[DEBUG] Mensagem recebida: '%s'\n", buffer);

    // Extrai os valores num, linha, coluna, e tentativa
    if (sscanf(buffer, "%d %d %d %d", &num, &linha, &coluna, &tentativa) != 4) {
        printf("[ERRO] Mensagem inválida recebida do cliente: '%s'\n", buffer);
        return;
    }
    printf("[DEBUG] Dados extraídos: Tabuleiro ID=%d, Linha=%d, Coluna=%d, Tentativa=%d\n", num, linha, coluna, tentativa);

    // Prepara o feedback (certo ou errado)
    char resposta[BUFFER_SIZE];
    if (matriz_of[num - 1][linha - 1][coluna - 1] == tentativa) {
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está correta.", tentativa, linha, coluna);
    } else {
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está incorreta.", tentativa, linha, coluna);
    }
    printf("[DEBUG] Feedback gerado: '%s'\n", resposta);

    // Envia o feedback ao cliente
    if (send(client_socket, resposta, strlen(resposta), 0) < 0) {
        perror("[ERRO] Falha ao enviar feedback ao cliente");
    } else {
        printf("[DEBUG] Feedback enviado ao cliente com sucesso.\n");
    }
}
// Function to manage each client
void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);
    char buffer[BUFFER_SIZE];
    int opcao, client_id;
    int num = (rand() % 4) + 1;

    if (recv(sock, &client_id, sizeof(client_id), 0) <= 0) {
        perror("Error receiving client ID");
        close(sock);
        return NULL;
    }
    printf("New client connected with ID: %d\n", client_id);
    escrever_log("New client connected");
    enviar_id_tabuleiro(sock, num);
    escolhe_tabuleiro(sock, num, matriz_of);
    enviar_menu(sock);
    printf("Sending menu to client: %d\n", client_id);

    int running = 1;
    while (running) {
        printf("Waiting for option from client: %d\n", client_id);

        // Receive the client's option
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Error receiving data from client");
            printf("Client %d disconnected due to error.\n", client_id);
            escrever_log("Client disconnected due to receive error");
            break;
        } else if (bytes_received == 0) {
            printf("Client %d disconnected.\n", client_id);
            escrever_log("Client disconnected");
            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received from client %d: '%s' (bytes_received: %d)\n", client_id, buffer, bytes_received); // Debug

        // Convert received message to an integer option
        opcao = atoi(buffer); // Try parsing the received data as an integer
        printf("Parsed option from client %d: %d\n", client_id, opcao); // Debug

// Handle the selected option
switch (opcao) {
    case 1:
        printf("Client %d selected to solve one cell\n", client_id);

        // Envia uma resposta inicial ao cliente confirmando a opção
        strcpy(buffer, "Option 1: one cell requested.\n");
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("[ERRO] Falha ao enviar resposta inicial ao cliente");
            
            break;
        }
        recebe_tentativa_e_envia_feedback(sock, matriz_solucao);
        printf("[DEBUG] Resposta inicial enviada para o cliente %d: '%s'\n", client_id, buffer);


        break;

    case 2:
        printf("Client %d requested Partial Solution.\n", client_id);
        strcpy(buffer, "Option 2: Partial Solution revealed.\n");
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("[ERRO] Falha ao enviar resposta parcial ao cliente");
            break;
        }
        printf("[DEBUG] Resposta parcial enviada para o cliente %d.\n", client_id);
        break;

    case 3:
        printf("Client %d requested Full Solution from Server.\n", client_id);
        strcpy(buffer, "Option 3: Server reveals Solution.\n");
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("[ERRO] Falha ao enviar solução completa ao cliente");
            break;
        }
        printf("[DEBUG] Mensagem de solução completa enviada ao cliente %d.\n", client_id);

        memset(buffer, 0, BUFFER_SIZE-1);
        int bytesReceived = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived < 0) {
            perror("[ERRO] Falho sincronizacao do Socket");
        }
        else{
            envia_solucao(sock, num);
        }

        // Envia a solução completa
        
        break;

    case 4:
        printf("Client %d requested Partial Solution from Server.\n", client_id);
        strcpy(buffer, "Option 4: Partial Solution revealed by server.\n");
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("[ERRO] Falha ao enviar solução parcial ao cliente");
            break;
        }
        printf("[DEBUG] Mensagem de solução parcial enviada ao cliente %d.\n", client_id);
        break;

    case 5:
        printf("Client %d quit the game.\n", client_id);
        strcpy(buffer, "Option 5: Exiting the game.\n");
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("[ERRO] Falha ao enviar mensagem de saída ao cliente");
            break;
        }
        printf("[DEBUG] Mensagem de saída enviada para o cliente %d.\n", client_id);
        running = 0; // Exit loop
        break;

    default:
        printf("Client %d selected an invalid option: %d\n", client_id, opcao);
        strcpy(buffer, "Invalid option! Please try again.\n");
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("[ERRO] Falha ao enviar mensagem de opção inválida");
        }
        else{
            printf("[DEBUG] Mensagem de opção inválida enviada para o cliente %d.\n", client_id);
        }
        printf("[DEBUG] opção escolhida pelo cliente: %d\n", opcao); 
        break;
}

// Debug opcional para verificar envio final (não necessário)
printf("[DEBUG] Opção processada para cliente %d.\n", client_id);

    }

    // Close connection and update client count
    close(sock);
    pthread_mutex_lock(&clients_mutex);
    num_clients_sessao--;
    pthread_mutex_unlock(&clients_mutex);

    printf("Current clients: %d\n", num_clients_sessao);
    printf("Connection with client %d closed\n", client_id);
    escrever_log("Client connection closed");
    return NULL;
}


int main(int argc, char *argv[]) {
    
    transforma_matriz(matriz_of);
    transforma_matriz_solucao(matriz_solucao);
    ler_matrizes(matriz_of);
    printf("----------");
    ler_matrizes(matriz_solucao);
  


    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if (argc < 2) {
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }

    char ficheiro_jogos[100], ficheiro_solucoes[100];
    ler_configuracao(argv[1], ficheiro_jogos, ficheiro_solucoes, &porta);

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

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

        pthread_mutex_lock(&clients_mutex_board);
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)new_sock) != 0) {
            perror("Erro ao criar thread para o cliente");
            free(new_sock);
            close(client_socket);
            continue;
        }
        pthread_mutex_unlock(&clients_mutex_board);

        pthread_detach(tid);
    }

    close(server_socket);
    return 0;
}