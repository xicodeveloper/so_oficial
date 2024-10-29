#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "header.h"
#define PORT 8080
#define BUFFER_SIZE 1024




// Função para obter um novo ID de usuário

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
int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Cria o socket do cliente
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erro ao criar socket");
        escrever_log_cliente("Cliente erro no socket");
        exit(EXIT_FAILURE);
    }

    // Configura o endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("10.2.15.230");  // Conectando ao servidor

    // Conecta ao servidor
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        escrever_log_cliente("Cliente erro ao conectar ao servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    
     cliente_inf cliente; // Crie uma instância da estrutura cliente_inf
    cliente.id = get_new_user_id(); // Atribua um novo ID de usuário
    strcpy(cliente.nome, "Nome do Cliente"); // Exemplo de atribuição de nome
    printf("Cliente %d Conectado ao servidor\n", cliente.id);
      // Envia o ID do cliente para o servidor
    if (send(client_socket, &cliente.id, sizeof(cliente.id), 0) < 0) {
        perror("Erro ao enviar ID do cliente");
        close(client_socket);
        exit(EXIT_FAILURE);
    }



    while (1) {
        // Lê a mensagem do usuário
        printf("Insira um numero (ou 'sair' para encerrar): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        // Verifica se o usuário quer encerrar a conexão
        if (strncmp(buffer, "sair", 4) == 0) {
            printf("Encerrando a conexão com o servidor...\n");
            escrever_log_cliente("Cliente saiu do server");
            break;
        }

        // Envia a mensagem para o servidor
        send(client_socket, buffer, strlen(buffer), 0);
    
       
        // Recebe a resposta do servidor
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Servidor resposta:\n %s\n", buffer);
             escrever_log_cliente("Cliente recebeu resposta do Servidor");

        } else {
            printf("Servidor desconectado\n");
            escrever_log_cliente("Cliente desconectado do serviodor");
            break;
        }
    }

    close(client_socket);
    return 0;
}
