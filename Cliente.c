#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 4001
#define BUFFER_SIZE 1024

void escrever_log_cliente(const char *mensagem) {
    FILE *f = fopen("log.txt", "a");
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
void ler_configuracao_cliente(char *config_path,  int *porta) {
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
        } 
    }
    fclose(config); 
    escrever_log_cliente("Inicio de um cliente:Configuracao lida com sucesso");
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
void configurar_endereco_servidor(struct sockaddr_in *server_addr) {
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = inet_addr("10.2.15.230");
}
void conectar_servidor(int client_socket, struct sockaddr_in *server_addr) {
    if (connect(client_socket, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0) {
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

    // Recebe o menu inicial do servidor apenas uma vez
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

        // Verifica se o usuário deseja sair
        if (strncmp(buffer, "sair", 4) == 0) {
            printf("Saindo do cliente...\n");
            break;
        }

        // Envia a opção escolhida para o servidor
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("Erro ao enviar dados");
            break;
        }

        // Recebe a resposta do servidor para a opção escolhida
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Servidor desconectado.\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Resposta do servidor:\n%s", buffer);
    }
}


int main(int argc, char *argv[]) {
    int client_socket;
     int porta;
       if (argc < 2) {
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }
    ler_configuracao_cliente(argv[1], &porta);
    struct sockaddr_in server_addr;
    // Cria o socket do cliente
    client_socket = criar_socket_cliente();
    // Configura o endereço do servidor
    configurar_endereco_servidor(&server_addr);
    // Conecta ao servidor
    conectar_servidor(client_socket, &server_addr);
    // Gera e envia o ID do cliente
    int client_id = get_new_user_id();
    printf("Cliente %d conectado ao servidor\n", client_id);
    enviar_id_cliente(client_socket, client_id);
    // Comunica com o servidor
    comunicar_servidor(client_socket);
    // Fecha o socket e registra a desconexão
    close(client_socket);
    escrever_log_cliente("Cliente desconectado");
    return 0;
}