#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include "header.h"
#include <semaphore.h>

#define BUFFER_SIZE 1024
#define TAMANHO 1024
#define SIZE 4
#define LC 9
#define T 81

int matriz_of[SIZE][LC][LC] = {{{0}}};
int matriz_solucao[SIZE][LC][LC] = {{{0}}};
int modoJogo;
int num_clients_sessao = 0;
int server_socket, client_socket, porta;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clients_mutex_board = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clients_mutex_board_2 = PTHREAD_MUTEX_INITIALIZER;
//-------------------Barrier-------------------------------
#define MAX_CLIENTES 4
int clients_waiting = 0;
sem_t sem_barrier;
pthread_mutex_t mutex_barreira = PTHREAD_MUTEX_INITIALIZER;
//-------------------Exclusão Mútua-------------------------
pthread_mutex_t mutexCelulas[LC][LC];
//-------------------Produtores/Consumidores------------------------------
int apagadores_Espera = 0;
sem_t pode_inserir;
sem_t pode_apagar;
pthread_mutex_t mutex_resolvedores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_resolvedores_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_apagadores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_apagadores_2 = PTHREAD_MUTEX_INITIALIZER;

//-------------------Prioridades------------------------------
sem_t prioridade_1[LC][LC];
sem_t prioridade_2[LC][LC];
sem_t prioridade_3[LC][LC];
pthread_mutex_t mutex_Escritura = PTHREAD_MUTEX_INITIALIZER;
int prioridade_1_count[LC][LC] = {{0}};
int prioridade_2_count[LC][LC] = {{0}};
int prioridade_3_count[LC][LC] = {{0}};

typedef struct
{
    int client_socket;
    int rol_Jogador;
    int prioridade;
    int num;
} client_args_t;

/**
 *  Função para registrar logs do servidor incluindo a data e hora do evento
 * @param mensagem Mensagem a ser registrada no log
 * @return void
 */
void escrever_log(const char *mensagem)
{
    FILE *f = fopen("./logs/log.txt", "a");
    if (f == NULL)
    {
        printf("Erro ao abrir o ficheiro log.txt\n");

        return;
    }

    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = 0; // Remove a nova linha do timestamp

    fprintf(f, "%s | %s\n", timestamp, mensagem);
    fclose(f);
}

void escrever_log_com_cliente_id(int cliente_id, const char *mensagem)
{
    FILE *f = fopen("./logs/log.txt", "a");
    if (f == NULL)
    {
        printf("Erro ao abrir o ficheiro log.txt\n");
        return;
    }

    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove a nova linha do timestamp

    // Construir a mensagem a ser registrada
    char log_message[256];
    snprintf(log_message, sizeof(log_message), "Cliente: %d - %s", cliente_id, mensagem);
    fprintf(f, "%s | %s\n", timestamp, log_message);

    fclose(f);
}

void escrever_log_com_cliente_id_tentativa(int cliente_id, int tentativa, const char *mensagem)
{

    FILE *f = fopen("./logs/log.txt", "a");
    if (f == NULL)
    {
        printf("Erro ao abrir o ficheiro log.txt\n");
        return;
    }

    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove a nova linha do timestamp

    // Construir a mensagem a ser registrada
    char log_message[256];
    snprintf(log_message, sizeof(log_message), "%d - %s - %d", cliente_id, mensagem, tentativa);
    fprintf(f, "%s | %s\n", timestamp, log_message);

    fclose(f);
}

void escrever_log_com_cliente_id_tentativa_col_row(int cliente_id, int tentativa, int linha, int coluna, const char *mensagem)
{
    FILE *f = fopen("./logs/log.txt", "a");
    if (f == NULL)
    {
        printf("Erro ao abrir o ficheiro log.txt\n");
        return;
    }

    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove a nova linha do timestamp

    // Construir a mensagem a ser registrada
    char log_message[256];
    snprintf(log_message, sizeof(log_message), "Client: %d - %s - %d -> (%d,%d)", cliente_id, mensagem, tentativa, linha, coluna);
    fprintf(f, "%s | %s\n", timestamp, log_message);

    fclose(f);
}

void reset_user_id()
{
    FILE *file = fopen("users.txt", "w+");
    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo de ID");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "0");
    fclose(file);
}
/**
 * Função para ler o ficheiro de configuração e carregar as configurações
 * @param config_path Caminho para o ficheiro de configuração
 * @param ficheiro_jogos Ponteiro para armazenar o caminho do ficheiro de jogos
 * @param ficheiro_solucoes Ponteiro para armazenar o caminho do ficheiro de soluções
 * @param porta Ponteiro para armazenar a porta do servidor
 * @return void
 */
void ler_configuracao(char *config_path, char *ficheiro_jogos, char *ficheiro_solucoes, int *porta)
{
    FILE *config = fopen(config_path, "r");
    if (config == NULL)
    {
        escrever_log("Erro ao abrir o ficheiro de configuração");
        printf("Erro ao abrir o ficheiro de configuração.\n");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), config))
    {
        char *token = strtok(linha, "=");

        if (strcmp(token, "ficheiro_jogos") == 0)
        {
            token = strtok(NULL, "\n");
            strcpy(ficheiro_jogos, token);
            escrever_log("Ficheiro de jogos lido com sucesso");
        }
        else if (strcmp(token, "ficheiro_solucoes") == 0)
        {
            token = strtok(NULL, "\n");
            strcpy(ficheiro_solucoes, token);
            escrever_log("Ficheiro de soluções lido com sucesso");
        }
        else if (strcmp(token, "porta") == 0)
        {
            token = strtok(NULL, "\n");
            *porta = atoi(token);
            escrever_log("Porta lida com sucesso");
        }
    }
    fclose(config);
    escrever_log("Inicio do servidor: Configuração lida com sucesso");
}

/**
 * Função para transformar o ficheiro de solucoes.txt numa matriz 3D
 * @param ficheiro_jogos Caminho para o ficheiro de jogos
 * @param matriz_of Matriz 3D para armazenar os jogos
 * @return void
 */
void transforma_matriz_solucao(char *ficheiro_solucoes)
{
    char buffer[BUFFER_SIZE];
    ficheiro_solucoes[strcspn(ficheiro_solucoes, "\r")] = 0;

    FILE *f = fopen(ficheiro_solucoes, "r");

    if (f == NULL)
    {
        printf("Erro ao abrir o ficheiro das soluçoes jogos para leitura.\n");
        escrever_log("Erro ao abrir o ficheiro dos jogos para leitura");
        return;
    }

    int jogo_index = 0;
    while (fgets(buffer, BUFFER_SIZE, f) != NULL && jogo_index < SIZE)
    {
        buffer[strcspn(buffer, "\n")] = 0; // Remover o '\n'
        if (fgets(buffer, BUFFER_SIZE, f) == NULL)
        {
            break; // Se não houver outra linha, encerra o loop
        }
        buffer[strcspn(buffer, "\n")] = 0; // Remover o '\n'

        // Preencher a matriz 9x9 com os valores do buffer
        int k = 0; // Índice do caractere no buffer
        for (int i = 0; i < LC; i++)
        {
            for (int j = 0; j < LC; j++)
            {
                if (buffer[k] == '_')
                {
                    matriz_solucao[jogo_index][i][j] = 0; // Usar 0 para posições desconhecidas
                }
                else
                {
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
void transforma_matriz(char *ficheiro_jogos)
{
    char buffer[BUFFER_SIZE];
    ficheiro_jogos[strcspn(ficheiro_jogos, "\r")] = 0;
    FILE *f = fopen(ficheiro_jogos, "r");

    if (f == NULL)
    {
        escrever_log("Erro ao abrir o ficheiro dos jogos para leitura");
        printf("Erro ao abrir o ficheiro dos jogos para leitura.\n");
        return;
    }

    int jogo_index = 0;
    while (fgets(buffer, BUFFER_SIZE, f) != NULL && jogo_index < SIZE)
    {
        buffer[strcspn(buffer, "\n")] = 0; // Remover o '\n'
        if (fgets(buffer, BUFFER_SIZE, f) == NULL)
        {
            break; // Se não houver outra linha, encerra o loop
        }
        buffer[strcspn(buffer, "\n")] = 0; // Remover o '\n'

        // Preencher a matriz 9x9 com os valores do buffer
        int k = 0; // Índice do caractere no buffer
        for (int i = 0; i < LC; i++)
        {
            for (int j = 0; j < LC; j++)
            {
                if (buffer[k] == '_')
                {
                    matriz_of[jogo_index][i][j] = 0; // Usar 0 para posições desconhecidas
                }
                else
                {
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
 * Função para calcular o número total de células vazias num tabuleiro específico
 * @param matriz Matriz 3D com os jogos
 * @param num ID do jogo
 * @return int Número total de células vazias
 */
int numero_total_vazias(int num)
{
    int total_vazias = 0;
    // Percorrer a matriz e registrar coordenadas de células vazias

    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (matriz_of[num - 1][i][j] == 0)
            {
                total_vazias++;
            }
        }
    }
    return total_vazias;
}

/**
 * Função para escrever no terminal as matrizes de jogos atuais
 * @param matriz_of Matriz 3D com os jogos
 * @return void
 */
void ler_matrizes(int matriz_of[SIZE][LC][LC])
{
    for (int i = 0; i < SIZE; i++)
    {
        printf("Jogo %d:\n", i + 1);
        for (int j = 0; j < LC; j++)
        {
            for (int l = 0; l < LC; l++)
            {
                printf("%d ", matriz_of[i][j][l]);
            }
            printf("\n"); // Nova linha após cada linha da matriz
        }
        printf("\n"); // Linha em branco entre jogos
    }
}

/**
 * Função para escrever no terminal uma matriz de jogos específica mediante o ID
 * @param matriz_of Matriz 3D com os jogos
 * @param num ID do jogo
 * @return void
 */
void ler_matrizes_id(int matriz_of[SIZE][LC][LC], int num)
{

    printf("Jogo %d:\n", num);
    for (int j = 0; j < LC; j++)
    {
        for (int l = 0; l < LC; l++)
        {
            printf("%d ", matriz_of[num - 1][j][l]);
        }
        printf("\n"); // Nova linha após cada linha da matriz
    }
    printf("\n"); // Linha em branco entre jogos
}

/**
 * Função para formatar o tabuleiro de 81 caracteres em um tabuleiro 9x9 com os devidos espaços es barras
 * @param tabuleiro Tabuleiro de 81 caracteres
 * @param formatted_board Tabuleiro formatado resultante
 * @return void
 */
void formatar_tabuleiro(char *tabuleiro, char *formatted_board)
{
    int index = 0;
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            formatted_board[index++] = tabuleiro[i * 9 + j];
            if ((j + 1) % 3 == 0 && j < 8)
            {
                formatted_board[index++] = ' ';
                formatted_board[index++] = '|';
                formatted_board[index++] = ' ';
            }
            else
            {
                formatted_board[index++] = ' ';
            }
        }
        formatted_board[index++] = '\n';
        if ((i + 1) % 3 == 0 && i < 8)
        {
            snprintf(formatted_board + index, 24, "------+-------+------\n");
            index += strlen("------+-------+------\n");
        }
    }
    formatted_board[index] = '\0';
}

/**
 * Função auxiliar para converter o tabuleiro 2D em uma string unidimensional
 * @param tabuleiro Tabuleiro 2D
 * @param tabuleiro_str String unidimensional para armazenar o tabuleiro
 */
void converter_tabuleiro_para_string(int tabuleiro[LC][LC], char *tabuleiro_str)
{
    int index = 0;
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (tabuleiro[i][j] == 0)
            {
                tabuleiro_str[index++] = '_'; // Representação para valores desconhecidos
            }
            else
            {
                tabuleiro_str[index++] = tabuleiro[i][j] + '0'; // Converte para caractere
            }
        }
    }
    tabuleiro_str[index] = '\0'; // Termina a string
}


/**
 * Função para inicializar os mutexes das células
 */
void inicializaSinc_1()
{
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (pthread_mutex_init(&mutexCelulas[i][j], NULL) != 0)
            {
                perror("Erro ao inicializar mutex");
            }
        }
    }
}

/**
 * Função para destruir os mutexes das células
 */
void destruirSinc_1()
{
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (pthread_mutex_destroy(&mutexCelulas[i][j]) != 0)
            {
                perror("Erro ao destruir mutex");
            }
        }
    }
}

/**
 * Função para inicializar os semaforos para os prod/cons
 * ainda não implementado
 */
void inicializaSinc_2(int tabID)
{
    int vazias = numero_total_vazias(tabID);
    sem_init(&pode_inserir, 0, vazias);
    sem_init(&pode_apagar, 0, 81 - vazias);
}

/**
 * Função para destruir os semaforos para os prod/cons
 * ainda não implementado
 */
void destruirSinc_2()
{
    sem_destroy(&pode_inserir);
    sem_destroy(&pode_apagar);
}

/**
 * Função para inicalizar os semaforos para as prioridades
 * ainda não implementado
 */
void inicializaSinc_3()
{
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (sem_init(&prioridade_1[i][j], 0, 0) != 0)
            {
                perror("Erro ao inicializar os semaforo");
            }
        }
    }
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (sem_init(&prioridade_2[i][j], 0, 0) != 0)
            {
                perror("Erro ao inicializar os semaforo");
            }
        }
    }
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (sem_init(&prioridade_3[i][j], 0, 0) != 0)
            {
                perror("Erro ao inicializar os semaforo");
            }
        }
    }
}

/**
 * Função para destruir os semaforos para as prioridades
 * ainda não implementado
 */
void destruirSinc_3()
{
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (sem_destroy(&prioridade_1[i][j]) != 0)
            {
                perror("Error al destruir el semáforo");
                // Manejo de errores
            }
        }
    }
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (sem_destroy(&prioridade_2[i][j]) != 0)
            {
                perror("Error al destruir el semáforo");
                // Manejo de errores
            }
        }
    }
    for (int i = 0; i < LC; i++)
    {
        for (int j = 0; j < LC; j++)
        {
            if (sem_destroy(&prioridade_3[i][j]) != 0)
            {
                perror("Error al destruir el semáforo");
                // Manejo de errores
            }
        }
    }
}

void semaforo()
{

    pthread_mutex_lock(&mutex_barreira);
    clients_waiting++;
    printf("Clientes esperando %d\n", clients_waiting);
    printf("Aguardando por %d clientes....\n", MAX_CLIENTES);
    if (clients_waiting == MAX_CLIENTES)
    {
        // Libera o semáforo para permitir que todos os 4 threads avancem
        for (int i = 0; i < MAX_CLIENTES; i++)
        {
            printf("Liberta\n");
            sem_post(&sem_barrier);
        }
    }
    pthread_mutex_unlock(&mutex_barreira);

    // Espera até que o semáforo seja sinalizado
    sem_wait(&sem_barrier);
}

void assinalaResolvedores()
{
    pthread_mutex_lock(&mutex_resolvedores_2);
    sem_post(&pode_inserir);
    pthread_mutex_unlock(&mutex_resolvedores_2);
}

void assinalaApagadores()
{

    pthread_mutex_lock(&mutex_apagadores_2);
    sem_post(&pode_apagar);
    pthread_mutex_unlock(&mutex_apagadores_2);
}

/**
 * Função para esperar pelos semaforos com prioridade
 * @param prioridade Prioridade a ser esperada
 * @param linha Linha da célula
 * @param coluna Coluna da célula
 */
void esperaPrioridade(int prioridade, int linha, int coluna, int client_id)
{
    char buffer[BUFFER_SIZE];
    int bloqueado = 1;

    bloqueado = pthread_mutex_trylock(&mutexCelulas[linha][coluna]);
    if (bloqueado != 0)
    {
        sprintf(buffer, "Já bloqueado em (%d,%d)", linha, coluna);
        escrever_log_com_cliente_id(client_id, buffer);

        if (prioridade == 1)
        {
            pthread_mutex_lock(&mutex_Escritura);
            prioridade_1_count[linha][coluna]++;
            pthread_mutex_unlock(&mutex_Escritura);
            sem_wait(&prioridade_1[linha][coluna]);
        }
        else if (prioridade == 2)
        {
            pthread_mutex_lock(&mutex_Escritura);
            prioridade_2_count[linha][coluna]++;
            pthread_mutex_unlock(&mutex_Escritura);
            sem_wait(&prioridade_2[linha][coluna]);
        }
        else if (prioridade == 3)
        {
            pthread_mutex_lock(&mutex_Escritura);
            prioridade_3_count[linha][coluna]++;
            pthread_mutex_unlock(&mutex_Escritura);
            sem_wait(&prioridade_3[linha][coluna]);
        }

        pthread_mutex_lock(&mutexCelulas[linha][coluna]);
    }
}

/**
 * Função para assinalar os semaforos de prioridade
 * @param linha Linha da célula
 * @param coluna Coluna da célula
 */
void assinalaPrioridade(int linha, int coluna, int client_id)
{
    pthread_mutex_lock(&mutex_Escritura);

    char buffer[BUFFER_SIZE];

    sprintf(buffer, "Prioridade 1 de (%d,%d) Count: %d", linha, coluna, prioridade_1_count[linha][coluna]);
    escrever_log_com_cliente_id(client_id, buffer);
    sprintf(buffer, "Prioridade 2 de (%d,%d) Count: %d", linha, coluna, prioridade_2_count[linha][coluna]);
    escrever_log_com_cliente_id(client_id, buffer);
    sprintf(buffer, "Prioridade 3 de (%d,%d) Count: %d", linha, coluna, prioridade_3_count[linha][coluna]);
    escrever_log_com_cliente_id(client_id, buffer);

    if (prioridade_1_count[linha][coluna] > 0)
    {
        sem_post(&prioridade_1[linha][coluna]);
        prioridade_1_count[linha][coluna]--;
        escrever_log_com_cliente_id(client_id, "Prioridade 1 assinalada");
    }
    else
    {
        if (prioridade_2_count[linha][coluna] > 0)
        {
            sem_post(&prioridade_2[linha][coluna]);
            prioridade_2_count[linha][coluna]--;
            escrever_log_com_cliente_id(client_id, "Prioridade 2 assinalada");
        }
        else
        {
            if (prioridade_3_count[linha][coluna] > 0)
            {
                sem_post(&prioridade_3[linha][coluna]);
                prioridade_3_count[linha][coluna]--;
                escrever_log_com_cliente_id(client_id, "Prioridade 3 assinalada");
            }
        }
    }
    pthread_mutex_unlock(&mutex_Escritura);

    pthread_mutex_unlock(&mutexCelulas[linha][coluna]);
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
void enviar_menu_resolvedor(int client_socket, int client_id)
{
    const char *menu =
        "---------- Menu de Sudoku ----------\n"
        "1. Resolver Tabuleiro Total.\n"
        "2. O Servidor revela a Solução e Desistir.\n"
        "3. Desistir.\n"
        "------------------------------------\n";
    send(client_socket, menu, BUFFER_SIZE, 0);
    escrever_log_com_cliente_id(client_id, "Menu enviado com sucesso");
}

/**
 * Função para enviar o menu para o cliente Apagador
 *
 * socket: send
 *
 * @param client_socket Socket do cliente
 * @return void
 */
void enviar_menu_apagador(int client_socket, int client_id)
{
    const char *menu =
        "---------- Menu de Sudoku ----------\n"
        "1. Apagar uma a uma as Celulas do Tabuleiro.\n"
        "2. O Servidor revela a Solução e Desistir.\n"
        "3. Desistir.\n"
        "------------------------------------\n";
    printf("Enviando menu para o cliente\n"); // Debug
    send(client_socket, menu, BUFFER_SIZE, 0);
    escrever_log_com_cliente_id(client_id, "Menu enviado com sucesso");
}

/**
 * Função para escolher e enviar o tabuleiro para o cliente
 *
 * socket: send
 *
 * @param client_socket Socket do cliente
 * @param client_id ID do cliente
 * @param num ID do tabuleiro
 * @return void
 */
void envia_tabuleiro(int client_socket, int client_id, int num)
{
    char tabuleiro_str[LC * LC + 1]; // String temporária para armazenar o tabuleiro unidimensional
    char formatted_tabuleiro[BUFFER_SIZE];

    if (num < 1 || num > SIZE)
    {
        printf("ID do tabuleiro inválido: %d\n", num);
        return;
    }

    converter_tabuleiro_para_string(matriz_of[num - 1], tabuleiro_str); // Converte para string unidimensional
    formatar_tabuleiro(tabuleiro_str, formatted_tabuleiro);             // Formata o tabuleiro

    if (send(client_socket, formatted_tabuleiro, BUFFER_SIZE, 0) < 0)
    {
        escrever_log_com_cliente_id(client_id, "Erro ao enviar o Tabuleiro");
    }
}

/**
 *  Função para enviar solução para o cliente
 *
 * Socket: send
 *
 * @param client_socket Socket do cliente
 * @param client_id ID do cliente
 * @param num ID do tabuleiro
 *
 */
void envia_solucao(int client_socket, int client_id, int num)
{
    char buffer[BUFFER_SIZE];
    char formatted_tabuleiro[BUFFER_SIZE];

    converter_tabuleiro_para_string(matriz_solucao[num - 1], buffer); // Converte para string unidimensional
    formatar_tabuleiro(buffer, formatted_tabuleiro);
    if (send(client_socket, formatted_tabuleiro, BUFFER_SIZE, 0) < 0)
    {
        escrever_log_com_cliente_id(client_id, "Erro ao enviar a solução");
    }
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
void enviar_id_tabuleiro(int client_socket, int num)
{

    if (send(client_socket, &num, sizeof(int), 0) < 0)
    {

        perror("Erro ao enviar ID do tabuleiro");
        escrever_log("Erro ao enviar ID do tabuleiro");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
}

void analisa_apaga_celula(int client_id, int num, int linha, int coluna, char *resposta)
{
    char buffer[BUFFER_SIZE];
    int valor_semaforo;
    sem_getvalue(&pode_apagar, &valor_semaforo);
    sprintf(buffer, "semaforo de apagadores tem: %d", valor_semaforo);
    escrever_log(buffer);
    sem_wait(&pode_apagar);
    if (matriz_of[num - 1][linha][coluna] != 0)
    {
        sprintf(buffer, "Entra na celula:(%d,%d)", linha, coluna);
        escrever_log_com_cliente_id(client_id, buffer);

        matriz_of[num - 1][linha][coluna] = 0;

        snprintf(resposta, BUFFER_SIZE, "OK: Posição (%d, %d) apagada com sucesso.", linha + 1, coluna + 1);
        escrever_log_com_cliente_id(client_id, "Valor apagado com sucesso");

        assinalaResolvedores();

        sem_getvalue(&pode_inserir, &valor_semaforo);
        sprintf(buffer, "Assinalado semaforo de resolvedores, agora tem: %d", valor_semaforo);
        escrever_log(buffer);

    }
    else
    {
        
        snprintf(resposta, BUFFER_SIZE, "ERRO: Posição (%d, %d) já está vazia.", linha + 1, coluna + 1);
        assinalaApagadores();
        escrever_log("Tentativa de apagar errada: Posição já vazia");
    }
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
void recebe_apaga_celula(int client_socket, int client_id)
{
    char buffer[BUFFER_SIZE];
    int num, linha, coluna;
    char resposta[BUFFER_SIZE];

    // Recebe mensagem
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0)
    {
        escrever_log("Erro ao receber dados do cliente");
        perror("[ERRO] Falha no recv");
        return;
    }

    // Extrai os valores num, linha, coluna, e tentativa
    if (sscanf(buffer, "%d,%d,%d", &num, &linha, &coluna) != 3)
    {
        printf("[ERRO] Mensagem inválida recebida do cliente: '%s'\n", buffer);
        escrever_log_com_cliente_id(client_id, "O servidor o pedido de apgar célula inválido:");
        escrever_log_com_cliente_id(client_id, buffer);
        sprintf(buffer, "Mensagem inválida recebida do cliente %d", client_id);
        escrever_log(buffer);
        return;
    }

    sprintf(buffer, "Dados extraídos: Tabuleiro ID=%d, Linha=%d, Coluna=%d", num, linha, coluna);
    escrever_log_com_cliente_id(client_id, buffer);

    analisa_apaga_celula(client_id, num, linha, coluna, resposta);

    // Envia resposta
    if (send(client_socket, resposta, BUFFER_SIZE, 0) < 0)
    {
        perror("[ERRO] Falha ao enviar feedback ao cliente");
        escrever_log("Erro ao enviar feedback ao cliente");
    }
    else
    {
        escrever_log("Feedback enviado com sucesso");
    }
}

/**
 * Função para analisar a tentativa do cliente, Fazer as alterações no tabuleiro e criar o feedback
 * @param client_socket Socket do cliente
 * @param num ID do tabuleiro
 * @param linha Linha da célula
 * @param coluna Coluna da célula
 * @param tentativa Valor da tentativa
 * @param resposta Resposta a ser enviada ao cliente
 */
void analisaTentativa_Exclusao(int client_id, int num, int linha, int coluna, int tentativa, char *resposta)
{
    char buffer[BUFFER_SIZE];

    sprintf(buffer, "Chega ao mutex(%d,%d)", linha, coluna);
    escrever_log_com_cliente_id(client_id, buffer);

    pthread_mutex_lock(&mutexCelulas[linha][coluna]); // Bloqueia o mutex da célula

    sprintf(buffer, "Entra e bloqueia o mutex(%d,%d)", linha, coluna);
    escrever_log_com_cliente_id(client_id, buffer);

    //____________________________________________verificação________________________________________________________
    if (matriz_solucao[num - 1][linha][coluna] == tentativa && matriz_of[num - 1][linha][coluna] == 0)
    {

        matriz_of[num - 1][linha][coluna] = tentativa;
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está correta.", tentativa, linha + 1, coluna + 1);
        escrever_log_com_cliente_id_tentativa_col_row(client_id, tentativa, linha, coluna, "O servidor recebeu tentativa correta na (linha-coluna)");
        escrever_log("Tentativa correta recebida do cliente");
        printf("Cliente %d acertou em %d -> (%d,%d)\n", client_id, tentativa, linha, coluna);
        ler_matrizes_id(matriz_of, num);
    }
    else
    {
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está errada.", tentativa, linha, coluna);
        escrever_log_com_cliente_id_tentativa_col_row(client_id, tentativa, linha, coluna, "O servidor recebeu tentativa errada na (linha-coluna)");
    }
    //_______________________________________________________________________________________________________________
    pthread_mutex_unlock(&mutexCelulas[linha][coluna]); // Desbloqueia o mutex da célula

    sprintf(buffer, "Desbloqueia o mutex(%d,%d)", linha, coluna);
    escrever_log_com_cliente_id(client_id, buffer);
}

/**
 * Função para analisar a tentativa do cliente
 * @param client_id ID do cliente
 * @param num ID do tabuleiro
 * @param linha Linha da célula
 * @param coluna Coluna da célula
 * @param tentativa Valor da tentativa
 * @param resposta Resposta a ser enviada ao cliente
 */
void analisaTentativa_Prod(int client_id, int num, int linha, int coluna, int tentativa, char *resposta)
{
    char buffer[BUFFER_SIZE];
    int valor_semaforo;
    sem_getvalue(&pode_inserir, &valor_semaforo);
    sprintf(buffer, "semaforo de resolvedores tem: %d", valor_semaforo);
    escrever_log(buffer);
    sem_wait(&pode_inserir);
    if (matriz_solucao[num - 1][linha][coluna] == tentativa && matriz_of[num - 1][linha][coluna] == 0)
    {

        sprintf(buffer, "Entra na celula:(%d,%d)", linha, coluna);
        escrever_log_com_cliente_id(client_id, buffer);

        matriz_of[num - 1][linha][coluna] = tentativa;
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está correta.", tentativa, linha + 1, coluna + 1);
        escrever_log_com_cliente_id_tentativa_col_row(client_id, tentativa, linha, coluna, "O servidor recebeu tentativa correta na (linha-coluna)");
        escrever_log("Tentativa correta recebida do cliente");

        printf("Cliente %d acertou em %d -> (%d,%d)\n", client_id, tentativa, linha, coluna);
        ler_matrizes_id(matriz_of, num);

        assinalaApagadores();
        sem_getvalue(&pode_apagar, &valor_semaforo);
        sprintf(buffer, "Assinalado o semaforo de apagadores, agora tem: %d", valor_semaforo);
        escrever_log(buffer);
    }
    else
    {
        assinalaResolvedores();
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está errada.", tentativa, linha, coluna);
        escrever_log_com_cliente_id_tentativa_col_row(client_id, tentativa, linha, coluna, "O servidor recebeu tentativa errada na (linha-coluna)");
    }
}

void analisaTentativa_Prioridade(int client_id, int num, int linha, int coluna, int tentativa, int prioridade, char *resposta)
{
    char buffer[BUFFER_SIZE];

    sprintf(buffer, "Chega ao mutex(%d,%d) com prioridade: %d", linha, coluna, prioridade);
    escrever_log_com_cliente_id(client_id, buffer);

    esperaPrioridade(prioridade, linha, coluna, client_id); // Bloqueia o mutex da célula

    sprintf(buffer, "Entra e bloqueia o mutex(%d,%d) com prioridade: %d", linha, coluna, prioridade);
    escrever_log_com_cliente_id(client_id, buffer);

    //____________________________________________verificação________________________________________________________
    if (matriz_solucao[num - 1][linha][coluna] == tentativa && matriz_of[num - 1][linha][coluna] == 0)
    {

        matriz_of[num - 1][linha][coluna] = tentativa;
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está correta.", tentativa, linha + 1, coluna + 1);
        escrever_log_com_cliente_id_tentativa_col_row(client_id, tentativa, linha, coluna, "O servidor recebeu tentativa correta na (linha-coluna)");
        escrever_log("Tentativa correta recebida do cliente");
        printf("Cliente %d acertou em %d -> (%d,%d)\n", client_id, tentativa, linha, coluna);
        ler_matrizes_id(matriz_of, num);
    }
    else
    {
        snprintf(resposta, BUFFER_SIZE, "Tentativa %d na posição (%d, %d) está errada.", tentativa, linha, coluna);
        escrever_log_com_cliente_id_tentativa_col_row(client_id, tentativa, linha, coluna, "O servidor recebeu tentativa errada na (linha-coluna)");
    }
    //_______________________________________________________________________________________________________________
    assinalaPrioridade(linha, coluna, client_id); // Desbloqueia o mutex da célula

    sprintf(buffer, "Desbloqueia o mutex(%d,%d)", linha, coluna);
    escrever_log_com_cliente_id(client_id, buffer);
}

/**
 * Função para receber a tentativa do cliente e enviar o feedback
 *
 * Socket: recv - send
 *
 * @param client_socket Socket do cliente
 * @param client_id ID do cliente
 * @param prioridade Prioridade do Jogador
 * @return void
 */
void recebe_tentativa_e_envia_feedback(int client_socket, int client_id, int prioridade)
{
    char buffer[BUFFER_SIZE];
    char resposta[BUFFER_SIZE];
    int num, linha, coluna, tentativa;

    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0)
    {
        escrever_log_com_cliente_id(client_id, "Erro ao receber tentativa do cliente");
        perror("[ERRO] Falha no recv");
        return;
    }
    escrever_log_com_cliente_id(client_id, "Tentativa recebida com sucesso");

    buffer[bytes_received] = '\0'; // Garante que a mensagem recebida é válida

    // Extrai os valores num, linha, coluna, e tentativa
    if (sscanf(buffer, "%d %d %d %d", &num, &linha, &coluna, &tentativa) != 4)
    {
        printf("[ERRO] Mensagem inválida recebida do cliente: '%s'\n", buffer);
        escrever_log_com_cliente_id_tentativa(client_id, tentativa, "O servidor recebeu a tentativa");
        escrever_log_com_cliente_id_tentativa_col_row(client_id, tentativa, linha, coluna, "O servidor recebeu tentativa na (linha-coluna)");
        sprintf(buffer, "Mensagem inválida recebida do cliente %d", client_id);
        escrever_log(buffer);
        return;
    }

    // SINCONIZAÇÃO E ANALISE DA TENTATIVA ______________________________________________________________________________________
    if (modoJogo == 1)
    {
        analisaTentativa_Exclusao(client_id, num, linha, coluna, tentativa, resposta);
    }
    else if (modoJogo == 2)
    {
        analisaTentativa_Prod(client_id, num, linha, coluna, tentativa, resposta);
    }
    else if (modoJogo == 3)
    {
        analisaTentativa_Prioridade(client_id, num, linha, coluna, tentativa, prioridade, resposta);
    }
    //____________________________________________________________________________________________________

    // ENVIA O FEEDBACK AO CLIENTE
    if (send(client_socket, resposta, BUFFER_SIZE, 0) < 0)
    {
        perror("[ERRO] Falha ao enviar feedback ao cliente");
        escrever_log("Erro ao enviar feedback ao cliente");
    }
    
}

/**
 * Função para escolher o modo de jogo
 *
 * @param modoJogo Modo de jogo escolhido : int *
 * @return void
 */
void escolhe_modo_de_Jogo()
{
    printf("Escolha o modo de jogo:\n");
    printf("1 - Modo Clásico cooperativo\n");
    printf("2 - Modo Apagadores e Resolvedores\n");
    printf("3 - Modo de Prioridades\n");

    printf("Insira um número ('0' para encerrar): ");
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, stdin);

    if (strlen(buffer) == 1)
    {
        printf("Por favor, insira um número válido.\n");
        return;
    }

    // Remove o newline que `fgets` deixa no buffer
    buffer[strcspn(buffer, "\n")] = 0;

    modoJogo = atoi(buffer); // Converte a entrada para inteiro

    switch (modoJogo)
    {
    case 1:
        printf("Modo de jogo 1: Modo Clásico cooperativo selecionado.\n");
        escrever_log("Modo de jogo 1: Modo Clásico cooperativo selecionado");
        break;
    case 2:
        printf("Modo de jogo 2: Modo Apagadores e Resolvedores selecionado.\n");
        escrever_log("Modo de jogo 2: Modo Apagadores e Resolvedores selecionado");
        break;

    case 3:
        printf("Modo de jogo 3: Modo de Prioridades selecionado.\n");
        escrever_log("Modo de jogo 3: Modo de Prioridades selecionado");
        break;
    case 0:
        printf("A encerrar...\n");
        escrever_log("Jogador Saiu");
        break;
    }

    return;
}

/**
 * Função para lidar com o sinal SIGINT (Ctrl + C)
 * @param sig Sinal recebido
 * @return void
 */
void handle_sigint(int sig)
{
    printf("\nSIGINT received. Closing server socket...\n");
    // Destuição dos semaforos e mutexes

    sem_destroy(&sem_barrier);
    if (modoJogo == 1)
    {
        destruirSinc_1();
    }
    else if (modoJogo == 2)
    {
        destruirSinc_2();
    }
    else if (modoJogo == 3)
    {
        destruirSinc_3();
    }

    // Após sair do loop principal
    escrever_log("Servidor encerrado");
    close(server_socket);
    reset_user_id();
    exit(0);
}



/**
 * Função para lidar com o cliente Resolvedor
 *
 * @param client_socket Socket do cliente
 * @return void
 */
void *handle_client(void *args)
{
    client_args_t *client_args = (client_args_t *)args;
    int sock = client_args->client_socket;
    int rol_Jogador = client_args->rol_Jogador;
    int prioridade_Jogador = client_args->prioridade;
    int num = client_args->num;

    free(client_args);
    char buffer[BUFFER_SIZE];
    int opcao, client_id;

    if (recv(sock, &client_id, sizeof(int), 0) <= 0)
    {
        escrever_log("Erro ao receber ID do cliente");
        printf("Error receiving client ID: %d\n", client_id);
        close(sock);
        return NULL;
    }

    printf("Novo cliente conectado com ID: %d\n", client_id);
    escrever_log_com_cliente_id(client_id, "Novo cliente conectado");

    enviar_id_tabuleiro(sock, num);

    int running = 1;
    while (running)
    {

        envia_tabuleiro(sock, client_id, num);

        //_____________________________________________________________
        if (rol_Jogador == 2 && modoJogo == 2)
        {

            enviar_menu_apagador(sock, client_id);
        }
        else
        {
            enviar_menu_resolvedor(sock, client_id);
        }

        printf("Sending menu to client: %d\n", client_id);

        printf("Waiting for option from client: %d\n", client_id);

        // Receive the client's option
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0)
        {
            escrever_log("Erro ao receber opção do cliente");
            perror("Error receiving data from client");
            printf("Client %d disconnected due to error.\n", client_id);
            escrever_log("Client disconnected due to receive error");
            break;
        }
        else if (bytes_received == 0)
        {
            escrever_log("Cliente desconectado");
            printf("Client %d disconnected.\n", client_id);
            escrever_log("Client disconnected");
            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received data

        // Convert received message to an integer option
        opcao = atoi(buffer); // Try parsing the received data as an integer

        // Handle the selected option
        switch (opcao)
        {
        case 1:
            semaforo();
            if (rol_Jogador == 2 && modoJogo == 2) // caso for apagador do modo de jogo 2
            {
                // APAGADOR ESCOLHEU APAGAR O TABULEIRO UM A UM AS CELULAS
                printf("Client %d selected to solve one cell\n", client_id);
                escrever_log_com_cliente_id(client_id, "Selecionou apagar uma a uma as células");

                int num_vazias = numero_total_vazias(num);
                printf("Número total de casas vazias: %d\n", num_vazias);
                while (num_vazias < 81 && num_vazias > 0)
                {


                    recebe_apaga_celula(sock, client_id);

                    envia_tabuleiro(sock, client_id, num);
                    num_vazias = numero_total_vazias(num);
                    printf("Número total de casas vazias: %d\n", num_vazias);
                }
            }
            else
            {
                // O RESOLVEDOR ESCOLHEU RESOLVER O TABULEIRO UM A UM
                printf("Client %d selected to solve one cell\n", client_id);
                escrever_log_com_cliente_id(client_id,"Cliente selecionou resolver uma célula");

                int total_vazias = numero_total_vazias(num);
                printf("Número total de casas vazias: %d\n", total_vazias);
                while (total_vazias > 0 && total_vazias < 81)
                {
                    
                    // Enquanto houver casas vazias, processa as tentativas recebidas

                    recebe_tentativa_e_envia_feedback(sock, client_id, prioridade_Jogador);

                    envia_tabuleiro(sock, client_id, num);
                    total_vazias = numero_total_vazias(num);
                    printf("Número total de casas vazias: %d\n", total_vazias);
                }
            }
            escrever_log_com_cliente_id(client_id, "Jogo acabado com sucesso");
            running = 0; // Exit loop
            break;

        case 2:
            printf("Client %d requested Full Solution from Server.\n", client_id);
            escrever_log_com_cliente_id(client_id, "Cliente selecionou solução completa");

            envia_solucao(sock, client_id, num);
            escrever_log("Solução enviada com sucesso");
            running = 0; // Exit loop

            // Envia a solução completa
            break;

        case 3:
            printf("Cliente %d desistiu do jogo.\n", client_id);
            escrever_log_com_cliente_id(client_id,"Cliente desistiu do jogo");
            
            running = 0; // Exit loop
            break;

        default:
            printf("Cliente %d escolheu uma opção invalida: %d\n", client_id, opcao);
            escrever_log("Cliente selecionou uma opção inválida");

            break;
        }
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

//----------------------------

//----------------------------
int main(int argc, char *argv[])
{

    // INICIALIZAÇÃO DO SERVIDOR
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if (argc < 2)
    {
        escrever_log("Erro ao abrir o ficheiro de configuração");
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);
        return 1;
    }

    char ficheiro_jogos[100], ficheiro_solucoes[100];
    ler_configuracao(argv[1], ficheiro_jogos, ficheiro_solucoes, &porta);

    transforma_matriz(ficheiro_jogos);
    transforma_matriz_solucao(ficheiro_solucoes);
    ler_matrizes(matriz_of);
    printf("----------");
    ler_matrizes(matriz_solucao);
    //_________________________________________________________________________

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // CRIAÇÃO DO SOCKET DO SERVIDOR
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    escrever_log("Socket do servidor criado com sucesso");
    if (server_socket < 0)
    {
        escrever_log("Erro ao criar socket do servidor");
        perror("Erro ao criar socket do servidor");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(porta);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        escrever_log("Erro no bind");
        perror("Erro no bind");
        close(server_socket);
        return 1;
    }
    escrever_log("Bind feito com sucesso");

    if (listen(server_socket, 10) < 0)
    {
        escrever_log("Erro ao ouvir no socket");
        perror("Erro ao ouvir no socket");
        close(server_socket);
        return 1;
    }
    escrever_log("Servidor pronto para ouvir conexões");

    escolhe_modo_de_Jogo();

    //_________________________________________________________________

    // INICIALIZAÇÃO DOS MUTEX
    srand(time(NULL));

    int tabID = rand() % 4 + 1;

    if (modoJogo == 1)
    {
        inicializaSinc_1();
    }
    else if (modoJogo == 2)
    {
        inicializaSinc_2(tabID);
    }
    else if (modoJogo == 3)
    {
        inicializaSinc_3();
    }

    sem_init(&sem_barrier, 0, 0);
    //_________________________________________________________________

    while (1)
    {
        int rol_Jogador;
        int prioridade_Jogador;
        char buffer[BUFFER_SIZE];

        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0)
        {
            escrever_log("Erro ao aceitar conexão");
            perror("Erro ao aceitar conexão");
            continue;
        }
        escrever_log("Conexão aceita com sucesso");

        //_________________________________________________________________
        pthread_mutex_lock(&clients_mutex);
        if (num_clients_sessao >= 20)
        {
            escrever_log("Número máximo de clientes atingido");
            close(client_socket);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        escrever_log("Número de clientes na sessão atualizado");
        num_clients_sessao++;
        pthread_mutex_unlock(&clients_mutex);
        //_________________________________________________________________

        int *new_sock = malloc(sizeof(int));
        if (new_sock == NULL)
        {
            escrever_log("Erro ao alocar memória para new_sock");
            perror("Erro ao alocar memória para new_sock");
            close(client_socket);
            continue;
        }
        *new_sock = client_socket;

        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
        {
            escrever_log("Erro ao receber dados do cliente");
            perror("Erro ao receber dados que do cliente");
            free(new_sock);
            close(client_socket);
            continue;
        }

        buffer[bytes_received] = '\0'; // Assegura-se de que a mensagem recebida seja terminada com '\0'
        // Extraer os inteiros do buffer
        if (sscanf(buffer, "%d,%d", &rol_Jogador, &prioridade_Jogador) != 2)
        {
            printf("rol do Jogador: %d, prioridade do jogador: %d\n", rol_Jogador, prioridade_Jogador);
            printf("Erro ao analisar os dados recebidos: '%s'\n", buffer);
            free(new_sock);
            close(client_socket);
            continue;
        }

        send(client_socket, &modoJogo, sizeof(int), 0);

        pthread_t tid;
        pthread_mutex_lock(&clients_mutex_board_2); // Bloqueia o primeiro mutex

        client_args_t *client_args = malloc(sizeof(client_args_t));
        client_args->client_socket = client_socket;
        client_args->rol_Jogador = rol_Jogador;
        client_args->prioridade = prioridade_Jogador;
        client_args->num = tabID;

        if (pthread_create(&tid, NULL, handle_client, (void *)client_args) != 0)
        {
            escrever_log("Erro ao criar thread para o cliente");
            perror("Erro ao criar thread para o cliente ");
            free(new_sock);
            close(client_socket);
            pthread_mutex_unlock(&clients_mutex_board_2); // Libera mutex no caso de erro
            continue;
        }
        pthread_mutex_unlock(&clients_mutex_board_2);
    }
    
    return 0;
}
