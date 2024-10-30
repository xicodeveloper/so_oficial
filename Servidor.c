#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

// Função para registrar logs do servidor
void escrever_log(const char *mensagem) {
    FILE *f = fopen("log.txt", "a");
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro log.txt\n");
        return;
    }

    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = 0;  // Remove a nova linha do timestamp

    fprintf(f, "%s | %s\n", timestamp, mensagem);
    fclose(f);
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
        } 
        else if (strcmp(token, "ficheiro_solucoes") == 0) {
            token = strtok(NULL, "\n");
            strcpy(ficheiro_solucoes, token);
        } 
        else if (strcmp(token, "porta") == 0) {
            token = strtok(NULL, "\n");
            *porta = atoi(token);
        } 
    }
    fclose(config); 
    escrever_log("Inicio do servidor: Configuração lida com sucesso");
}

// Função para enviar o menu para o cliente
void enviar_menu(int client_socket) {
    const char *menu =
        "---------- Menu de Sudoku ----------\n"
        "1. Inserir.\n"
        "2. O Servidor revela a Solução.\n"
        "3. O Cliente resolve a Solução.\n"
        "4. Desistir.\n"
        "------------------------------------\n"
        "Escolha uma opção: ";
    send(client_socket, menu, strlen(menu), 0);
}

void *handle_client(void *client_socket) {
    int sock = *(int*)client_socket;
    free(client_socket);
    char buffer[BUFFER_SIZE];
    int opcao;
    int client_id;

    // Recebe o ID do cliente
    if (recv(sock, &client_id, sizeof(client_id), 0) <= 0) {
        perror("Erro ao receber ID do cliente");
        close(sock);
        return NULL;
    }
    printf("Novo cliente conectado com ID: %d\n", client_id);
    escrever_log("Novo cliente conectado");

    // Envia o menu apenas uma vez, logo após o cliente se conectar
    enviar_menu(sock);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Cliente %d desconectado\n", client_id);
            escrever_log("Cliente desconectado");
            break;
        }

        buffer[bytes_received] = '\0';
        opcao = atoi(buffer);

        switch (opcao) {
            case 1:
                printf("Cliente %d selecionou inserir um valor no Sudoku\n", client_id);
                strcpy(buffer, "Opção 1: Valor inserido.\n");
                break;
            case 2:
                printf("Cliente %d pediu para revelar a solução.\n", client_id);
                strcpy(buffer, "Opção 2: Solução revelada.\n");
                break;
            case 3:
                printf("Cliente %d resolveu a solução localmente.\n", client_id);
                strcpy(buffer, "Opção 3: Solução resolvida pelo cliente.\n");
                break;
            case 4:
                printf("Cliente %d desistiu do jogo.\n", client_id);
                strcpy(buffer, "Opção 4: Saindo do jogo.\n");
                send(sock, buffer, strlen(buffer), 0);
                goto encerra_conexao;
            default:
                printf("Cliente %d selecionou uma opção inválida.\n", client_id);
                strcpy(buffer, "Opção inválida! Tente novamente.\n");
                break;
        }

        send(sock, buffer, strlen(buffer), 0);
    }

encerra_conexao:
    close(sock);
    printf("Conexão com cliente %d encerrada\n", client_id);
    escrever_log("Conexão com cliente encerrada");
    return NULL;
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    int porta;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    if (argc < 2) {
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }

    char ficheiro_jogos[256];
    char ficheiro_solucoes[256];

    // Ler configuração
    ler_configuracao(argv[1], ficheiro_jogos, ficheiro_solucoes, &porta);
    
    printf("Ficheiro de jogos: %s\n", ficheiro_jogos);
    printf("Ficheiro de soluções: %s\n", ficheiro_solucoes);
    printf("Porta: %d\n", porta);

    // Cria o socket do servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erro ao criar socket");
        escrever_log("Erro ao criar socket do servidor");
        exit(EXIT_FAILURE);
    }

    // Configura o endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(porta);

    // Faz o bind do socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(server_socket);
        escrever_log("Erro no bind do servidor");
        exit(EXIT_FAILURE);
    }

    // Define o servidor para ouvir conexões
    if (listen(server_socket, 10) < 0) {
        perror("Erro no listen");
        close(server_socket);
        escrever_log("Erro no listen do servidor");
        exit(EXIT_FAILURE);
    }

    printf("Servidor aguardando conexões na porta %d...\n", porta);
    escrever_log("Servidor iniciado e aguardando conexões");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Erro no accept");
            continue;
        }

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
