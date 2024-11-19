#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "header.h"
#define BUFFER_SIZE 1024
#define SIZE 9
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
    escrever_log_cliente("Matriz transformada com sucesso string para matriz");
}



void ler_configuracao_cliente(const char *config_path, int *porta, char *ip_server, int *resolvedor) {
    FILE *config = fopen(config_path, "r");
    if (config == NULL) {
        printf("Erro ao abrir o ficheiro de configuração.\n");
        escrever_log_cliente("Erro ao abrir o ficheiro de configuração");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), config)) {
        char *token = strtok(linha, "=");

        if (strcmp(token, "porta") == 0) {
            token = strtok(NULL, "\n");
            *porta = atoi(token);
            escrever_log_cliente("Porta lida com sucesso");
        } else if (strcmp(token, "ip_servidor") == 0) {
            token = strtok(NULL, "\n");
            strcpy(ip_server, token);
            escrever_log_cliente("IP do servidor lido com sucesso");
        }else if(strcmp(token, "resolvedor") == 0){
            token = strtok(NULL, "\n");
            *resolvedor = atoi(token);
            escrever_log_cliente("Resolvedor lida com sucesso");
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
    escrever_log_cliente("ID do cliente lido com sucesso");
    return id;
}

int criar_socket_cliente() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        escrever_log_cliente("Erro no socket do cliente");
        perror("Erro ao criar socket");
        escrever_log_cliente("Erro no socket do cliente");
        exit(EXIT_FAILURE);
    }
    escrever_log_cliente("Socket do cliente criado com sucesso");
    return client_socket;
}

void configurar_endereco_servidor(struct sockaddr_in *server_addr, int porta, const char *ip_server) {
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(porta);
    server_addr->sin_addr.s_addr = inet_addr(ip_server);
    escrever_log_cliente("Endereço do servidor configurado com sucesso");
}

void conectar_servidor(int client_socket, struct sockaddr_in *server_addr) {
    if (connect(client_socket, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        escrever_log_cliente("Erro ao conectar ao servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    escrever_log_cliente("Cliente conectado ao servidor");
}

void enviar_id_cliente(int client_socket, int client_id) {
    if (send(client_socket, &client_id, sizeof(client_id), 0) < 0) {
        perror("Erro ao enviar ID do cliente");
        escrever_log_cliente("Erro ao enviar ID do cliente");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    escrever_log_cliente("ID do cliente enviado com sucesso");
}

void imprima_matriz(int matriz[9][9]){
for(int i=0;i<9;i++){
    if(i%3==0 && i!=0){
        printf("---------------------\n");
    }
    for (int j = 0; j < 9; j++)
    {
        if(j%3==0 && j!=0){
            printf("| ");
        }
        printf("%d ", matriz[i][j]);
    }
    printf("\n");
}
escrever_log_cliente("Matriz impressa com sucesso");
}
int escolhe_celula_sem_nada_aleatoria(int matriz[9][9], int *linha, int *coluna) {
    int vazias[81][2];
    int count = 0;

    // Lista todas as posições vazias
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (matriz[i][j] == 0) { // Considera 0 como vazio
                vazias[count][0] = i;
                vazias[count][1] = j;
                count++;
            }
        }
    }

    if (count == 0) return 0; // Não há posições vazias

    // Escolhe uma posição aleatória dentre as vazias
    int indice = rand() % count;
    *linha = vazias[indice][0];
    *coluna = vazias[indice][1];
    escrever_log_cliente("Célula sem nada escolhida com sucesso");
    return 1;
}
int escolhe_celula_com_algo(int matriz[9][9], int *linha, int *coluna) {
    int vazias[81][2];
    int count = 0;

    // Lista todas as posições vazias
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (matriz[i][j] != 0) { // Considera 0 como vazio
                vazias[count][0] = i;
                vazias[count][1] = j;
                count++;
            }
        }
    }

    if (count == 81) return 0; // já nao há posições ocupadas

    // Escolhe uma posição aleatória dentre as vazias
    int indice = rand() % count;
    *linha = vazias[indice][0];
    *coluna = vazias[indice][1];
    escrever_log_cliente("Célula com algo escolhida com sucesso");
    return 1;
}

int numCelulasVazias(int matriz[9][9]){
    int count = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (matriz[i][j] == 0) { // Considera 0 como vazio
                count++;
            }
        }
    }
    return count;
}

void envia_tentativa(int client_socket, int num, int linha, int coluna) {
    int tentativa = (rand() % 9) + 1;
    char buffer[BUFFER_SIZE];

    printf("[DEBUG] Tentativa gerada: %d\n", tentativa);
    snprintf(buffer, BUFFER_SIZE, "%d %d %d %d", num, linha, coluna, tentativa);

    printf("[DEBUG] Enviando mensagem para o servidor: '%s'\n", buffer);
    if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
        perror("[ERRO] Falha ao enviar tentativa para o servidor");
        escrever_log_cliente("Tentativa não enviada com sucesso");
        return;
    }
    escrever_log_cliente("Tentativa enviada com sucesso");
    printf("[DEBUG] Tentativa enviada com sucesso!\n");
}

void envia_tentativa_apaga(int client_socket, int num, int linha, int coluna) {
    char buffer[BUFFER_SIZE];

    // Formata a mensagem para envio com os valores para apagar (eliminação)
    snprintf(buffer, BUFFER_SIZE, "%d,%d,%d", num, linha, coluna);

    printf("[DEBUG] Enviando solicitação para eliminar ao servidor: '%s'\n", buffer);

    // Envia os dados para o servidor
    if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
        perror("[ERRO] Falha ao enviar solicitação ao servidor");
        escrever_log_cliente("Solicitação de apagar não enviada com sucesso");
        return;
    }

    escrever_log_cliente("Solicitação de apagar enviada com sucesso");
    printf("[DEBUG] Solicitação de apagar enviada com sucesso!\n");
}

void processa_feedback(const char *feedback) {
    // Variável local para armazenar o valor temporário de casas vazias

    // Verifica se a tentativa está correta ou errada
    if (strstr(feedback, "correta") != NULL) {
        escrever_log_cliente("Tentativa correta");
    } else if (strstr(feedback, "errada") != NULL) {
        escrever_log_cliente("Tentativa incorreta");
    } else {
        printf("[ERRO] Feedback não reconhecido: '%s'\n", feedback);
        escrever_log_cliente("Feedback não reconhecido");
        return; // Sai da função em caso de erro
    }

}
void processa_feedback_apaga(const char *feedback) {
    // Variável local para armazenar o valor temporário de casas vazias

    // Verifica se a tentativa está correta ou errada
    if (strstr(feedback, "Apagado_ja") != NULL) {
        escrever_log_cliente("Já está apagado");
    } else if (strstr(feedback, "Limpar") != NULL) {
        escrever_log_cliente("Célula apagada com sucesso");
    } else {
        printf("[ERRO] Feedback não reconhecido: '%s'\n", feedback);
        escrever_log_cliente("Feedback não reconhecido");
        return; // Sai da função em caso de erro
    }

}

void feedback_Apaga(int client_socket) {
    char buffer[BUFFER_SIZE];

    escrever_log_cliente("Aguardando feedback do servidor");
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        escrever_log_cliente("Feedback do servidor recebido");
        printf("[DEBUG] Feedback do servidor recebido: '%s'\n", buffer);
    } else {
        escrever_log_cliente("Falha ao receber feedback do servidor");
        perror("[ERRO] Falha ao receber feedback do servidor");
    }
    printf("Resposta do servidor:\n%s\n", buffer);
    processa_feedback_apaga(buffer);
}

void recebe_feedback_tentativa(int client_socket) {
    char buffer[BUFFER_SIZE];

    escrever_log_cliente("Aguardando feedback do servidor");
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        escrever_log_cliente("Feedback do servidor recebido");
        printf("[DEBUG] Feedback do servidor recebido: '%s'\n", buffer);
    } else {
        escrever_log_cliente("Falha ao receber feedback do servidor");
        perror("[ERRO] Falha ao receber feedback do servidor");
    }
    printf("Resposta do servidor:\n%s\n", buffer);
    processa_feedback(buffer);
    send(client_socket, "Feedback recebido", 17, 0);
}

void recebe_Tabuleiro(int client_socket, int matriz[9][9]) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received <= 0) {
        printf("Servidor desconectado.\n");
        escrever_log_cliente(" Cliente não recebeu tabuleiro");
        return;
    }
    escrever_log_cliente("TabuleirO recebido com sucesso");
    send(client_socket, "Tabuleiro recebido\0", 19, 0);
    escrever_log_cliente("Confirmação de tabuleiro enviada");
    buffer[bytes_received] = '\0'; // Corrigido para usar buffer
    string_para_matriz(buffer, matriz);

}

void comunicar_servidor(int client_socket, int resolvedor) {
    char buffer[BUFFER_SIZE];
    int matriz[SIZE][SIZE] = {0}; // Inicializa a matriz com zeros
    int id_tabuleiro;
    int linha_branca;
    int coluna_branca;
    int (*total_vazias_ptr);
    if (resolvedor == 1) {
        if (recv(client_socket, &id_tabuleiro, sizeof(id_tabuleiro), 0) <= 0) {
            escrever_log_cliente("Erro ao receber ID do tabuleiro");
            perror("Erro ao receber ID do tabuleiro");
            close(client_socket);
            return;
        }
        escrever_log_cliente("ID do tabuleiro recebido com sucesso");
        printf("Id recebido %d \n", id_tabuleiro);

        // Inicia o loop para enviar e receber respostas
        while (1) {
            // solicita Tabuleiro
            // ____________________________________
            strcpy(buffer, "Solicita Tabuleiro");
            send(client_socket, buffer, strlen(buffer), 0);
            escrever_log_cliente("Solicitação de tabuleiro enviada");
            
            recebe_Tabuleiro(client_socket, matriz);
            imprima_matriz(matriz);

            // _______________________________________________
            // Recebe o menu inicial do servidor
            printf("Recebe menu inicial\n");
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            escrever_log_cliente("Menu inicial recebido");
            printf("Depois de receber menu inicial\n");
            if (bytes_received <= 0) {
                escrever_log_cliente("Erro ao receber menu inicial");
                printf("Servidor desconectado.\n");
                return;
            }
            printf("bytes_received: %d\n", bytes_received);
            buffer[bytes_received] = '\0';
            printf("Resposta do servidor:\n%s", buffer);

            printf("Insira um número ('5' para encerrar): ");
            escrever_log_cliente("Esperando opcao do cliente");
            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);

            if (strlen(buffer) == 1) {
                printf("Por favor, insira um número válido.\n");
                continue;
            }

            // Remove o newline que `fgets` deixa no buffer
            buffer[strcspn(buffer, "\n")] = 0;

            int resposta = atoi(buffer); // Converte a entrada para inteiro

            // Envia a opção para o servidor
            if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
                escrever_log_cliente("Erro ao enviar opção para o servidor");
                perror("Erro ao enviar dados");
                return;
            }
            escrever_log_cliente("Opção enviada com sucesso");
            switch (resposta) {
                case 1:
                    // Determina o ponteiro para a variável global de casas vazias
                    // Determina o ponteiro para a variável global de casas vazias

                    printf("[DEBUG] Opção 1: Resolver o tabuleiro completo selecionada.\n");

                    escrever_log_cliente("Opção 1: Resolver o tabuleiro completo selecionada");
                    // Recebe confirmação do servidor
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        buffer[bytes_received] = '\0';
                        printf("Resposta do servidor:\n%s", buffer);
                    }
                    else {
                            escrever_log_cliente("Erro ao receber resposta do servidor");
                            perror("[ERRO] Falha ao receber resposta do servidor");
                            return;
                    }    
                    escrever_log_cliente("Confirmação de opção recebida");
                    int total_vazias = numCelulasVazias(matriz);

                    while (total_vazias > 0) {

                        printf("Número total de casas vazias: %d\n", total_vazias);
                        escrever_log_cliente("Número total de casas vazias recebido com sucesso");
                        if (escolhe_celula_sem_nada_aleatoria(matriz, &linha_branca, &coluna_branca)) {
                            printf("Posição vazia encontrada em: linha %d, coluna %d\n", linha_branca, coluna_branca);
                            printf("O ID do tabuleiro é: %d\n", id_tabuleiro);

                            // Envia tentativa ao servidor
                            envia_tentativa(client_socket, id_tabuleiro, linha_branca, coluna_branca);  //send
                            // Recebe feedback do servidor
                            printf("[DEBUG] Tentativa enviada. Aguardando feedback do servidor...\n");

                            recebe_feedback_tentativa(client_socket);  //recv - send
                        } else {
                            escrever_log_cliente("Nenhuma posição vazia encontrada. Sudoku resolvido!");
                            printf("Nenhuma posição vazia encontrada. Sudoku resolvido!\n");
                            break;
                        }
                        
                        recebe_Tabuleiro(client_socket, matriz); //recv - send
                        imprima_matriz(matriz);
                        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0); // recv

                        total_vazias = numCelulasVazias(matriz);

                    }
                    printf("[INFO] Tabuleiro %d resolvido!\n", id_tabuleiro);
                    escrever_log_cliente("Tabuleiro resolvido com sucesso");
                    break;

                case 2:
                    printf("[DEBUG] Opção 2: Solicitar solução parcial selecionada.\n");
                    escrever_log_cliente("Opção 2: Solicitar solução parcial selecionada");
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        buffer[bytes_received] = '\0';
                        escrever_log_cliente("Resposta do servidor recebida da opcao 2");
                        printf("Resposta do servidor:\n%s", buffer);
                        printf("Quantas tentativas deseja fazer?\n");
                        escrever_log_cliente("Pedir tentativas ao cliente.");
                        memset(buffer, 0, BUFFER_SIZE);
                        fgets(buffer, BUFFER_SIZE, stdin);

                        // Remove o newline que `fgets` deixa no buffer
                        buffer[strcspn(buffer, "\n")] = 0;

                        int tentativa = atoi(buffer); // Converte a entrada para inteiro

                        if (send(client_socket, &tentativa, sizeof(tentativa), 0) < 0) {
                            escrever_log_cliente("Erro ao enviar número de tentativas");
                            perror("Erro ao enviar número de tentativas");
                            close(client_socket);
                            exit(EXIT_FAILURE);
                        }
                        escrever_log_cliente("Número de tentativas enviadas com sucesso");
                        // Enquanto houver casas vazias, envia tentativas e recebe feedback
                        while (tentativa != 0) {
                            if (escolhe_celula_sem_nada_aleatoria(matriz, &linha_branca, &coluna_branca)) {
                                printf("Posição vazia encontrada em: linha %d, coluna %d\n", linha_branca, coluna_branca);
                                printf("O ID do tabuleiro é: %d\n", id_tabuleiro);

                                // Envia tentativa ao servidor
                                envia_tentativa(client_socket, id_tabuleiro, linha_branca, coluna_branca); // send 

                                // Recebe feedback do servidor
                                printf("[DEBUG] Tentativa enviada. Aguardando feedback do servidor...\n");
                                recebe_feedback_tentativa(client_socket);
                                

                            } else {
                                escrever_log_cliente("Nenhuma posição vazia encontrada. Sudoku resolvido!");
                                printf("Nenhuma posição vazia encontrada. Sudoku resolvido!\n");
                                break;
                            }
                            tentativa--;
                        }
                    } else {
                        escrever_log_cliente("Erro ao receber resposta do servidor da opcocao 2");
                        perror("[ERRO] Falha ao receber resposta do servidor");
                        return;
                    }
                    break;
                case 3:
                    printf("[DEBUG] Opção 3: Solicitar solução parcial selecionada.\n");
                    escrever_log_cliente("Opção 3: Solicitar solução parcial selecionada");
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        escrever_log_cliente("Resposta do servidor recebida da opcao 3");
                        buffer[bytes_received] = '\0';
                        printf("Resposta do servidor: %s", buffer);
                        send(client_socket, "Resposta do servidor confirmada", 25, 0);
                        recebe_Tabuleiro(client_socket, matriz);
                        imprima_matriz(matriz);
                    } else {
                        perror("[ERRO] Falha ao receber resposta do servidor");
                        escrever_log_cliente("Erro ao receber resposta do servidor da opcao 3");
                        return;
                    }
                    break;

                case 4:
                    printf("[DEBUG] Opção 4: Solicitar solução parcial selecionada.\n");
                    escrever_log_cliente("Opção 4: Solicitar solução parcial selecionada");
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        escrever_log_cliente("Resposta do servidor recebida da opcao 4");
                        buffer[bytes_received] = '\0';
                        printf("Resposta do servidor:\n%s", buffer);
                    } else {
                        escrever_log_cliente("Erro ao receber resposta do servidor da opcao 4");
                        perror("[ERRO] Falha ao receber resposta do servidor");
                        return;
                    }
                    break;

                case 5:
                    printf("[DEBUG] Opção 5: Desistir do Jogo.\n");
                    escrever_log_cliente("Opção 5: Desistir do Jogo");
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        escrever_log_cliente("Resposta do servidor recebida da opcao 5");
                        buffer[bytes_received] = '\0';
                        printf("Resposta do servidor:\n%s", buffer);
                    } else {
                        escrever_log_cliente("Erro ao receber resposta do servidor da opcao 5");
                        perror("[ERRO] Falha ao receber resposta do servidor");
                    }
                    return;
                    break; // Sai do loop principal no cliente

                default:
                    printf("[ERRO] Opção inválida! Tente novamente.\n");
                    escrever_log_cliente("Opção inválida! Tente novamente");
                    printf("[DEBUG] opção escolhida: %d\n", resposta);
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        escrever_log_cliente("Resposta do servidor recebida da opcao invalida");
                        buffer[bytes_received] = '\0';
                        printf("Resposta do servidor:\n%s", buffer);
                    } else {
                        escrever_log_cliente("Erro ao receber resposta do servidor da opcao invalida");
                        perror("[ERRO] Falha ao receber resposta do servidor");
                    }
                    printf("[DEBUG] opção escolhida: %d\n", resposta);
                    break;
            }
        }
    }else{//apagador
        int linha_ocupada, coluna_ocupada;
    
        if (recv(client_socket, &id_tabuleiro, sizeof(id_tabuleiro), 0) <= 0) {
            escrever_log_cliente("Erro ao receber ID do tabuleiro");
            perror("Erro ao receber ID do tabuleiro");
            close(client_socket);
            return;
        }
        escrever_log_cliente("ID do tabuleiro recebido com sucesso");
        printf("Id recebido %d", id_tabuleiro);
    
        // Inicia o loop para enviar e receber respostas
        while (1) {
            //solicita Tabuleiro
            //____________________________________
            strcpy(buffer, "Solicita Tabuleiro");
            send(client_socket, buffer, strlen(buffer), 0);
            escrever_log_cliente("Solicitação de tabuleiro enviada");
            // Recebe o tabuleiro do servidor
            int bytes_received2 = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received2 <= 0) {
                escrever_log_cliente("Erro ao receber tabuleiro");
                printf("Servidor desconectado.\n");
                return;
            }
            escrever_log_cliente("Tabuleiro recebido com sucesso");
            buffer[bytes_received2] = '\0'; // Corrigido para usar buffer
            printf("\nTabuleiro enviado:\n%s\n", buffer);
            send(client_socket, "Tabuleiro recebido\0", 19, 0);
            escrever_log_cliente("Confirmação de tabuleiro enviada");
            // Converte a string para a matriz
            string_para_matriz(buffer, matriz);
            printf("Matriz transformada:\n");
            imprima_matriz(matriz);
    
            //_______________________________________________
            // Recebe o menu inicial do servidor
            printf("Recebe menu inicial\n");
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            escrever_log_cliente("Menu inicial recebido");
            printf("Depois de receber menu inicial\n");
            if (bytes_received <= 0) {
                escrever_log_cliente("Erro ao receber menu inicial");
                printf("Servidor desconectado.\n");
                return;
            }
            printf("bytes_received: %d\n", bytes_received);
            buffer[bytes_received] = '\0';
            printf("Resposta do servidor:\n%s", buffer);
    
            printf("Insira um número ('5' para encerrar): ");
            escrever_log_cliente("Esperando opcao do cliente");
            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);
    
            if (strlen(buffer) == 1) {
                printf("Por favor, insira um número válido.\n");
                continue;
            }
    
            // Remove o newline que `fgets` deixa no buffer
            buffer[strcspn(buffer, "\n")] = 0;
    
            int resposta = atoi(buffer); // Converte a entrada para inteiro
    
            // Envia a opção para o servidor
            if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
                escrever_log_cliente("Erro ao enviar opção para o servidor");
                perror("Erro ao enviar dados");
                return;
            }
            escrever_log_cliente("Opção enviada com sucesso");
            switch (resposta) {
                case 1:
                    // Determina o ponteiro para a variável global de casas vazias
                    if (id_tabuleiro == 1) {
                        total_vazias_ptr = &zero_1;
                    } else if (id_tabuleiro == 2) {
                        total_vazias_ptr = &zero_2;
                    } else if (id_tabuleiro == 3) {
                        total_vazias_ptr = &zero_3;
                    } else if (id_tabuleiro == 4) {
                        total_vazias_ptr = &zero_4;
                    }
    
                    printf("[DEBUG] Opção 1: Apagar o tabuleiro completo selecionada.\n");
                    escrever_log_cliente("Opção 1: Apagar o tabuleiro completo selecionada");
                    // Recebe confirmação do servidor
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    escrever_log_cliente("Confirmação de tabuleiro recebida");
                    if (bytes_received > 0) {
                        escrever_log_cliente("Tabuleiro recebido com sucesso");
                        buffer[bytes_received] = '\0';
                        printf("Resposta do servidor:\n%s", buffer);
    
                        while ((*total_vazias_ptr) != 0) {
                            printf("Número total de casas vazias: %d\n", *total_vazias_ptr);
                            escrever_log_cliente("Número total de casas vazias recebido com sucesso");
                            if (escolhe_celula_com_algo(matriz, &linha_ocupada, &coluna_ocupada)) {
                                printf("Posição ocupada encontrada em: linha %d, coluna %d\n", linha_ocupada, coluna_ocupada);
                                printf("O ID do tabuleiro é: %d\n", id_tabuleiro);
    
                                // Envia tentativa ao servidor
                                envia_tentativa_apaga(client_socket, id_tabuleiro, linha_ocupada, coluna_ocupada);
    
                                // Recebe feedback do servidor
                                printf("[DEBUG] Tentativa enviada. Aguardando feedback do servidor...\n");
                                feedback_Apaga(client_socket);
    
                            } else {
                                escrever_log_cliente("Nenhuma posição ocupada encontrada. Sudoku Apagado!");
                                printf("Nenhuma posição ocupada encontrada. Sudoku Apagado!\n");
                                break;
                            }
                        }
                    } else {
                        escrever_log_cliente("Erro ao receber resposta do servidor");
                        perror("[ERRO] Falha ao receber resposta do servidor");
                        return;
                    }
                    printf("[INFO] Tabuleiro %d Apagado!\n", id_tabuleiro);
                    escrever_log_cliente("Tabuleiro apagado com sucesso");
                    break;
    
                case 2:
                    printf("[DEBUG] Opção 2: Solicitar solução parcial selecionada.\n");
                    escrever_log_cliente("Opção 2: Solicitar solução parcial selecionada");
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        buffer[bytes_received] = '\0';
                        escrever_log_cliente("Resposta do servidor recebida da opcao 2");
                        printf("Resposta do servidor:\n%s", buffer);
                        printf("Quantas tentativas deseja fazer?\n");
                        escrever_log_cliente("Pedir tentativas ao cliente.");
                        memset(buffer, 0, BUFFER_SIZE);
                        fgets(buffer, BUFFER_SIZE, stdin);
    
                        // Remove o newline que `fgets` deixa no buffer
                        buffer[strcspn(buffer, "\n")] = 0;
    
                        int tentativa = atoi(buffer); // Converte a entrada para inteiro
    
                        if (send(client_socket, &tentativa, sizeof(tentativa), 0) < 0) {
                            escrever_log_cliente("Erro ao enviar número de tentativas");
                            perror("Erro ao enviar número de tentativas");
                            close(client_socket);
                            exit(EXIT_FAILURE);
                        }
                        escrever_log_cliente("Número de tentativas enviadas com sucesso");
                        // Enquanto houver casas vazias, envia tentativas e recebe feedback
                        while (tentativa != 0) {
                            if (escolhe_celula_com_algo(matriz, &linha_ocupada, &coluna_ocupada)) {
                                printf("Posição vazia encontrada em: linha %d, coluna %d\n", linha_ocupada, coluna_ocupada);
                                printf("O ID do tabuleiro é: %d\n", id_tabuleiro);
    
                                // Envia tentativa ao servidor
                                envia_tentativa_apaga(client_socket, id_tabuleiro, linha_ocupada, coluna_ocupada);
    
                                // Recebe feedback do servidor
                                printf("[DEBUG] Tentativa enviada. Aguardando feedback do servidor...\n");
                                feedback_Apaga(client_socket);
    
                            } else {
                                escrever_log_cliente("Nenhuma posição ocupada encontrada. Sudoku Apagado!");
                                printf("Nenhuma posição ocupada encontrada. Sudoku Apagado!\n");
                                break;
                            }
                            tentativa--;
                        }
                    } else {
                        escrever_log_cliente("Erro ao receber resposta do servidor da opcocao 2");
                        perror("[ERRO] Falha ao receber resposta do servidor");
                        return;
                    }
                    break;
                default:
                    printf("[ERRO] Opção inválida! Tente novamente.\n");
                    escrever_log_cliente("Opção inválida! Tente novamente");
                    printf("[DEBUG] opção escolhida: %d\n", resposta);
                    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received > 0) {
                        escrever_log_cliente("Resposta do servidor recebida da opcao invalida");
                        buffer[bytes_received] = '\0';
                        printf("Resposta do servidor:\n%s", buffer);
                    } else {
                        escrever_log_cliente("Erro ao receber resposta do servidor da opcao invalida");
                        perror("[ERRO] Falha ao receber resposta do servidor");
                    }
                    printf("[DEBUG] opção escolhida: %d\n", resposta);
                    break;
            }
        }
    }
}    

int main(int argc, char *argv[]) {
    int client_socket;
    int porta;
    char ip[256];
    int resolvedor;
    if (argc < 2) {
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }

    int client_id = get_new_user_id();

    ler_configuracao_cliente(argv[1], &porta, ip, &resolvedor);
    if(resolvedor==1){
        printf("Resolvedor\n");
    }else{

        printf("Apagador\n");
    }
    struct sockaddr_in server_addr;

    client_socket = criar_socket_cliente();

    configurar_endereco_servidor(&server_addr, porta, ip);

    conectar_servidor(client_socket, &server_addr);

    printf("Cliente %d conectado ao servidor\n", client_id);
   if (send(client_socket, &resolvedor, sizeof(resolvedor), 0) < 0) {
        perror("Erro ao enviar resolvedor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    enviar_id_cliente(client_socket, client_id);
    comunicar_servidor(client_socket, resolvedor);
    close(client_socket);
    escrever_log_cliente("Cliente desconectado");
    return 0;
}