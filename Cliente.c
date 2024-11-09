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

    // Percorre a string do tabuleiro diretamente, assumindo que não contém o ID
    for (int k = 0; k < strlen(tabuleiro_str); k++) {
        if (tabuleiro_str[k] >= '1' && tabuleiro_str[k] <= '9') {
            matriz[i][j] = tabuleiro_str[k] - '0'; // Converte char para int
        } else if (tabuleiro_str[k] == '_') {
            matriz[i][j] = 0; // Converte underscore para 0
        } else {
            continue; // Ignora qualquer outro caractere
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
    timestamp[strlen(timestamp) - 1] = 0; // Remove a nova linha do timestamp

    fprintf(f, "%s | %s.\n", timestamp, mensagem);
    fclose(f);
}

// Função para ler configurações do arquivo config.txt
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
    escrever_log_cliente("Inicio de um cliente: Configuração lida com sucesso");
}

// Função para obter um novo ID de usuário
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
        escrever_log_cliente("Cliente erro no socket");
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
        escrever_log_cliente("Cliente erro ao conectar ao servidor");
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
void comunicar_servidor(int client_socket) {
    char buffer[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    int matriz[SIZE][SIZE] = {0}; // Inicializa a matriz com zeros

    // Recebe o tabuleiro do servidor
    int bytes_received2 = recv(client_socket, buffer2, BUFFER_SIZE - 1, 0);
    if (bytes_received2 <= 0) {
        printf("Servidor desconectado.\n");
        return;
    }
    buffer2[bytes_received2] = '\0'; // Corrigido para usar buffer2
    printf("\nTabuleiro enviado:\n%s\n", buffer2);

    // Converte a string para a matriz
    string_para_matriz(buffer2, matriz);

    // Recebe o menu inicial do servidor
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        printf("Servidor desconectado.\n");
        return;
    }
    buffer[bytes_received] = '\0';
    printf("Resposta do servidor:\n%s", buffer);

    // Inicia o loop para enviar e receber respostas
    while (1) {
        printf("Insira um número (ou 'sair' para encerrar): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Remove o newline que `fgets` deixa no buffer
        buffer[strcspn(buffer, "\n")] = 0;

    int resposta = atoi(buffer);
        int bytes_received3;
        char buffer3[BUFFER_SIZE];
        switch (resposta){
            case 1:

            // send(client_socket, buffer, strlen(buffer), 0)
                break;
           case 2: {

                bytes_received3 = recv(client_socket, buffer3, BUFFER_SIZE - 1, 0);
                if (bytes_received3 <= 0) {
                    printf("Servidor desconectado.\n");
                return;
                }
                buffer3[bytes_received3] = '\0';
                printf("Resposta do servidor:\n%s", buffer3);
                break;
            }
            case 3:
            
                //send(client_socket, buffer, strlen(buffer), 0)
                break;
            
            default:
                break;
        }


        
    }
            // Envia a opção escolhida para o servidor
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("Erro ao enviar dados");
            break;
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

    // Gera o ID do cliente e cria o arquivo de configuração
    int client_id = get_new_user_id();

    // Lê as configurações
    ler_configuracao_cliente(argv[1], &porta, ip);

    struct sockaddr_in server_addr;

    // Cria o socket do cliente
    client_socket = criar_socket_cliente();

    // Configura o endereço do servidor com IP e porta lidos
    configurar_endereco_servidor(&server_addr, porta, ip);

    // Conecta ao servidor
    conectar_servidor(client_socket, &server_addr);

    printf("Cliente %d conectado ao servidor\n", client_id);
    enviar_id_cliente(client_socket, client_id);

    // Comunica com o servidor
    comunicar_servidor(client_socket);

    // Fecha o socket e registra a desconexão
    close(client_socket);
    escrever_log_cliente("Cliente desconectado");
    return 0;
}
