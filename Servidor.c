#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define TAMANHO 9
void escrever_log(const char *mensagem) {
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
void enviar_menu(int client_socket) {
    char menu[] = 
        "---------- Menu de Sudoku ----------\n"
        "1. Inserir.\n"
        "2. O Servidor revela a Solução.\n"
        "3. O Cliente resolve a Solução.\n"
        "4. Desistir.\n"
        "------------------------------------\n"
        "Escolha uma opção: ";
    send(client_socket, menu, strlen(menu), 0);
}

// Função que processa o menu e opções escolhidas pelo cliente
void *handle_client(void *client_socket) {
    int sock = *(int*)client_socket;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int opcao;
    int client_id;

    while (1) {
        // Envia o menu ao cliente
        enviar_menu(sock);



if (recv(sock, &client_id, sizeof(client_id), 0) <= 0) {
    perror("Erro ao receber ID do cliente");
    close(sock);
    return NULL;
}

printf("Novo cliente conectado com ID: %d\n", client_id);
printf("Iniciando jogo para o cliente %d.\n", client_id);
        // Recebe a opção do cliente
        bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Cliente desconectado\n");
            break;
        }
        
    // Inicia o jogo

        buffer[bytes_received] = '\0';
        opcao = atoi(buffer); // Converte a opção recebida para um número inteiro

        switch (opcao) {
            case 1:
                printf("Cliente %d selecionou inserir um valor no Sudoku\n", client_id);
                // Implementar a função jogar_sudoku(tabuleiro);
                // Responder ao cliente após o movimento
                send(sock, "Opção 1: Valor inserido.\n", 26, 0);
                break;
            case 2:
                printf("Cliente %d pediu para revelar a solução.\n", client_id);
                // Implementar função de leitura e exibição da solução
                // Ler solução e enviar ao cliente
                send(sock, "Opção 2: Solução revelada.\n", 28, 0);
                break;
            case 3:
                printf("Cliente %d selecionou para resolver a solução localmente.\n", client_id);
                // Implementar função que resolve e envia ao cliente
                send(sock, "Opção 3: Solução resolvida pelo cliente.\n", 41, 0);
                break;
            case 4:
                printf("Cliente %d desistiu do jogo.\n", client_id);
                send(sock, "Opção 4: Saindo do jogo.\n", 25, 0);
                goto encerra_conexao; // Encerra o jogo para o cliente
                break;
            default:
                printf(" Cliente %d Opção inválida selecionada.\n", client_id);
                send(sock, "Opção inválida! Tente novamente.\n", 35, 0);
                break;
        }
    }

encerra_conexao:
    close(sock);
    free(client_socket);
    printf("Conexão com cliente %d encerrada\n", client_id);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Erro no listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Servidor esperando conexões na porta %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Erro no accept");
            continue;
        }

        printf("Novo cliente conectado\n");

        int *new_sock = malloc(sizeof(int));
        *new_sock = client_socket;
        pthread_t client_thread;

        if (pthread_create(&client_thread, NULL, handle_client, (void*)new_sock) != 0) {
            perror("Erro ao criar thread");
            free(new_sock);
        }

        pthread_detach(client_thread);
    }

    close(server_socket);
    return 0;
}
