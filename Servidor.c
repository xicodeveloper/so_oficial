#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include "header.h"

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
pthread_mutex_t clients_mutex_board_2 = PTHREAD_MUTEX_INITIALIZER;

/**
 *  Função para registrar logs do servidor incluindo a data e hora do evento
 * @param mensagem Mensagem a ser registrada no log
 * @return void
*/  
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

/**
 * Função para ler o ficheiro de configuração e carregar as configurações
 * @param config_path Caminho para o ficheiro de configuração
 * @param ficheiro_jogos Ponteiro para armazenar o caminho do ficheiro de jogos
 * @param ficheiro_solucoes Ponteiro para armazenar o caminho do ficheiro de soluções
 * @param porta Ponteiro para armazenar a porta do servidor
 * @return void
 */
void ler_configuracao(char *config_path, char *ficheiro_jogos, char *ficheiro_solucoes, int *porta) {
    FILE *config = fopen(config_path, "r");
    escrever_log("Inicio do servidor: Leitura do ficheiro de configuração");
    if (config == NULL) {
        escrever_log("Erro ao abrir o ficheiro de configuração");
        printf("Erro ao abrir o ficheiro de configuração.\n");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), config)) {
        char *token = strtok(linha, "=");

        if (strcmp(token, "ficheiro_jogos") == 0) {
            token = strtok(NULL, "\n");
            strcpy(ficheiro_jogos, token);
            escrever_log("Ficheiro de jogos lido com sucesso");
        } else if (strcmp(token, "ficheiro_solucoes") == 0) {
            token = strtok(NULL, "\n");
            strcpy(ficheiro_solucoes, token);
            escrever_log("Ficheiro de soluções lido com sucesso");
        } else if (strcmp(token, "porta") == 0) {
            token = strtok(NULL, "\n");
            *porta = atoi(token);
            escrever_log("Porta lida com sucesso");
        }
    }
    fclose(config);
    escrever_log("Inicio do servidor: Configuração lida com sucesso");
    printf("Configuração carregada com sucesso\n"); // Debug
}



/**
 * Função para transformar o ficheiro de solucoes.txt numa matriz 3D
 * @param ficheiro_jogos Caminho para o ficheiro de jogos
 * @param matriz_of Matriz 3D para armazenar os jogos
 * @return void
 */
void transforma_matriz_solucao(char *ficheiro_solucoes, int matriz_of[SIZE][LC][LC]) {
    char buffer[BUFFER_SIZE];
    ficheiro_solucoes[strcspn(ficheiro_solucoes, "\r")] = 0;

    FILE *f = fopen(ficheiro_solucoes, "r");
    escrever_log("Inicio da transformação da matriz solução, leitura soluções.txt");
    printf("Inicio da transformação da matriz solução, leitura %s\n", ficheiro_solucoes);    
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro das soluçoes jogos para leitura.\n");
        escrever_log("Erro ao abrir o ficheiro dos jogos para leitura");
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
                    matriz_solucao[jogo_index][i][j] = 0; // Usar 0 para posições desconhecidas
                } else {
                    matriz_solucao[jogo_index][i][j] = buffer[k] - '0'; // Converter caractere para inteiro
                }
                k++; // Avançar para o próximo caractere do buffer
            }
        }
        jogo_index++; // Próximo jogo
    }
    
    fclose(f);
    escrever_log("Matriz solução transformada com sucesso de soluçoes.txt para array");
}

/**
 * Função para transformar o ficheiro de jogos.txt numa matriz 3D
 * @param ficheiro_jogos Caminho para o ficheiro de jogos
 * @param matriz_of Matriz 3D para armazenar os jogos
 * @return void
 */
void transforma_matriz(char *ficheiro_jogos,int matriz_of[SIZE][LC][LC]) {
    char buffer[BUFFER_SIZE];
    ficheiro_jogos[strcspn(ficheiro_jogos, "\r")] = 0;
    FILE *f = fopen(ficheiro_jogos, "r");
    escrever_log("Inicio da transformação da matriz, leitura jogos.txt");
    if (f == NULL) {
        escrever_log("Erro ao abrir o ficheiro dos jogos para leitura");
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
    escrever_log("Matriz transformada com sucesso de jogos.txt para array");
    return; // Retornar a quantidade de jogos lidos
}


/**
 * Função para escrever no terminal as matrizes de jogos atuais
 * @param matriz_of Matriz 3D com os jogos
 * @return void
 */
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
    escrever_log("Matrizes lidas com sucesso");
}


/**
 * Função para escrever no terminal uma matriz de jogos específica mediante o ID
 * @param matriz_of Matriz 3D com os jogos
 * @param num ID do jogo
 * @return void
 */
void ler_matrizes_id(int matriz_of[SIZE][LC][LC], int num) {

        printf("Jogo %d:\n", num);
        for (int j = 0; j < LC; j++) {
            for (int l = 0; l < LC; l++) {
                printf("%d ", matriz_of[num-1][j][l]);
            }
            printf("\n");  // Nova linha após cada linha da matriz
        }
        printf("\n");  // Linha em branco entre jogos
        escrever_log("Matrizes lidas com sucesso");
}


/**
 * Função para formatar o tabuleiro de 81 caracteres em um tabuleiro 9x9 com os devidos espaços es barras
 * @param tabuleiro Tabuleiro de 81 caracteres
 * @param formatted_board Tabuleiro formatado resultante
 * @return void   
 */
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
    escrever_log("Tabuleiro formatado com sucesso");
}


/**
 * Função auxiliar para converter o tabuleiro 2D em uma string unidimensional
 * @param tabuleiro Tabuleiro 2D
 * @param tabuleiro_str String unidimensional para armazenar o tabuleiro
 */
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
    escrever_log("Tabuleiro convertido para string com sucesso");
}


/** 
 * Função para calcular o número total de células vazias num tabuleiro específico
 * @param matriz Matriz 3D com os jogos
 * @param num ID do jogo
 * @return int Número total de células vazias
 */
int numero_total_vazias(int matriz[4][9][9], int num) {
    int total_vazias = 0;
    // Percorrer a matriz e registrar coordenadas de células vazias
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
                if (matriz[num-1][i][j] == 0) {
                    total_vazias++;
                }
        }
    }

    return total_vazias; 
}

/**
 * Função para lidar com o sinal SIGINT (Ctrl + C)
 * @param sig Sinal recebido
 * @return void
 */
void handle_sigint(int sig) {
    printf("\nSIGINT received. Closing server socket...\n");
    close(server_socket);
    exit(0);
}



/**
 * Função para enviar o menu para o cliente Resolvedor
 * 
 * socket: send
 * 
 * @param client_socket Socket do cliente
 * @return void
 * 
 */
void enviar_menu_resolvedor(int client_socket) {
    const char *menu =
        "---------- Menu de Sudoku ----------\n"
        "1. Resolver Tabuleiro Total.\n"
        "2. O Servidor revela a Solução e Desistir.\n"
        "3. Desistir.\n"
        "------------------------------------\n";
    printf("Enviando menu para o cliente\n"); // Debug
    send(client_socket, menu, strlen(menu), 0);
    escrever_log("Menu enviado com sucesso");
}

/**
 * Função para enviar o menu para o cliente Apagador
 * 
 * socket: send
 * 
 * @param client_socket Socket do cliente
 * @return void
 */
void enviar_menu_apagador(int client_socket) {
    const char *menu =
        "---------- Menu de Sudoku ----------\n"
        "1. Apagar uma a uma as Celulas do Tabuleiro.\n"
        "2. Desistir.\n"
        "------------------------------------\n";
    printf("Enviando menu para o cliente\n"); // Debug
    send(client_socket, menu, strlen(menu), 0);
    escrever_log("Menu enviado com sucesso");
}

/**
 * Função para escolher e enviar o tabuleiro para o cliente
 * 
 * socket: send - recv
 * 
 * @param client_socket Socket do cliente
 * @param num ID do tabuleiro
 * @param matriz Matriz 3D com os jogos
 * @return void
 */
void envia_tabuleiro(int client_socket, int num, int matriz[SIZE][LC][LC]) {
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
    escrever_log("Tabuleiro enviado com sucesso");
    // Recebe a confirmação de que o cliente recebeu o tabuleiro
    if (recv(client_socket, received_message, BUFFER_SIZE - 1, 0) > 0) {
        received_message[BUFFER_SIZE - 1] = '\0'; // Assegura-se de que a mensagem recebida seja terminada com '\0'
        if (strcmp(received_message, "Tabuleiro recebido") == 0) {
            printf("Tabuleiro recebido pelo cliente\n");
            escrever_log("Tabuleiro recebido pelo cliente");
        } else {
            printf("Erro ao receber tabuleiro: %s\n", received_message);
            escrever_log("Erro ao receber tabuleiro");
        }
    } else {
        printf("Erro na recepção da confirmação do cliente\n");
        escrever_log("Erro na recepção da confirmação do cliente");
    }
}

/**
 *  Função para enviar solução para o cliente
 * 
 * Socket: send
 * 
 * @param client_socket Socket do cliente
 * @param num ID do tabuleiro
 * 
 */
void envia_solucao(int client_socket, int num) {
    char buffer[BUFFER_SIZE];
    char formatted_tabuleiro[BUFFER_SIZE];

    converter_tabuleiro_para_string(matriz_solucao[num - 1], buffer); // Converte para string unidimensional
    formatar_tabuleiro(buffer, formatted_tabuleiro);
    send(client_socket, formatted_tabuleiro, strlen(formatted_tabuleiro), 0);
    
    escrever_log("Solução enviada com sucesso");
}

/**
 * Função para enviar o id do tabuleiro para o cliente
 * 
 * Socket: send
 * 
 * @param client_socket Socket do cliente
 * @param num ID do tabuleiro
 * @return void
 */
void enviar_id_tabuleiro(int client_socket, int num) {

    if (send(client_socket, &num, sizeof(num), 0) < 0) {

        perror("Erro ao enviar ID do tabuleiro");
        escrever_log("Erro ao enviar ID do tabuleiro");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    escrever_log("Enviar ID do tabuleiro ao cliente");

}

/**
 * Função para apagar a informação da célula que o cliente escolheu
 * 
 * Socket: recv - send
 * 
 * @param client_socket Socket do cliente
 * @param matriz_of Matriz 3D com os jogos
 * @return void
 */
void recebe_apaga_celula(int client_socket, int matriz_of[4][9][9]) {
    char buffer[BUFFER_SIZE];
    int num, linha, coluna;

    printf("[DEBUG] Aguardando solicitação do cliente...\n");
    escrever_log("Recebendo solicitação do cliente");

    // Recebe mensagem
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        escrever_log("Erro ao receber dados do cliente");
        perror("[ERRO] Falha no recv");
        return;
    }

    buffer[bytes_received] = '\0'; // Assegura que a string é válida
    escrever_log("Solicitação recebida com sucesso");
    printf("[DEBUG] Mensagem recebida: '%s'\n", buffer);

    // Processa manualmente a mensagem (sem sscanf)
    char *token = strtok(buffer, ",");
    if (!token || (num = atoi(token)) < 1 || num > 4) {
        printf("[ERRO] ID de tabuleiro inválido\n");
        escrever_log("ID de tabuleiro inválido recebido");
        return;
    }

    token = strtok(NULL, ",");
    if (!token || (linha = atoi(token)) < 0 || linha >= 9) {
        printf("[ERRO] Linha inválida\n");
        escrever_log("Linha inválida recebida");
        return;
    }

    token = strtok(NULL, ",");
    if (!token || (coluna = atoi(token)) < 0 || coluna >= 9) {
        printf("[ERRO] Coluna inválida\n");
        escrever_log("Coluna inválida recebida");
        return;
    }

    printf("[DEBUG] Dados extraídos: Tabuleiro ID=%d, Linha=%d, Coluna=%d\n", num, linha, coluna);


    // Processa solicitação
    char resposta[BUFFER_SIZE];
    if (matriz_of[num - 1][linha][coluna] != 0) {
        matriz_of[num - 1][linha][coluna] = 0;

        snprintf(resposta, BUFFER_SIZE, "OK: Posição (%d, %d) apagada com sucesso.", linha+1, coluna+1);
        escrever_log("Valor apagado com sucesso");
        ler_matrizes_id(matriz_of, num);
    } else {
        snprintf(resposta, BUFFER_SIZE, "ERRO: Posição (%d, %d) já está vazia.", linha+1, coluna+1);
        escrever_log("Tentativa de apagar posição já vazia");
    }

    // Envia resposta
    printf("[DEBUG] Feedback gerado: '%s'\n", resposta);
    if (send(client_socket, resposta, strlen(resposta), 0) < 0) {
        perror("[ERRO] Falha ao enviar feedback ao cliente");
        escrever_log("Erro ao enviar feedback ao cliente");
    } else {
        printf("[DEBUG] Feedback enviado com sucesso\n");
        escrever_log("Feedback enviado com sucesso");
    }
}

/**
 * Função para receber a tentativa do cliente e enviar o feedback
 * 
 * Socket: recv - send
 * 
 * @param client_socket Socket do cliente
 * @param matriz_sol Matriz 3D com as soluções
 * @param matriz_of Matriz 3D com os jogos
 * @return void
 */
void recebe_tentativa_e_envia_feedback(int client_socket, int matriz_sol[4][9][9], int matriz_of[4][9][9]) {
    char buffer[BUFFER_SIZE];
    int num, linha, coluna, tentativa;

    printf("[DEBUG] Aguardando mensagem do cliente para receber tentativa...\n");
    escrever_log("Recebendo tentativa do cliente");
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        escrever_log("Erro ao receber tentativa do cliente");
        perror("[ERRO] Falha no recv");
        return;
    }
    escrever_log("Tentativa recebida com sucesso");
    buffer[bytes_received] = '\0'; // Garante que a mensagem recebida é válida
    printf("[DEBUG] Mensagem recebida: '%s'\n", buffer);

    // Extrai os valores num, linha, coluna, e tentativa
    if (sscanf(buffer, "%d %d %d %d", &num, &linha, &coluna, &tentativa) != 4) {
        printf("[ERRO] Mensagem inválida recebida do cliente: '%s'\n", buffer);
        escrever_log("Mensagem inválida recebida do cliente");
        return;
    }
    escrever_log("Mensagem válida recebida do cliente");
    printf("[DEBUG] Dados extraídos: Tabuleiro ID=%d, Linha=%d, Coluna=%d, Tentativa=%d\n", num, linha, coluna, tentativa);
    escrever_log("Dados extraídos com sucesso");


    // Prepara o feedback (certo ou errado)
    char resposta[BUFFER_SIZE];
    if (matriz_sol[num-1][linha][coluna] == tentativa) {
        matriz_of[num-1][linha][coluna] = tentativa;
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está correta.", tentativa, linha+1, coluna+1);
        escrever_log("Tentativa correta recebida do cliente");
        ler_matrizes_id(matriz_of, num);
    } else {
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está errada.", tentativa, linha, coluna);
        escrever_log("Tentativa errada recebida do cliente");
    }
    printf("[DEBUG] Feedback gerado: '%s'\n", resposta);

    // Envia o feedback ao cliente
    if (send(client_socket, resposta, strlen(resposta), 0) < 0) {
        perror("[ERRO] Falha ao enviar feedback ao cliente");
        escrever_log("Erro ao enviar feedback ao cliente");
    } else {
        printf("[DEBUG] Feedback enviado ao cliente com sucesso.\n");
        escrever_log("Feedback enviado ao cliente com sucesso");
    }
}


/**
 * Função para lidar com o cliente Apagador
 * 
 * @param client_socket Socket do cliente
 * @return void
 */
void *handle_client_Apagador(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);
    char buffer[BUFFER_SIZE];
    int opcao, client_id;
    int num = 2;

    if (recv(sock, &client_id, sizeof(client_id), 0) <= 0) {
        escrever_log("Erro ao receber ID do cliente");
        perror("Error receiving client ID");
        close(sock);
        return NULL;
    }
    escrever_log("ID do cliente recebido com sucesso");
    printf("New client connected with ID: %d\n", client_id);
    escrever_log("New client connected");
    enviar_id_tabuleiro(sock, num);

    int running2 = 1;
    while (running2) {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            escrever_log("Erro ao receber pedido de tabuleiro");
            perror("Error ao receber pedido de tabuleiro");
            printf("Client %d disconnected due to error.\n", client_id);
            escrever_log("Client disconnected due to receive error");
            break;
        }
        escrever_log("Pedido de tabuleiro recebido com sucesso");
        printf("Cliente %d: %s \n", client_id, buffer);
        envia_tabuleiro(sock, num, matriz_of);

        //_____________________________________________________________
        enviar_menu_apagador(sock);
        printf("Sending menu to client: %d\n", client_id);

        printf("Waiting for option from client: %d\n", client_id);

        // Receive the client's option
        bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            escrever_log("Erro ao receber opção do cliente");
            perror("Error receiving data from client");
            printf("Client %d disconnected due to error.\n", client_id);
            escrever_log("Client disconnected due to receive error");
            break;
        } else if (bytes_received == 0) {
            escrever_log("Cliente desconectado");
            printf("Client %d disconnected.\n", client_id);
            escrever_log("Client disconnected");
            break;
        }
        escrever_log("Opção do cliente recebida com sucesso");
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received from client %d: '%s' (bytes_received: %d)\n", client_id, buffer, bytes_received); // Debug

        // Convert received message to an integer option
        opcao = atoi(buffer); // Try parsing the received data as an integer
        printf("Parsed option from client %d: %d\n", client_id, opcao); // Debug
        escrever_log("Opção do cliente convertida com sucesso");

        // Handle the selected option
        switch (opcao) {
            case 1:
                // Determina o ponteiro para a variável global de casas vazias
                printf("Client %d selected to solve one cell\n", client_id);
                escrever_log("Cliente selecionou resolver uma célula");
                // Envia uma resposta inicial ao cliente confirmando a opção
                strcpy(buffer, "Option 1: one cell requested.\n");
                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("[ERRO] Falha ao enviar resposta inicial ao cliente");
                    escrever_log("Erro ao enviar resposta inicial ao cliente");
                    break;
                }
                

                recv(sock, buffer, BUFFER_SIZE - 1, 0);
                int num_vazias = numero_total_vazias(matriz_of, num);
                printf("[DEBUG] Resposta inicial enviada para o cliente %d: '%s'\n", client_id, buffer);
                while (num_vazias < 81 && num_vazias > 0) {
                    send(sock, &num_vazias, sizeof(num_vazias), 0);
                    printf("Número total de casas vazias: %d\n", num_vazias);
                    // Enquanto houver casas vazias, processa as tentativas recebidas
                    escrever_log("Número total de casas vazias recebido com sucesso");
                    recebe_apaga_celula(sock, matriz_of);
                    recv(sock, buffer, BUFFER_SIZE - 1, 0);
                    envia_tabuleiro(sock, num, matriz_of);
                    num_vazias = numero_total_vazias(matriz_of, num);
                }
                send(sock, "Jogo Acabado.\n", 31, 0);
                printf("[INFO] Tabuleiro %d resolvido pelo cliente %d\n", num, client_id);
                running2 = 0; // Exit loop
                break;

            case 2:
                printf("Client %d quit the game.\n", client_id);
                strcpy(buffer, "Option 2: Exiting the game.\n");
                escrever_log("Cliente desistiu do jogo");
                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("[ERRO] Falha ao enviar mensagem de saída ao cliente");
                    escrever_log("Erro ao enviar mensagem de saída ao cliente");
                    break;
                }
                printf("[DEBUG] Mensagem de saída enviada para o cliente %d.\n", client_id);
                escrever_log("Mensagem de saída enviada ao cliente");
                running2 = 0; // Exit loop
                break;
            default:
                printf("Client %d selected an invalid option: %d\n", client_id, opcao);
                strcpy(buffer, "Invalid option! Please try again.\n");
                escrever_log("Cliente selecionou uma opção inválida");

                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("[ERRO] Falha ao enviar mensagem de opção inválida");
                    escrever_log("Erro ao enviar mensagem de opção inválida");
                } else {
                    escrever_log("Mensagem de opção inválida enviada ao cliente");
                    printf("[DEBUG] Mensagem de opção inválida enviada para o cliente %d.\n", client_id);
                }
                escrever_log("Mensagem de opção inválida enviada ao cliente");
                printf("[DEBUG] opção escolhida pelo cliente: %d\n", opcao);
                break;
        }

        // Debug opcional para verificar envio final (não necessário)
        printf("[DEBUG] Opção processada para cliente %d.\n", client_id);
    }

    // Close connection and update client count
    close(sock);
    escrever_log("Conexão fechada e contagem de clientes atualizada");
    pthread_mutex_lock(&clients_mutex);
    num_clients_sessao--;
    pthread_mutex_unlock(&clients_mutex);
    escrever_log("Clientes atualizados");
    printf("Current clients: %d\n", num_clients_sessao);
    printf("Connection with client %d closed\n", client_id);
    escrever_log("Client connection closed");
    return NULL;
}

/**
 * Função para lidar com o cliente Resolvedor
 * 
 * @param client_socket Socket do cliente
 * @return void
 */
void *handle_client_Resolvedor(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);
    char buffer[BUFFER_SIZE];
    int opcao, client_id;
    int num = 2;
 
    if (recv(sock, &client_id, sizeof(client_id), 0) <= 0) {                     //erro aqui
        escrever_log("Erro ao receber ID do cliente");
        printf("Error receiving client ID: %d\n", client_id);
        close(sock);
        return NULL;
    }
    escrever_log("ID do cliente recebido com sucesso");
    printf("New client connected with ID: %d\n", client_id);
    escrever_log("New client connected");
    enviar_id_tabuleiro(sock, num);

    int running = 1;
    while (running) {

        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            escrever_log("Erro ao receber pedido de tabuleiro");
            perror("Error ao receber pedido de tabuleiro");
            printf("Client %d disconnected due to error.\n", client_id);
            escrever_log("Client disconnected due to receive error");
            break;
        }
        escrever_log("Pedido de tabuleiro recebido com sucesso");
        printf("Cliente %d: %s \n", client_id, buffer);
        envia_tabuleiro(sock, num, matriz_of);

        //_____________________________________________________________
        enviar_menu_resolvedor(sock);
        printf("Sending menu to client: %d\n", client_id);

        printf("Waiting for option from client: %d\n", client_id);

        // Receive the client's option
        bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            escrever_log("Erro ao receber opção do cliente");
            perror("Error receiving data from client");
            printf("Client %d disconnected due to error.\n", client_id);
            escrever_log("Client disconnected due to receive error");
            break;
        } else if (bytes_received == 0) {
            escrever_log("Cliente desconectado");
            printf("Client %d disconnected.\n", client_id);
            escrever_log("Client disconnected");
            break;
        }
        escrever_log("Opção do cliente recebida com sucesso");
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received from client %d: '%s' (bytes_received: %d)\n", client_id, buffer, bytes_received); // Debug

        // Convert received message to an integer option
        opcao = atoi(buffer); // Try parsing the received data as an integer
        printf("Parsed option from client %d: %d\n", client_id, opcao); // Debug
        escrever_log("Opção do cliente convertida com sucesso");

        // Handle the selected option
        switch (opcao) {
            case 1:
                printf("Client %d selected to solve one cell\n", client_id);
                escrever_log("Cliente selecionou resolver uma célula");
                // Envia uma resposta inicial ao cliente confirmando a opção
                strcpy(buffer, "Option 1: one cell requested.\n");
                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("[ERRO] Falha ao enviar resposta inicial ao cliente");
                    escrever_log("Erro ao enviar resposta inicial ao cliente");
                    break;
                }
                printf("[DEBUG] Resposta inicial enviada para o cliente %d: '%s'\n", client_id, buffer);

                int total_vazias = numero_total_vazias(matriz_of, num);
                while (total_vazias > 0 && total_vazias < 81) {
                    printf("Número total de casas vazias: %d\n", total_vazias);
                    // Enquanto houver casas vazias, processa as tentativas recebidas
                    recebe_tentativa_e_envia_feedback(sock, matriz_solucao, matriz_of);
                    bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes_received < 0) {
                        perror("Erro ao receber a confirmaçao de recebimento de feedback do cliente");
                        escrever_log("Erro ao receber tentativa do cliente");
                        break;
                    }

                    envia_tabuleiro(sock, num, matriz_of);
                    send(sock, "espera", strlen("espera"), 0);
                    total_vazias = numero_total_vazias(matriz_of, num);
                }
                printf("[INFO] Tabuleiro %d resolvido pelo cliente %d\n", num, client_id);
                running = 0; // Exit loop
                break;

            
            case 2:
                printf("Client %d requested Full Solution from Server.\n", client_id);
                strcpy(buffer, "Option 3: Server reveals Solution.\n");
                escrever_log("Cliente selecionou solução completa");
                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("[ERRO] Falha ao enviar solução completa ao cliente");
                    escrever_log("Erro ao enviar solução completa ao cliente");
                    break;
                }
                printf("[DEBUG] Mensagem de solução completa enviada ao cliente %d.\n", client_id);
                escrever_log("Mensagem de solução completa enviada ao cliente");
                memset(buffer, 0, BUFFER_SIZE - 1);
                int bytesReceived = recv(sock, buffer, BUFFER_SIZE - 1, 0);
                if (bytesReceived < 0) {
                    perror("[ERRO] Falho sincronizacao do Socket");
                    escrever_log("Erro na sincronização do socket");
                } else {
                    envia_solucao(sock, num);
                    escrever_log("Solução enviada com sucesso");
                    running = 0; // Exit loop
                }

                // Envia a solução completa
                break;

            case 3:
                printf("Client %d quit the game.\n", client_id);
                strcpy(buffer, "Option 5: Exiting the game.\n");
                escrever_log("Cliente desistiu do jogo");
                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("[ERRO] Falha ao enviar mensagem de saída ao cliente");
                    escrever_log("Erro ao enviar mensagem de saída ao cliente");
                    break;
                }
                printf("[DEBUG] Mensagem de saída enviada para o cliente %d.\n", client_id);
                escrever_log("Mensagem de saída enviada ao cliente");
                running = 0; // Exit loop
                break;

            default:
                printf("Client %d selected an invalid option: %d\n", client_id, opcao);
                strcpy(buffer, "Invalid option! Please try again.\n");
                escrever_log("Cliente selecionou uma opção inválida");

                if (send(sock, buffer, strlen(buffer), 0) < 0) {
                    perror("[ERRO] Falha ao enviar mensagem de opção inválida");
                    escrever_log("Erro ao enviar mensagem de opção inválida");
                } else {
                    escrever_log("Mensagem de opção inválida enviada ao cliente");
                    printf("[DEBUG] Mensagem de opção inválida enviada para o cliente %d.\n", client_id);
                }
                escrever_log("Mensagem de opção inválida enviada ao cliente");
                printf("[DEBUG] opção escolhida pelo cliente: %d\n", opcao);
                break;
        }

        // Debug opcional para verificar envio final (não necessário)
        printf("[DEBUG] Opção processada para cliente %d.\n", client_id);
    }

    // Close connection and update client count
    close(sock);
    escrever_log("Conexão fechada e contagem de clientes atualizada");
    pthread_mutex_lock(&clients_mutex);
    num_clients_sessao--;
    pthread_mutex_unlock(&clients_mutex);
    escrever_log("Clientes atualizados");
    printf("Current clients: %d\n", num_clients_sessao);
    printf("Connection with client %d closed\n", client_id);
    escrever_log("Client connection closed");
    return NULL;
}


int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if (argc < 2) {
        escrever_log("Erro ao abrir o ficheiro de configuração");
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }

    char ficheiro_jogos[100], ficheiro_solucoes[100];
    ler_configuracao(argv[1], ficheiro_jogos, ficheiro_solucoes, &porta);

    transforma_matriz(ficheiro_jogos, matriz_of);
    transforma_matriz_solucao(ficheiro_solucoes, matriz_solucao);
    ler_matrizes(matriz_of);
    printf("----------");
    ler_matrizes(matriz_solucao);

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    escrever_log("Socket do servidor criado com sucesso");
    if (server_socket < 0) {
        escrever_log("Erro ao criar socket do servidor");
        perror("Erro ao criar socket do servidor");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(porta);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        escrever_log("Erro no bind");
        perror("Erro no bind");
        close(server_socket);
        return 1;
    }
    escrever_log("Bind feito com sucesso");
    if (listen(server_socket, 10) < 0) {
        escrever_log("Erro ao ouvir no socket");
        perror("Erro ao ouvir no socket");
        close(server_socket);
        return 1;
    }
    escrever_log("Servidor pronto para ouvir conexões");
    printf("Servidor iniciado na porta %d\n", porta); // Debug
    srand(time(NULL));

    while (1) {
        int modoJogo;
        int jogadorModo;
        char buffer[BUFFER_SIZE];

        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            escrever_log("Erro ao aceitar conexão");
            perror("Erro ao aceitar conexão");
            continue;
        }
        escrever_log("Conexão aceita com sucesso");
        pthread_mutex_lock(&clients_mutex);
        if (num_clients_sessao >= 10) {                                   
            escrever_log("Número máximo de clientes atingido");
            close(client_socket);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        escrever_log("Número de clientes na sessão atualizado");
        num_clients_sessao++;
        pthread_mutex_unlock(&clients_mutex);

        int *new_sock = malloc(sizeof(int));
        if (new_sock == NULL) {
            escrever_log("Erro ao alocar memória para new_sock");
            perror("Erro ao alocar memória para new_sock");
            close(client_socket);
            continue;
        }
        *new_sock = client_socket;
        escrever_log("Memória alocada para new_sock com sucesso");

        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            escrever_log("Erro ao receber resolvedor do cliente");
            perror("Erro ao receber resolvedor");
            free(new_sock);
            close(client_socket);
            continue;
        }

        buffer[bytes_received] = '\0'; // Assegura-se de que a mensagem recebida seja terminada com '\0'
        // Extraer os inteiros do buffer
        if (sscanf(buffer, "%d,%d", &modoJogo, &jogadorModo) != 2) {
            printf("modoJogo: %d, jogadorModo: %d\n", modoJogo, jogadorModo);
            printf("Erro ao analisar os dados recebidos: '%s'\n", buffer);
            free(new_sock);
            close(client_socket);
            continue;
        }
        printf("modoJogo: %d, jogadorModo: %d\n", modoJogo, jogadorModo);
        printf("Received from client: '%s' (bytes_received: %d)\n", buffer, bytes_received); // Debug
        send(client_socket, "Conexão estabelecida com sucesso", 30, 0);


        pthread_t tid;
        pthread_mutex_lock(&clients_mutex_board_2); // Bloqueia o primeiro mutex

        switch (modoJogo) {
            case 1:
                /* modo trinco */
                break;

            case 2:
                /* modo Resolvedores-Apagadores (leitores-escritores) */
                

                if (jogadorModo == 1) {
                    // Tenta criar uma thread para "leitor"
                    if (pthread_create(&tid, NULL, handle_client_Resolvedor, (void *)new_sock) != 0) {
                        escrever_log("Erro ao criar thread para o cliente leitor");
                        perror("Erro ao criar thread para o cliente leitor");
                        free(new_sock);
                        close(client_socket);
                        pthread_mutex_unlock(&clients_mutex_board_2); // Libera mutex no caso de erro
                        continue;
                    }
                } else if (jogadorModo == 2) {
                    pthread_mutex_unlock(&clients_mutex_board_2); // Libera o mutex antes de usar outro

                    pthread_mutex_lock(&clients_mutex_board); // Bloqueia o segundo mutex

                    // Tenta criar uma thread para "Resolvedor"
                    if (pthread_create(&tid, NULL, handle_client_Apagador, (void *)new_sock) != 0) {
                        escrever_log("Erro ao criar thread para o cliente escritor");
                        perror("Erro ao criar thread para o cliente escritor");
                        free(new_sock);
                        close(client_socket);
                        pthread_mutex_unlock(&clients_mutex_board); // Libera mutex no caso de erro
                        continue;
                    }

                    pthread_mutex_unlock(&clients_mutex_board); // Libera o segundo mutex após uso
                }

                escrever_log("Thread criada com sucesso");
                pthread_detach(tid); // Desanexa a thread
                escrever_log("Thread desanexada com sucesso");
                break;

            default:
                break;
        }
        pthread_mutex_unlock(&clients_mutex_board_2); // Libera mutex no caso de erro
    }
    // Após sair do loop principal
    escrever_log("Servidor encerrado");
    close(server_socket);
    return 0;
}