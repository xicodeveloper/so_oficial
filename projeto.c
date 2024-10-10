#include "header.h"
#define TAMANHO 9

// Inicialização do mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


// Função para verificar se é seguro colocar um número no tabuleiro
int pode_colocar(int tabuleiro[TAMANHO][TAMANHO], int linha, int col, int num) {
    for (int x = 0; x < TAMANHO; x++) {
        if (tabuleiro[linha][x] == num) {
            return 0;
        }
    }

    // coluna
    for (int x = 0; x < TAMANHO; x++) {
        if (tabuleiro[x][col] == num) {
            return 0;
        }
    }

    // subgrade 3x3
    int startLinha = linha - linha % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (tabuleiro[i + startLinha][j + startCol] == num) {
                return 0;
            }
        }
    }
    return 1;
}

// para resolver só chamar a funçao
int resolver_sudoku(int tabuleiro[TAMANHO][TAMANHO], int linha, int col) {
    if (linha == TAMANHO - 1 && col == TAMANHO) {
        return 1;
    }

    if (col == TAMANHO) {
        linha++;
        col = 0;
    }

    if (tabuleiro[linha][col] != 0) {
        return resolver_sudoku(tabuleiro, linha, col + 1);
    }

    for (int num = 1; num <= 9; num++) {
        if (pode_colocar(tabuleiro, linha, col, num)) {
            tabuleiro[linha][col] = num;
            if (resolver_sudoku(tabuleiro, linha, col + 1)) {
                return 1;
            }
            tabuleiro[linha][col] = 0;
        }
    }
    return 0;
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

    // Remover os números do tabuleiro
    while (num_removidos > 0) {
        int linha = rand() % TAMANHO;
        int col = rand() % TAMANHO;
        if (tabuleiro[linha][col] != 0) {
            tabuleiro[linha][col] = 0;
            num_removidos--;
        }
    }
}

// Função para printar o tabuleiro e salvar no ficheiro log.txt
void imprimir_tabuleiro(int tabuleiro[TAMANHO][TAMANHO]) {
    FILE *fe = fopen("log.txt", "a"); // abre o ficheiro "log.txt" para acrescentar
    time_t mytime = time(NULL);
    char *timestamp = ctime(&mytime);
    timestamp[strlen(timestamp) - 1] = 0; // Remove a nova linha do timestamp

    if (fe == NULL) {
        printf("Erro ao abrir o ficheiro log.txt\n");
        return;
    }

    fprintf(fe, "Tabuleiro de Sudoku (%s):\n", timestamp);
    fclose(fe);

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

// Função para verificar se o Sudoku está completo e válido
int verificar_vitoria(int tabuleiro[TAMANHO][TAMANHO]) {
    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            if (tabuleiro[i][j] == 0 || !pode_colocar(tabuleiro, i, j, tabuleiro[i][j])) {
                return 0;
            }
        }
    }
    return 1;
}

// Função de jogo
void jogar_sudoku(int tabuleiro[TAMANHO][TAMANHO]) {
    int linha, col, num;
    int erros = 0;

    while (1) {
        imprimir_tabuleiro(tabuleiro);

        // Verificar vitória
        if (verificar_vitoria(tabuleiro)) {
            printf("Parabéns! Completou o Sudoku corretamente!\n");
            FILE *fe = fopen("log.txt", "a");
            if (fe != NULL) {
                fprintf(fe, "Parabéns! Completou o Sudoku corretamente!\n");
                fclose(fe);
            }
            break;
        }

        printf("Insira a linha (1-9), coluna (1-9) e o número (1-9) ou 0 0 0 para sair: ");
        scanf("%d %d %d", &linha, &col, &num);
      
            

        if (linha == 0 && col == 0 && num == 0) {
            printf("Jogo terminado. Até à próxima!\n");
            FILE *fe = fopen("log.txt", "a");
            if (fe != NULL) {
                fprintf(fe, "Jogo terminado. Até à próxima!\n");
                fclose(fe);
            }
            break;
        }

        linha--; // Ajustar  índice 0
        col--;   // Ajustar  índice 0

        // Verifica se a posição está vazia e o número é válido
        if (tabuleiro[linha][col] == 0 && pode_colocar(tabuleiro, linha, col, num)) {
            tabuleiro[linha][col] = num;
        } else {
             FILE *fe = fopen("log.txt", "a");
            fprintf(fe,"Movimento inválido!\n");
            fclose(fe);
            erros++;
            if (erros >= 3) {
                printf("Perdeu o jogo. Excedeu as 3 tentativas.\n");
                FILE *fe = fopen("log.txt", "a");
                if (fe != NULL) {
                    fprintf(fe, "Perdeu o jogo. Excedeu as 3 tentativas.\n");
                    fclose(fe);
                }
                break;
            }
        }
    }
}

int main(void) {
    int tabuleiro[TAMANHO][TAMANHO];
    int dificuldade;

    srand(time(NULL));

    // Pedir o nível de dificuldade ao jogador
    printf("ID jogo a decorrer -> \n");
    printf("Hora de inicio  -> \n");
    printf("Tempo decorrido -> \n");
    printf("Numero de Tarefas -> \n");
    printf("Escolha o nível de dificuldade (1 = Fácil, 2 = Médio, 3 = Difícil): ");
    scanf("%d", &dificuldade);

    gerar_sudoku(tabuleiro, dificuldade);
    printf("Tabuleiro de Sudoku gerado:\n");
    jogar_sudoku(tabuleiro);

    return 0;
}
