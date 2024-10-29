#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Função para gerar um ID aleatório para o cliente
int gerar_id_cliente() {
    srand(time(NULL));
    return rand() % 1000 + 1;  // ID entre 1 e 1000
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int client_id = gerar_id_cliente();

    // Cria o socket do cliente
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configura o endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // IP local para testes

    // Conecta ao servidor
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Envia o ID do cliente ao servidor
    if (send(client_socket, &client_id, sizeof(client_id), 0) <= 0) {
        perror("Erro ao enviar ID do cliente");
        close(client_socket);
        return 1;
    }
    printf("Conectado ao servidor com ID %d\n", client_id);

    // Loop de interação com o menu
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);  // Limpa o buffer antes de receber o menu
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s", buffer);  // Exibe o menu recebido
        } else {
            printf("Servidor desconectado.\n");
            break;
        }

        // Solicita a opção do usuário
        printf("Insira um número (ou 'sair' para encerrar): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Verifica se o usuário deseja sair
        if (strncmp(buffer, "sair", 4) == 0) {
            printf("Encerrando a conexão com o servidor...\n");
            break;
        }

        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);  // Limpa o buffer antes de receber a resposta

        // Recebe a resposta do servidor
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Resposta do servidor:\n%s", buffer);

            // Verifica se o servidor mandou uma mensagem de término
            if (strstr(buffer, "Saindo do jogo") != NULL) {
                printf("O servidor encerrou a conexão.\n");
                break;
            }
        } else {
            printf("Servidor desconectado.\n");
            break;
        }
    }

    close(client_socket);
    return 0;
}
