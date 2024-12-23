#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "header.h"

#define BUFFER_SIZE 1024
#define SIZE 9

/**
 *  Escreve nos Logs do utilizador a mensagem pretendida com a data e hora do evento
 *
 * @param mensagem Mensagem a ser registada no log : const char *
 * @return void
 */
void escrever_log_cliente(const char *mensagem)
{
    FILE *f = fopen("./logs/cliente_log.txt", "a");
    if (f == NULL)
    {
        printf("Erro ao abrir o ficheiro log.txt\n");
        return;
    }

    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = 0;

    fprintf(f, "%s | %s.\n", timestamp, mensagem);
    fclose(f);
}

/**
 * Função para transformar a string do tabuleiro em uma matriz 9x9
 *
 * @param tabuleiro_str String do tabuleiro : char *
 * @param matriz Matriz 9x9 para armazenar o tabuleiro : int [SIZE][SIZE]
 */
void string_para_matriz(char *tabuleiro_str, int matriz[SIZE][SIZE])
{
    int i = 0, j = 0;
    for (int k = 0; k < strlen(tabuleiro_str); k++)
    {
        if (tabuleiro_str[k] >= '1' && tabuleiro_str[k] <= '9')
        {
            matriz[i][j] = tabuleiro_str[k] - '0';
        }
        else if (tabuleiro_str[k] == '_')
        {
            matriz[i][j] = 0;
        }
        else
        {
            continue;
        }

        j++;
        if (j == SIZE)
        {
            j = 0;
            i++;
        }

        if (i == SIZE)
        {
            break;
        }
    }
    escrever_log_cliente("Matriz transformada com sucesso string para matriz");
}

/**
 * Função para ler o ficheiro de configuração do cliente e carregar as configurações
 *
 * @param config_path Caminho para o ficheiro de configuração : const char *
 * @param porta Ponteiro para armazenar a porta do servidor : int *
 * @param ip_server Ponteiro para armazenar o IP do servidor : char *
 * @param rol Ponteiro para armazenar o rol do cliente : int *
 * @param prioridade Ponteiro para armazenar a prioridade do cliente : int *
 * @return void
 */
void ler_configuracao_cliente(const char *config_path, int *porta, char *ip_server, int *rol, int *prioridade)
{
    FILE *config = fopen(config_path, "r");
    if (config == NULL)
    {
        printf("Erro ao abrir o ficheiro de configuração.\n");
        escrever_log_cliente("Erro ao abrir o ficheiro de configuração");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), config))
    {
        char *token = strtok(linha, "=");

        if (strcmp(token, "porta") == 0)
        {
            token = strtok(NULL, "\n");
            *porta = atoi(token);
            escrever_log_cliente("Porta lida com sucesso");
        }
        else if (strcmp(token, "ip_servidor") == 0)
        {
            token = strtok(NULL, "\n");
            strcpy(ip_server, token);
            escrever_log_cliente("IP do servidor lido com sucesso");
        }
        else if (strcmp(token, "rol") == 0)
        {
            token = strtok(NULL, "\n");
            *rol = atoi(token);
            escrever_log_cliente("rol lido com sucesso");
        }
        else if (strcmp(token, "prioridade") == 0)
        {
            token = strtok(NULL, "\n");
            *prioridade = atoi(token);
            escrever_log_cliente("prioridade lida com sucesso");
        }
    }
    fclose(config);
    escrever_log_cliente("Configuração lida com sucesso");
}

/**
 * Função para obter o ID do utilizador
 *
 * @return ID do utilizador : int
 */
int get_new_user_id()
{
    FILE *file = fopen("users.txt", "r+");
    if (file == NULL)
    {
        file = fopen("users.txt", "w+");
        if (file == NULL)
        {
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

/**
 * Função para criar o socket do cliente
 *
 * @return Socket do cliente : int
 */
int criar_socket_cliente()
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        escrever_log_cliente("Erro no socket do cliente");
        perror("Erro ao criar socket");
        escrever_log_cliente("Erro no socket do cliente");
        exit(EXIT_FAILURE);
    }
    escrever_log_cliente("Socket do cliente criado com sucesso");
    return client_socket;
}

/**
 * Função para configurar o endereço do servidor
 *
 * @param server_addr  Ponteiro para a estrutura do endereço do servidor : struct sockaddr_in
 * @param porta Porta do servidor : int
 * @param ip_server IP do servidor : char *
 * @return void
 */
void configurar_endereco_servidor(struct sockaddr_in *server_addr, int porta, const char *ip_server)
{
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(porta);
    server_addr->sin_addr.s_addr = inet_addr(ip_server);
    escrever_log_cliente("Endereço do servidor configurado com sucesso");
}

/**
 * Função para conectar ao servidor
 *
 * Socket : connect
 *
 * @param client_socket Socket do cliente : int
 * @param server_addr Ponteiro para a estrutura do endereço do servidor : struct sockaddr_in *
 * @return void
 */
void conectar_servidor(int client_socket, struct sockaddr_in *server_addr)
{
    if (connect(client_socket, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0)
    {
        perror("Erro ao conectar ao servidor");
        escrever_log_cliente("Erro ao conectar ao servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    escrever_log_cliente("Cliente conectado ao servidor");
}

/**
 * Função para enviar o ID do utilizador ao servidor
 *
 * Socket : send
 *
 * @param client_socket Socket do cliente : int
 * @param client_id ID do utilizador : int
 * @return void
 */
void enviar_id_cliente(int client_socket, int client_id)
{
    if (send(client_socket, &client_id, sizeof(int), 0) < 0)
    {
        perror("Erro ao enviar ID do cliente");
        escrever_log_cliente("Erro ao enviar ID do cliente");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    escrever_log_cliente("ID do cliente enviado com sucesso");
}

/**
 * Função para imprimir a matriz do tabuleiro no terminal
 *
 * @param matriz Matriz do tabuleiro : int [SIZE][SIZE]
 * @return void
 */
void imprima_matriz(int matriz[9][9])
{
    for (int i = 0; i < 9; i++)
    {
        if (i % 3 == 0 && i != 0)
        {
            printf("---------------------\n");
        }
        for (int j = 0; j < 9; j++)
        {
            if (j % 3 == 0 && j != 0)
            {
                printf("| ");
            }
            printf("%d ", matriz[i][j]);
        }
        printf("\n");
    }
    escrever_log_cliente("Matriz impressa com sucesso");
}

/**
 * Função para escolher uma célula vazia aleatória
 *
 * @param matriz Matriz do tabuleiro : int [SIZE][SIZE]
 * @param linha Ponteiro para armazenar a linha da célula vazia : int *
 * @param coluna Ponteiro para armazenar a coluna da célula vazia : int *
 * @return 1 se a célula vazia foi escolhida, 0 caso contrário : int
 */
int escolhe_celula_sem_nada_aleatoria(int matriz[9][9], int *linha, int *coluna)
{
    int vazias[81][2];
    int count = 0;

    // Lista todas as posições vazias
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (matriz[i][j] == 0)
            { // Considera 0 como vazio
                vazias[count][0] = i;
                vazias[count][1] = j;
                count++;
            }
        }
    }

    if (count == 0)
        return 0; // Não há posições vazias

    // Escolhe uma posição aleatória dentre as vazias
    int indice = rand() % count;
    *linha = vazias[indice][0];
    *coluna = vazias[indice][1];
    escrever_log_cliente("Célula sem nada escolhida com sucesso");
    return 1;
}

/**
 * Função para escolher uma célula preenchida aleatória
 *
 * @param matriz Matriz do tabuleiro : int [SIZE][SIZE]
 * @param linha Ponteiro para armazenar a linha da célula preenchida : int *
 * @param coluna Ponteiro para armazenar a coluna da célula preenchida : int *
 * @return 1 se a célula com algo foi escolhida, 0 caso contrário : int
 */
int escolhe_celula_com_algo(int matriz[9][9], int *linha, int *coluna)
{
    int vazias[81][2];
    int count = 0;

    // Lista todas as posições vazias
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (matriz[i][j] != 0)
            { // Considera 0 como vazio
                vazias[count][0] = i;
                vazias[count][1] = j;
                count++;
            }
        }
    }

    if (count == 81)
        return 0; // já nao há posições ocupadas

    // Escolhe uma posição aleatória dentre as vazias
    int indice = rand() % count;
    *linha = vazias[indice][0];
    *coluna = vazias[indice][1];
    escrever_log_cliente("Célula com algo escolhida com sucesso");
    return 1;
}

/**
 * Função para contar o número de células vazias num tabuleiro
 *
 * @param matriz Matriz do tabuleiro : int [SIZE][SIZE]
 * @return Número de células vazias : int
 */
int numCelulasVazias(int matriz[9][9])
{
    int count = 0;
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (matriz[i][j] == 0)
            { // Considera 0 como vazio
                count++;
            }
        }
    }
    return count;
}

/**
 * Função para enviar a tentativa ao servidor
 *
 * Socket : send
 *
 * @param client_socket Socket do cliente : int
 * @param num ID do tabuleiro : int
 * @param linha Linha da célula : int
 * @param coluna Coluna da célula : int
 * @return void
 */
void envia_tentativa(int client_socket, int num, int linha, int coluna)
{
    int tentativa = (rand() % 9) + 1;
    char buffer[BUFFER_SIZE];

    printf("[DEBUG] Tentativa gerada: %d\n", tentativa);
    snprintf(buffer, BUFFER_SIZE, "%d %d %d %d", num, linha, coluna, tentativa);

    printf("[DEBUG] Enviando mensagem para o servidor: '%s'\n", buffer);
    if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0)
    {
        perror("[ERRO] Falha ao enviar tentativa para o servidor");
        escrever_log_cliente("Tentativa não enviada com sucesso");
        return;
    }
    escrever_log_cliente("Tentativa enviada com sucesso");
    printf("[DEBUG] Tentativa enviada com sucesso!\n");
}

/**
 * Função para enviar a tentativa do Apagador ao servidor
 *
 * Socket : send
 *
 * @param client_socket Socket do cliente : int
 * @param num ID do tabuleiro : int
 * @param linha Linha da célula : int
 * @param coluna Coluna da célula : int
 * @return void
 */
void envia_tentativa_apaga(int client_socket, int num, int linha, int coluna)
{
    char buffer[BUFFER_SIZE];

    // Formata a mensagem para envio com os valores para apagar (eliminação)
    snprintf(buffer, BUFFER_SIZE, "%d,%d,%d", num, linha, coluna);

    printf("[DEBUG] Enviando solicitação para eliminar ao servidor: '%s'\n", buffer);

    // Envia os dados para o servidor
    if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0)
    {
        perror("[ERRO] Falha ao enviar solicitação ao servidor");
        escrever_log_cliente("Solicitação de apagar não enviada com sucesso");
        return;
    }

    escrever_log_cliente("Solicitação de apagar enviada com sucesso");
    printf("[DEBUG] Solicitação de apagar enviada com sucesso!\n");
}

/**
 * Função para processar o feedback do servidor
 *
 * @param feedback Feedback do servidor : const char *
 * @return void
 */
void processa_feedback(const char *feedback)
{
    // Variável local para armazenar o valor temporário de casas vazias

    // Verifica se a tentativa está correta ou errada
    if (strstr(feedback, "correta") != NULL)
    {
        escrever_log_cliente("Tentativa correta");
    }
    else if (strstr(feedback, "errada") != NULL)
    {
        escrever_log_cliente("Tentativa incorreta");
    }
    else
    {
        printf("[ERRO] Feedback não reconhecido: '%s'\n", feedback);
        escrever_log_cliente("Feedback não reconhecido");
        return; // Sai da função em caso de erro
    }
}

/**
 * Função para processar o feedback do Apagador do servidor
 *
 * @param feedback Feedback do servidor : const char *
 * @return void
 */
void processa_feedback_apaga(const char *feedback)
{
    // Copia o feedback para um buffer local para poder modificá-lo
    char buffer[BUFFER_SIZE];
    strncpy(buffer, feedback, BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0'; // Assegura que a string esteja terminada em '\0'

    // Separa a string pelo caractere ':'
    char *token = strtok(buffer, ":");

    // Verifica se a tentativa está correta ou errada
    if (token != NULL && strstr(token, "OK") != NULL)
    {
        escrever_log_cliente(feedback);
    }
    else if (token != NULL && strstr(token, "ERRO") != NULL)
    {
        escrever_log_cliente(feedback);
    }
    else
    {
        printf("[ERRO] Feedback não reconhecido: '%s'\n", feedback);
        escrever_log_cliente("Feedback não reconhecido");
        return; // Sai da função em caso de erro
    }
}

/**
 * Função para receber o feedback do servidor
 *
 * Socket : recv
 *
 * @param client_socket Socket do cliente : int
 * @return void
 */
void recebe_feedback_Apaga(int client_socket)
{
    char buffer[BUFFER_SIZE];

    escrever_log_cliente("Aguardando feedback do servidor");
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        escrever_log_cliente("Feedback do servidor recebido");
        printf("[DEBUG] Feedback do servidor recebido: '%s'\n", buffer);
    }
    else
    {
        escrever_log_cliente("Falha ao receber feedback do servidor");
        perror("[ERRO] Falha ao receber feedback do servidor");
    }
    printf("Resposta do servidor:\n%s\n", buffer);
    processa_feedback_apaga(buffer);
}

/**
 * Função para receber o feedback do servidor
 *
 * Socket : recv
 *
 * @param client_socket Socket do cliente : int
 * @return void
 */
void recebe_feedback_tentativa(int client_socket)
{
    char buffer[BUFFER_SIZE];

    escrever_log_cliente("Aguardando feedback do servidor");
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        escrever_log_cliente("Feedback do servidor recebido");
        printf("[DEBUG] Feedback do servidor recebido: '%s'\n", buffer);
    }
    else
    {
        escrever_log_cliente("Falha ao receber feedback do servidor");
        perror("[ERRO] Falha ao receber feedback do servidor");
    }
    printf("Resposta do servidor:\n%s\n", buffer);
    processa_feedback(buffer);
}

/**
 * Função para receber o tabuleiro do servidor
 *
 * Socket : recv - send
 *
 * @param client_socket Socket do cliente : int
 * @param matriz Matriz para armazenar o tabuleiro : int [SIZE][SIZE]
 * @return void
 */
void recebe_Tabuleiro(int client_socket, int matriz[9][9])
{
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_received <= 0)
    {
        printf("Servidor desconectado.\n");
        escrever_log_cliente(" Cliente não recebeu tabuleiro");
        return;
    }
    escrever_log_cliente("TabuleirO recebido com sucesso");
    send(client_socket, "Tabuleiro recebido\0", BUFFER_SIZE, 0);
    escrever_log_cliente("Confirmação de tabuleiro enviada");
    buffer[bytes_received] = '\0'; // Corrigido para usar buffer
    string_para_matriz(buffer, matriz);
}

void comunicacao_Resolvedor(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int matriz[SIZE][SIZE] = {0}; // Inicializa a matriz com zeros
    int id_tabuleiro;
    int linha_branca;
    int coluna_branca;
    if (recv(client_socket, &id_tabuleiro, sizeof(int), 0) <= 0)
    {
        escrever_log_cliente("Erro ao receber ID do tabuleiro");
        perror("Erro ao receber ID do tabuleiro");
        close(client_socket);
        return;
    }
    escrever_log_cliente("ID do tabuleiro recebido com sucesso");
    printf("Id recebido %d \n", id_tabuleiro);

    // Inicia o loop para enviar e receber respostas
    while (1)
    {
        // solicita Tabuleiro
        // ____________________________________
        strcpy(buffer, "Solicita Tabuleiro");
        send(client_socket, buffer, BUFFER_SIZE, 0);
        escrever_log_cliente("Solicitação de tabuleiro enviada");

        recebe_Tabuleiro(client_socket, matriz);
        imprima_matriz(matriz);

        // _______________________________________________
        // Recebe o menu inicial do servidor
        printf("Recebe menu inicial\n");
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        escrever_log_cliente("Menu inicial recebido");
        printf("Depois de receber menu inicial\n");
        if (bytes_received <= 0)
        {
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

        if (strlen(buffer) == 1)
        {
            printf("Por favor, insira um número válido.\n");
            continue;
        }

        // Remove o newline que `fgets` deixa no buffer
        buffer[strcspn(buffer, "\n")] = 0;

        int resposta = atoi(buffer); // Converte a entrada para inteiro

        // Envia a opção para o servidor
        if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0)
        {
            escrever_log_cliente("Erro ao enviar opção para o servidor");
            perror("Erro ao enviar dados");
            return;
        }
        escrever_log_cliente("Opção enviada com sucesso");
        switch (resposta)
        {
        case 1:
            // Determina o ponteiro para a variável global de casas vazias
            // Determina o ponteiro para a variável global de casas vazias

            printf("[DEBUG] Opção 1: Resolver o tabuleiro completo selecionada.\n");

            escrever_log_cliente("Opção 1: Resolver o tabuleiro completo selecionada");
            // Recebe confirmação do servidor
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
            }
            else
            {
                escrever_log_cliente("Erro ao receber resposta do servidor");
                perror("[ERRO] Falha ao receber resposta do servidor");
                return;
            }
            escrever_log_cliente("Confirmação de opção recebida");
            int total_vazias = numCelulasVazias(matriz);

            while (total_vazias > 0 && total_vazias < 81)
            {

                printf("Número total de casas vazias: %d\n", total_vazias);
                escrever_log_cliente("Número total de casas vazias recebido com sucesso");
                if (escolhe_celula_sem_nada_aleatoria(matriz, &linha_branca, &coluna_branca))
                {
                    printf("Posição vazia encontrada em: linha %d, coluna %d\n", linha_branca, coluna_branca);
                    printf("O ID do tabuleiro é: %d\n", id_tabuleiro);

                    // Envia tentativa ao servidor
                    envia_tentativa(client_socket, id_tabuleiro, linha_branca, coluna_branca); // send
                    // Recebe feedback do servidor
                    printf("[DEBUG] Tentativa enviada. Aguardando feedback do servidor...\n");

                    recebe_feedback_tentativa(client_socket); // recv - send
                }
                else
                {
                    escrever_log_cliente("Nenhuma posição vazia encontrada. Sudoku resolvido!");
                    printf("Nenhuma posição vazia encontrada. Sudoku resolvido!\n");
                    break;
                }

                recebe_Tabuleiro(client_socket, matriz); // recv - send
                imprima_matriz(matriz);
                bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0); // recv

                total_vazias = numCelulasVazias(matriz);
            }
            printf("[INFO] Jogo %d Acabado!\n", id_tabuleiro);
            escrever_log_cliente("Jogo Acabado :)");
            return;
            break;

        case 2:
            printf("[DEBUG] Opção 3: Solicitar solução parcial selecionada.\n");
            escrever_log_cliente("Opção 3: Solicitar solução parcial selecionada");
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                escrever_log_cliente("Resposta do servidor recebida da opcao 3");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor: %s", buffer);
                send(client_socket, "Resposta do servidor confirmada", BUFFER_SIZE, 0);
                recebe_Tabuleiro(client_socket, matriz);
                imprima_matriz(matriz);
                return;
            }
            else
            {
                perror("[ERRO] Falha ao receber resposta do servidor");
                escrever_log_cliente("Erro ao receber resposta do servidor da opcao 3");
                return;
            }
            break;

        case 3:
            printf("[DEBUG] Opção 5: Desistir do Jogo.\n");
            escrever_log_cliente("Opção 5: Desistir do Jogo");
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                escrever_log_cliente("Resposta do servidor recebida da opcao 5");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
            }
            else
            {
                escrever_log_cliente("Erro ao receber resposta do servidor da opcao 5");
                perror("[ERRO] Falha ao receber resposta do servidor");
            }
            return;
            break; // Sai do loop principal no cliente

        default:
            printf("[ERRO] Opção inválida! Tente novamente.\n");
            escrever_log_cliente("Opção inválida! Tente novamente");
            printf("[DEBUG] opção escolhida: %d\n", resposta);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                escrever_log_cliente("Resposta do servidor recebida da opcao invalida");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
            }
            else
            {
                escrever_log_cliente("Erro ao receber resposta do servidor da opcao invalida");
                perror("[ERRO] Falha ao receber resposta do servidor");
            }
            printf("[DEBUG] opção escolhida: %d\n", resposta);
            break;
        }
    }
}

/**
 * Função para enviar a tentativa do Resolvedor ao servidor
 *
 *
 * @param client_socket Socket do cliente : int
 * @param id_Tabuleiro ID do tabuleiro : int
 * @param matriz Matriz do tabuleiro : int [SIZE][SIZE]
 * @param linha_branca Linha da célula vazia : int
 * @param coluna_branca Coluna da célula vazia : int
 *
 */


void comunicacao(int client_socket, int rol_Jogador)
{
    char buffer[BUFFER_SIZE];
    int matriz[SIZE][SIZE] = {0}; // Inicializa a matriz com zeros
    int id_tabuleiro;
    int linha_branca;
    int coluna_branca;
    if (recv(client_socket, &id_tabuleiro, sizeof(int), 0) <= 0)
    {
        escrever_log_cliente("Erro ao receber ID do tabuleiro");
        perror("Erro ao receber ID do tabuleiro");
        close(client_socket);
        return;
    }
    escrever_log_cliente("ID do tabuleiro recebido com sucesso");
    printf("Id recebido %d \n", id_tabuleiro);

    // Inicia o loop para enviar e receber respostas
    while (1)
    {
        // solicita Tabuleiro
        // ____________________________________
        strcpy(buffer, "Solicita Tabuleiro");
        send(client_socket, buffer, BUFFER_SIZE, 0);
        escrever_log_cliente("Solicitação de tabuleiro enviada");

        recebe_Tabuleiro(client_socket, matriz);
        imprima_matriz(matriz);

        // _______________________________________________
        // Recebe o menu inicial do servidor
        printf("Recebe menu inicial\n");
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        escrever_log_cliente("Menu inicial recebido");
        printf("Depois de receber menu inicial\n");
        if (bytes_received <= 0)
        {
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

        if (strlen(buffer) == 1)
        {
            printf("Por favor, insira um número válido.\n");
            continue;
        }

        // Remove o newline que `fgets` deixa no buffer
        buffer[strcspn(buffer, "\n")] = 0;

        int resposta = atoi(buffer); // Converte a entrada para inteiro

        // Envia a opção para o servidor
        if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0)
        {
            escrever_log_cliente("Erro ao enviar opção para o servidor");
            perror("Erro ao enviar dados");
            return;
        }
        escrever_log_cliente("Opção enviada com sucesso");
        switch (resposta)
        {
        case 1:

            if (rol_Jogador == 1)
            {
                printf("[DEBUG] Opção 1: Resolver o tabuleiro completo selecionada.\n");
                escrever_log_cliente("Opção 1: Resolver o tabuleiro completo selecionada");
                int total_vazias = numCelulasVazias(matriz);

                while (total_vazias > 0 && total_vazias < 81)
                {

                    printf("Número total de casas vazias: %d\n", total_vazias);
                    if (escolhe_celula_sem_nada_aleatoria(matriz, &linha_branca, &coluna_branca))
                    {
                        printf("Posição vazia encontrada em: linha %d, coluna %d\n", linha_branca, coluna_branca);
                        printf("O ID do tabuleiro é: %d\n", id_tabuleiro);

                        // Envia tentativa ao servidor
                        envia_tentativa(client_socket, id_tabuleiro, linha_branca, coluna_branca); // send
                        // Recebe feedback do servidor
                        printf("[DEBUG] Tentativa enviada. Aguardando feedback do servidor...\n");

                        recebe_feedback_tentativa(client_socket); // recv
                    }
                    else
                    {
                        escrever_log_cliente("Nenhuma posição vazia encontrada. Sudoku resolvido!");
                        printf("Nenhuma posição vazia encontrada. Sudoku resolvido!\n");
                        break;
                    }

                    recebe_Tabuleiro(client_socket, matriz); // recv - send
                    imprima_matriz(matriz);

                    total_vazias = numCelulasVazias(matriz);
                }
                printf("[INFO] Jogo %d Acabado!\n", id_tabuleiro);
                escrever_log_cliente("Jogo Acabado :)");
                return;
            }
            else if (rol_Jogador == 2)
            {
                // option1Apagador(client_socket, id_tabuleiro, matriz, linha_branca, coluna_branca);
            }

            break;

        case 2:
            printf("[DEBUG] Opção 3: Solicitar solução parcial selecionada.\n");
            escrever_log_cliente("Opção 3: Solicitar solução parcial selecionada");
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                escrever_log_cliente("Resposta do servidor recebida da opcao 3");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor: %s", buffer);
                send(client_socket, "Resposta do servidor confirmada", BUFFER_SIZE, 0);
                recebe_Tabuleiro(client_socket, matriz);
                imprima_matriz(matriz);
                return;
            }
            else
            {
                perror("[ERRO] Falha ao receber resposta do servidor");
                escrever_log_cliente("Erro ao receber resposta do servidor da opcao 3");
                return;
            }
            break;

        case 3:
            printf("[DEBUG] Opção 5: Desistir do Jogo.\n");
            escrever_log_cliente("Opção 5: Desistir do Jogo");
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                escrever_log_cliente("Resposta do servidor recebida da opcao 5");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
            }
            else
            {
                escrever_log_cliente("Erro ao receber resposta do servidor da opcao 5");
                perror("[ERRO] Falha ao receber resposta do servidor");
            }
            return;
            break; // Sai do loop principal no cliente

        default:
            printf("[ERRO] Opção inválida! Tente novamente.\n");
            escrever_log_cliente("Opção inválida! Tente novamente");
            printf("[DEBUG] opção escolhida: %d\n", resposta);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                escrever_log_cliente("Resposta do servidor recebida da opcao invalida");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
            }
            else
            {
                escrever_log_cliente("Erro ao receber resposta do servidor da opcao invalida");
                perror("[ERRO] Falha ao receber resposta do servidor");
            }
            printf("[DEBUG] opção escolhida: %d\n", resposta);
            break;
        }
    }
}

void comunicacao_Apagador(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int matriz[SIZE][SIZE] = {0}; // Inicializa a matriz com zeros
    int id_tabuleiro;
    // int linha_branca;
    // int coluna_branca;

    int linha_ocupada, coluna_ocupada;

    if (recv(client_socket, &id_tabuleiro, sizeof(int), 0) <= 0)
    {
        escrever_log_cliente("Erro ao receber ID do tabuleiro");
        perror("Erro ao receber ID do tabuleiro");
        close(client_socket);
        return;
    }

    escrever_log_cliente("ID do tabuleiro recebido com sucesso");
    printf("Id recebido %d", id_tabuleiro);

    // Inicia o loop para enviar e receber respostas
    while (1)
    {
        // solicita Tabuleiro
        // ____________________________________
        strcpy(buffer, "Solicita Tabuleiro");
        send(client_socket, buffer, BUFFER_SIZE, 0);
        escrever_log_cliente("Solicitação de tabuleiro enviada");

        recebe_Tabuleiro(client_socket, matriz);
        imprima_matriz(matriz);

        // _______________________________________________
        // Recebe o menu inicial do servidor
        printf("Recebe menu inicial\n");
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        escrever_log_cliente("Menu inicial recebido");
        printf("Depois de receber menu inicial\n");
        if (bytes_received <= 0)
        {
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

        if (strlen(buffer) == 1)
        {
            printf("Por favor, insira um número válido.\n");
            continue;
        }

        // Remove o newline que `fgets` deixa no buffer
        buffer[strcspn(buffer, "\n")] = 0;

        int resposta = atoi(buffer); // Converte a entrada para inteiro

        // Envia a opção para o servidor
        if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0)
        {
            escrever_log_cliente("Erro ao enviar opção para o servidor");
            perror("Erro ao enviar dados");
            return;
        }
        escrever_log_cliente("Opção enviada com sucesso");
        switch (resposta)
        {
        case 1:

            printf("[DEBUG] Opção 1: Apagar o tabuleiro completo selecionada.\n");
            escrever_log_cliente("Opção 1: Apagar o tabuleiro completo selecionada");
            // Recebe confirmação do servidor
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            escrever_log_cliente("Confirmação de opção recebida");

            if (bytes_received > 0)
            {

                escrever_log_cliente("Tabuleiro recebido com sucesso");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor:\n%s", buffer);

                send(client_socket, "Inicio", BUFFER_SIZE, 0);
                int num_vazias = numCelulasVazias(matriz);
                while (num_vazias < 81 && num_vazias > 0)
                {

                    recv(client_socket, buffer, BUFFER_SIZE, 0);
                    printf("Número total de casas vazias: %d\n", num_vazias);

                    escolhe_celula_com_algo(matriz, &linha_ocupada, &coluna_ocupada);
                    printf("Posição ocupada encontrada em: linha %d, coluna %d\n", linha_ocupada, coluna_ocupada);
                    printf("O ID do tabuleiro é: %d\n", id_tabuleiro);

                    // Envia tentativa ao servidor
                    envia_tentativa_apaga(client_socket, id_tabuleiro, linha_ocupada, coluna_ocupada);

                    // Recebe feedback do servidor
                    printf("[DEBUG] Tentativa enviada. Aguardando feedback do servidor...\n");
                    recebe_feedback_Apaga(client_socket);
                    send(client_socket, "Feedback recebido", BUFFER_SIZE, 0);

                    recebe_Tabuleiro(client_socket, matriz); // recv - send

                    imprima_matriz(matriz);
                    num_vazias = numCelulasVazias(matriz);
                }
                bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
                if (bytes_received > 0)
                {
                    buffer[bytes_received] = '\0';
                    printf("%s \n", buffer);
                    escrever_log_cliente(buffer);
                }
                printf("Número total de casas vazias: %d\n", num_vazias);
            }
            else
            {
                escrever_log_cliente("Erro ao receber resposta do servidor");
                perror("[ERRO] Falha ao receber resposta do servidor");
                return;
            }

            printf("[INFO] Jogo %d Acabado!\n", id_tabuleiro);
            escrever_log_cliente("Jogo acabado :)");
            return;
            break;
        case 2:
            printf("[DEBUG] Opção 2: Desistir do Jogo.\n");
            escrever_log_cliente("Opção 2: Desistir do Jogo");
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                escrever_log_cliente("Resposta do servidor recebida da opcao 5");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
            }
            else
            {
                escrever_log_cliente("Erro ao receber resposta do servidor da opcao 5");
                perror("[ERRO] Falha ao receber resposta do servidor");
            }
            return;
            break; // Sai do loop principal no cliente
        default:
            printf("[ERRO] Opção inválida! Tente novamente.\n");
            escrever_log_cliente("Opção inválida! Tente novamente");
            printf("[DEBUG] opção escolhida: %d\n", resposta);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0)
            {
                escrever_log_cliente("Resposta do servidor recebida da opcao invalida");
                buffer[bytes_received] = '\0';
                printf("Resposta do servidor:\n%s", buffer);
            }
            else
            {
                escrever_log_cliente("Erro ao receber resposta do servidor da opcao invalida");
                perror("[ERRO] Falha ao receber resposta do servidor");
            }
            printf("[DEBUG] opção escolhida: %d\n", resposta);
            break;
        }
    }
}

void comunicar_servidor(int client_socket, int modoJogo, int jogadorModo)
{
    switch (modoJogo)
    {
    case 1:
        //comunicacao_Trinco(client_socket);
        break;
    case 2:
        if (jogadorModo == 1)
        {
            printf("Resolvedor\n");
            comunicacao_Resolvedor(client_socket);
        }
        else if (jogadorModo == 2)
        {
            printf("Apagador\n");
            comunicacao_Apagador(client_socket);
        }
        break;
    case 3:
        printf("Resolvedor\n");
        //comunicacao_Trinco(client_socket);
        break;
    default:
        printf("[ERRO] Modo de jogo inválido! Tente novamente.\n");
        printf("[DEBUG] Modo de jogo escolhido: %d\n", modoJogo);
        return;
    }
}

int main(int argc, char *argv[])
{
    int client_socket;
    int porta;
    char ip[256];
    int modoJogo = 0;
    int rol = 0;
    int prioridade = 0;

    char buffer[BUFFER_SIZE];
    if (argc < 2)
    {
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }

    int client_id = get_new_user_id();

    ler_configuracao_cliente(argv[1], &porta, ip, &rol, &prioridade);

    struct sockaddr_in server_addr;

    client_socket = criar_socket_cliente();
    configurar_endereco_servidor(&server_addr, porta, ip);

    conectar_servidor(client_socket, &server_addr);
    printf("Cliente %d conectado ao servidor\n", client_id);

    snprintf(buffer, BUFFER_SIZE, "%d,%d", rol, prioridade);
    printf("rol, prioridade: %s\n", buffer);

    if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0)
    {
        perror("Erro ao enviar resolvedor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    recv(client_socket, &modoJogo, sizeof(int), 0);
    printf("Modo de jogo: %d\n", modoJogo);

    enviar_id_cliente(client_socket, client_id);
    printf("Modo de Jogo %d \n", modoJogo);
    printf("Rol do jogador %d \n", rol);
    printf("Prioridade do jogador %d \n", prioridade);

    comunicacao(client_socket, rol);

    close(client_socket);
    escrever_log_cliente("Cliente desconectado");
    return 0;
}
