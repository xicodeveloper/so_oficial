#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define TAMANHO 1024

int num_clients_sessao = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Função para registrar logs do servidor
void escrever_log(const char *mensagem) {
    FILE *f = fopen("./logs/log.txt", "a");
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
int getNumSudokus(const char *nome_ficheiro) {
    FILE *file = fopen(nome_ficheiro, "r");
    if (file == NULL) {
        printf("Erro ao abrir o ficheiro.\n");
        return -1;
    }

    int num_sudokus = 0;
    char linha[256];
    while (fgets(linha, sizeof(linha), file)) { 
        num_sudokus++;
    } 
    fclose(file);
    return num_sudokus;
}

//permite guardar o tabuleiro no  ficheiro jogos.txt
void salvar_tabuleiro(const char *nome_ficheiro, int tabuleiro[TAMANHO][TAMANHO], int id_Tabuleiro) {
    FILE *f = fopen(nome_ficheiro, "a"); // Abre o ficheiro para acrescentar
    if (f == NULL) {
        printf("Erro ao abrir o ficheiro %s para escrita.\n", nome_ficheiro);
        return;
    }
        
    // Grava o tabuleiro no ficheiro
    
    fprintf(f, "%d,",id_Tabuleiro);
    for (int i = 0; i < TAMANHO; i++) {//percorre toda a matriz
        for (int j = 0; j < TAMANHO; j++) {
            if (tabuleiro[i][j] == 0) {
                fprintf(f, "_"); // Representa espaços vazios com "_"
            } else {
                fprintf(f, "%d", tabuleiro[i][j]); //coloca numeros no ficheiro_jogos
            }
        }

    }
    escrever_log("O Servidor guardou o tabuleiro");
    fprintf(f,"\n");
    fclose(f); // Fecha o ficheiro



}

// Função para gravar a solução no ficheiro solucoes.txt
void gravar_solucao(int tabuleiro[TAMANHO][TAMANHO], const char *nome_ficheiro_solucoes, int idTabuleiro) {
    FILE *ficheiro = fopen(nome_ficheiro_solucoes, "a");
      time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = 0; // Remove a nova linha do timestamp
    if (ficheiro == NULL) {
        printf("Erro ao abrir o ficheiro!\n");
        return;
    }
    escrever_log("O Servidor guardou a solucao");
    

    // Escrever a solução no ficheiro numa linha única
    fprintf(ficheiro, "%d,", idTabuleiro);
    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            fprintf(ficheiro, "%d", tabuleiro[i][j]);
        }
    }
    fprintf(ficheiro, "\n");
    fclose(ficheiro);
}


void lerSolucao(int tabuleiro[TAMANHO][TAMANHO], const char* nome_ficheiro_solucoes, int idTabuleiro){
    FILE *ficheiro = fopen(nome_ficheiro_solucoes, "r");
    if (ficheiro == NULL) {
        printf("Erro ao abrir o ficheiro de soluções.\n");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), ficheiro)) { 
        // Divide a linha pelo caractere ','
        char *token = strtok(linha, ",");  
    
        // Verifica se o token é o id do tabuleiro
        if ( atoi(token) == idTabuleiro) {  
            // Pega o próximo token (após o ',') que é o \n
            token = strtok(NULL, "\n");  
            // Copia o valor do token para a tabela
            char *solucao = token;

            for(int i = 0; i < TAMANHO; i++ ){
                for(int j = 0; j < TAMANHO; j++ ){
                    
                        tabuleiro[i][j] = solucao[i * TAMANHO + j] - '0';
                    
                }
            }

            
        } 
        
        
    } 

    // Fecha o ficheiro de configuração após a leitura
    fclose(ficheiro); 
  
    escrever_log("Solução lida com sucesso");
}

// Função para printar o tabuleiro e salvar no ficheiro log.txt
void imprimir_tabuleiro(int tabuleiro[TAMANHO][TAMANHO]) {
    
    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = 0; // Remove a nova linha do timestamp
    escrever_log("O Servidor mostrou o tabuleiro ao cliente");

    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            if (j == 3 || j == 6) {
                printf(" | ");
            }
            if (tabuleiro[i][j] == 0) {
                printf(" _ ");
            } else {
                printf("%d ", tabuleiro[i][j]);
            }
        }
        printf("\n");
        if (i == 2 || i == 5) {
            printf("-------------------------\n");
        }
    }
    printf("\n");
    
}

// Função para verificar se é seguro colocar um número no tabuleiro
int pode_colocar(int tabuleiro[TAMANHO][TAMANHO], int linha, int col, int num) {

    // Verifica se o número já existe na linha
    for (int x = 0; x < TAMANHO; x++) {
        if (tabuleiro[linha][x] == num) {
            return 0; // Se o número já existir na linha, retorna 0 (não é seguro)
        }
    }

    // Verifica se o número já existe na coluna
    for (int x = 0; x < TAMANHO; x++) {
        if (tabuleiro[x][col] == num) {
            return 0; // Se o número já existir na coluna, retorna 0 (não é seguro)
        }
    }

    // Verifica se o número já existe na subgrade 3x3
    int startLinha = linha - linha % 3; // Calcula o início da linha da subgrade 3x3
    int startCol = col - col % 3; // Calcula o início da coluna da subgrade 3x3
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (tabuleiro[i + startLinha][j + startCol] == num) {
                return 0; // Se o número já existir na subgrade 3x3, retorna 0 (não é seguro)
            }
        }
    }
    return 1; // Se o número não existir na linha, coluna ou subgrade 3x3, retorna 1 (é seguro)
}

// funçao para resolver o sudoku
int resolver_sudoku(int tabuleiro[TAMANHO][TAMANHO], int linha, int col) {
    // Verifica se chegou ao final do tabuleiro ao estares completado todas as linhas e colunas
    if (linha == TAMANHO - 1 && col == TAMANHO) {
        return 1; // Sudoku resolvido
    }

    // Move para a próxima linha se a coluna atual for igual ao tamanho do tabuleiro
    if (col == TAMANHO) {
        linha++;
        col = 0;
    }

    // Se a célula atual já estiver preenchida, move para a próxima coluna
    if (tabuleiro[linha][col] != 0) {
        return resolver_sudoku(tabuleiro, linha, col + 1);
    }

    // Tenta preencher a célula atual com números de 1 a 9
    for (int num = 1; num <= 9; num++) {
        // Verifica se é seguro colocar o número na célula atual
        if (pode_colocar(tabuleiro, linha, col, num)) {
            tabuleiro[linha][col] = num; // Coloca o número na célula
            // Continua para a próxima célula
            if (resolver_sudoku(tabuleiro, linha, col + 1)) {
                return 1; // Sudoku resolvido
            }
            tabuleiro[linha][col] = 0; // Remove o número se não levar a uma solução
        }
    }
    return 0; // Retorna 0 se não for possível resolver o Sudoku
}

// Função para gerar um tabuleiro de Sudoku parcialmente preenchido
void gerar_sudoku(int tabuleiro[TAMANHO][TAMANHO], int dificuldade) {
    // Inicializa o tabuleiro vazio
    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            tabuleiro[i][j] = 0;
        }
    }

    // Gerar uma solução completa de Sudoku
    resolver_sudoku(tabuleiro, 0, 0);

    // Definir a quantidade de números removidos com base na dificuldade
    int num_removidos;
    if (dificuldade == 1) {
        num_removidos = 30; //Fácil
    } else if (dificuldade == 2) {
        num_removidos = 40; //Médio
    } else {
        num_removidos = 50; //Difícil
    }

    // Remover os números do tabuleiro para criar um tabuleiro parcialmente preenchido
    while (num_removidos > 0) {
        int linha = rand() % TAMANHO;
        int col = rand() % TAMANHO;
        if (tabuleiro[linha][col] != 0) {
            tabuleiro[linha][col] = 0;
            num_removidos--;
        }
    }
   
    escrever_log("O Servidor gera o Soduku");
}

void mudarLinhas(int tabuleiro[TAMANHO][TAMANHO], int fila1, int fila2) {
    //troca uma linha por outra

    for (int j = 0; j < TAMANHO; j++) {
        int temp = tabuleiro[fila1][j];
        tabuleiro[fila1][j] = tabuleiro[fila2][j];
        tabuleiro[fila2][j] = temp;
    }
}

void mudarColunas(int tabuleiro[TAMANHO][TAMANHO], int col1, int col2) {
    //troca uma coluna por outra
    for (int i = 0; i < TAMANHO; i++) {
        int temp = tabuleiro[i][col1];
        tabuleiro[i][col1] = tabuleiro[i][col2];
        tabuleiro[i][col2] = temp;
    }
}
void misturarSudoku(int tabuleiro[TAMANHO][TAMANHO]) {
    for (int i = 0; i < 3; i++) {
        // trocar filas dentro de cada subgrade de 3x3
        int fila1 = i * 3 + rand() % 3; // Seleciona uma linha aleatória dentro da subgrade
        int fila2 = i * 3 + rand() % 3; // Seleciona outra linha aleatória dentro da subgrade
        mudarLinhas(tabuleiro, fila1, fila2); // Troca as duas linhas selecionadas

        // trocar colunas dentro de cada subgrade de 3x3
        int col1 = i * 3 + rand() % 3; // Seleciona uma coluna aleatória dentro da subgrade
        int col2 = i * 3 + rand() % 3; // Seleciona outra coluna aleatória dentro da subgrade
        mudarColunas(tabuleiro, col1, col2); // Troca as duas colunas selecionadas
    }
}




// Função para verificar se o Sudoku está completo e válido
int verificar_vitoria(int tabuleiro[TAMANHO][TAMANHO]) {
    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            if (tabuleiro[i][j] == 0 || !pode_colocar(tabuleiro, i, j, tabuleiro[i][j])) {
                return 0;
            }
        }
    }
    escrever_log("O Servidor verifica se ganhou");
    return 1;
}



// Função para enviar o menu para o cliente
void enviar_menu(int client_socket) {
    char buffer[BUFFER_SIZE]; // Definindo o buffer com o tamanho necessário

    // Escrevendo o menu no buffer
    snprintf(buffer, sizeof(buffer),
        "---------- Menu de Sudoku ----------\n"
        "1. Criar Tabuleiro.\n"
        "2. O Servidor revela a Solução.\n"
        "3. O Cliente resolve a Solução.\n"
        "4. Desistir.\n"
        "------------------------------------\n");

    // Enviando o conteúdo do buffer ao cliente
    send(client_socket, buffer, strlen(buffer), 0);

    // Limpando o buffer
    memset(buffer, 0, sizeof(buffer));
}


// Função para enviar o horário de início da conexão para o cliente
void enviar_inicio_conecao(int client_socket) {
    time_t inicio_conexao = time(NULL);
    struct tm *info_tempo = localtime(&inicio_conexao);
    char mensagem_inicio[BUFFER_SIZE];
    strftime(mensagem_inicio, sizeof(mensagem_inicio), "Hora de início: %H:%M:%S\n", info_tempo);
    send(client_socket, mensagem_inicio, strlen(mensagem_inicio), 0);
}

// Função para receber o ID do cliente
int receber_id(int sock) {
    int client_id;
    if (recv(sock, &client_id, sizeof(client_id), 0) <= 0) {
        perror("Erro ao receber ID do cliente");
        close(sock);
        return 0;
    }
    printf("Novo cliente conectado com ID: %d\n", client_id);
    // Função de log (não incluída no código)
    escrever_log("Novo cliente conectado");
    return client_id;
}
int receber_opcao(int client_socket, int client_id) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received <= 0) {
        printf("Cliente %d desconectado\n", client_id);
        escrever_log("Cliente desconectado");
        return -1;  // Indicate disconnection or error
    }

    buffer[bytes_received] = '\0';
    return atoi(buffer);  // Convert the received option to integer
}
void pedir_opcao(int client_socket){
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "Escolha uma opção: ");
    send(client_socket, buffer, strlen(buffer), 0);
}
void enviar_resposta(int client_socket, int client_id, int opcao) {
    char buffer[BUFFER_SIZE];

    // Handle the option and prepare response
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
            return; 
        default:
            printf("Cliente %d selecionou uma opção inválida.\n", client_id);
            strcpy(buffer, "Opção inválida! Tente novamente.\n");
            break;
    }

    // Send the prepared response
    send(client_socket, buffer, strlen(buffer), 0);
}

void *handle_client(void *client_socket) {
    int sock = *(int*)client_socket;
    free(client_socket);

    int client_id = receber_id(sock);
    if (client_id == 0) {
        return NULL;  // Connection failed
    }

    enviar_inicio_conecao(sock);
    enviar_menu(sock);
    
    while (1) {
        pedir_opcao(sock);
        int opcao= receber_opcao( sock,  client_id);
        enviar_resposta(sock,  client_id, opcao);
       // int opcao = receber_opcao(sock, client_id);
        //if (opcao == -1 || opcao == 4) {  // Disconnection or exit option
          //  break;
        //}
        //enviar_resposta(sock, client_id, opcao);
    }

    close(sock);
    printf("Conexão com cliente %d encerrada\n", client_id);

    pthread_mutex_lock(&clients_mutex);
    num_clients_sessao--;
    pthread_mutex_unlock(&clients_mutex);

    printf("Clientes atuais: %d\n", num_clients_sessao);
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
         // Incrementa o contador de clientes de forma sincronizada
        pthread_mutex_lock(&clients_mutex);
        num_clients_sessao++;
        printf("Clientes atuais: %d\n", num_clients_sessao);
        pthread_mutex_unlock(&clients_mutex);

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
