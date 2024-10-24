#include "header.h"
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

// Função para, atraves da config.txt definir os caminhos dos ficheiros de jogos e de soluções
void ler_configuracao(char *config_path, char *ficheiro_jogos, char *ficheiro_solucoes) {//recebe o caminho do ficheiro de configuração
    FILE *config = fopen(config_path, "r");
    if (config == NULL) {
        printf("Erro ao abrir o ficheiro de configuração.\n");
        exit(1);
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), config)) { 
        // Divide a linha pelo caractere '='
        char *token = strtok(linha, "=");  
    
        // Verifica se o token é "ficheiro_jogos"
        if (strcmp(token, "ficheiro_jogos") == 0) {  
            // Pega o próximo token (após o '=') que é o \n
            token = strtok(NULL, "\n");  
            // Copia o valor do token para a variável ficheiro_jogos
            strcpy(ficheiro_jogos, token);  
        } 
        // Verifica se o token é "ficheiro_solucoes"
        else if (strcmp(token, "ficheiro_solucoes") == 0) {  
            // Pega o próximo token (após o '=')
            token = strtok(NULL, "\n");  
            // Copia o valor do token para a variável ficheiro_solucoes
            strcpy(ficheiro_solucoes, token);  
        } 
        
    } 

    // Fecha o ficheiro de configuração após a leitura
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


void Menu(int tabuleiro[TAMANHO][TAMANHO], const char *nome_ficheiro,const char *nome_ficheiro_solucoes) {
    int dificuldade;
    printf("Escolha o nível de dificuldade (1 = Fácil, 2 = Médio, 3 = Difícil): ");
    scanf("%d", &dificuldade);
    while (getchar() != '\n'); // Limpar o buffer de entrada

    gerar_sudoku(tabuleiro, dificuldade);
    int idTabuleiro = getNumSudokus(nome_ficheiro) + 1;
    misturarSudoku(tabuleiro); // Corrigido o nome da função
    salvar_tabuleiro(nome_ficheiro, tabuleiro, idTabuleiro); // Corrigido para usar o nome do ficheiro corretamente


    int tabuleiroResolvido[TAMANHO][TAMANHO];
    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            tabuleiroResolvido[i][j] = tabuleiro[i][j];
        }
    }
    resolver_sudoku(tabuleiroResolvido,  0, 0);
    gravar_solucao(tabuleiroResolvido, nome_ficheiro_solucoes, idTabuleiro);

    printf("Tabuleiro de Sudoku gerado.\n");

    while (1) {
        int opcao;
         time_t mytime = time(NULL);
        char *timestamp = ctime(&mytime);
        timestamp[strlen(timestamp) - 1] = 0; // Remove a nova linha do timestamp
        imprimir_tabuleiro(tabuleiro);
        // Exibir informações do jogo
        printf("ID jogo a decorrer -> \n");
        printf("Hora de inicio  -> \n");
        printf("Tempo decorrido -> \n");
        printf("Numero de Tarefas -> \n");

        printf("----------Menu----------\n");
        printf("1. Inserir.\n");
        printf("2. O Servidor revela a Solução.\n");
        printf("3. O Cliente resolve a Solução.\n");
        printf("4. Desistir.\n");
        printf("------------------------\n");
        printf("Selecione o que deseja: ");
        scanf("%d", &opcao);
        while (getchar() != '\n'); // Limpar o buffer de entrada

        switch (opcao) {
            case 1:
                escrever_log("Cliente clicou na opcao 1 do menu");
                jogar_sudoku(tabuleiro);
                break;
            case 2:
                escrever_log("Cliente clicou na opcao 2 do menu");
                lerSolucao(tabuleiro, nome_ficheiro_solucoes, idTabuleiro);
                imprimir_tabuleiro(tabuleiro);
                escrever_log("Cliente saiu do Sodoku");
                return;
                break;
            case 3:
                resolver(tabuleiro);
                imprimir_tabuleiro_cliente(tabuleiro);
                gravar_solucao(tabuleiro, nome_ficheiro_solucoes, idTabuleiro);
                return; 
                break;
            case 4:
                escrever_log("Cliente clicou na opcao 3 do menu");
                printf("Desistiu do jogo. A sair...\n");
                printf("Até a proxima.");
                escrever_log("Jogador desistiu do Sodoku");
                return;
                break; 
            default:
                printf("Opção inválida! Tente novamente.\n");
                escrever_log("O Jogador selecionou uma opcao invalida no Menu");
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <ficheiro de configuração>\n", argv[0]);//1 argumento passado na linha de comando
        return 1;
    }

    char ficheiro_jogos[256];
    char ficheiro_solucoes[256];

    // Ler configuração
    ler_configuracao(argv[1], ficheiro_jogos, ficheiro_solucoes); //2argumento passado na linha de comando
    //devolvendo o caminho dos ficheiros no sistema de configuração
    printf("Ficheiro de jogos: %s\n", ficheiro_jogos);
    printf("Ficheiro de soluções: %s\n", ficheiro_solucoes);



    int tabuleiro[TAMANHO][TAMANHO];
    


    srand(time(NULL)); // Inicializa a semente para números aleatórios

    Menu(tabuleiro, ficheiro_jogos, ficheiro_solucoes); 

    return 0;
}
