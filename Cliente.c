#include "header.h"
#define TAMANHO 9


// Função de jogo
void jogar_sudoku(int tabuleiro[TAMANHO][TAMANHO]) {
    int linha, col, num;
    char buffer[20];
    int erros = 0;


    while (1) {
    
        time_t mytime = time(NULL);
        char *timestamp = ctime(&mytime);
        timestamp[strlen(timestamp) - 1] = 0; // Remove a nova linha do timestamp
        imprimir_tabuleiro(tabuleiro);

        // Verificar vitória
        if (verificar_vitoria(tabuleiro)) {
            printf("Parabéns! Completou o Sudoku corretamente!\n");
            escrever_log("Jogador concluiu o jogo");
            break;
        }

        printf("Insira a linha (1-9), coluna (1-9) e o número (1-9) ou 0 0 0 para sair: ");
        

        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // tentar extrair os tres números
            int result = sscanf(buffer, "%d %d %d", &linha, &col, &num);
            // Verificar os tres números foram lidos
            if (result != 3) {
                printf("Erro se esperava 3 inteiros.\n");
                escrever_log("Cliente inseriu mal as colunas/linhas/número");
                continue; // Pedir input novamente
            }

        
        } else {
              escrever_log("Erro a ler a entrada");
            continue; // Pedir input novamente
        }
            

        if (linha == 0 && col == 0 && num == 0) {
            printf("Jogo terminado. Até à próxima!\n");
          escrever_log("O jogador terminou");
            break;
        }

        linha--; // por a 0
        col--;   // por a 0

        // Verifica se a posição está vazia e o número é válido
        if (tabuleiro[linha][col] == 0 && pode_colocar(tabuleiro, linha, col, num)) {
             escrever_log("Cliente inseriu no Soduku!");
            printf("Cliente inseriu no Soduku!\n");
            
            tabuleiro[linha][col] = num;
        } else {
           escrever_log("Movimento invalido");
            printf("Movimento inválido!\n");
            erros++;
            if (erros >= 3) {
                printf("Perdeu o jogo. Excedeu as 3 tentativas.\n");
               escrever_log("O Cliente perdeu o jogo");
                break;
            }
        }
    }
}
bool isValid(int tabuleiro[TAMANHO][TAMANHO], int row, int col, int num) {
    // Verifica se o número já está na linha ou na coluna
    for (int x = 0; x < TAMANHO; x++) {
        if (tabuleiro[row][x] == num || tabuleiro[x][col] == num)
            return false;
    }

    // Verifica se o número já está na subgrade 3x3
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (tabuleiro[i + startRow][j + startCol] == num)
                return false;
        }
    }

    return true;
}

// Função para encontrar uma célula vazia (com valor 0)
bool findEmptyCell(int tabuleiro[TAMANHO][TAMANHO], int *row, int *col) {
    for (*row = 0; *row < TAMANHO; (*row)++) {
        for (*col = 0; *col < TAMANHO; (*col)++) {
            if (tabuleiro[*row][*col] == 0) {
                return true; // Encontrei uma célula vazia
            }
        }
    }
    return false; // Não há células vazias, o tabuleiro está preenchido
}

// Função para resolver o Sudoku utilizando backtracking
bool resolver(int tabuleiro[TAMANHO][TAMANHO]) {
    int row, col;

    // Se não houver células vazias, o Sudoku está resolvido
    if (!findEmptyCell(tabuleiro, &row, &col)) {
        return true;
    }

    // Tenta números de 1 a 9
    for (int num = 1; num <= 9; num++) {
        if (isValid(tabuleiro, row, col, num)) {
            // Coloca o número na célula
            tabuleiro[row][col] = num;

            // Recursivamente tenta resolver o resto do tabuleiro
            if (resolver(tabuleiro)) {
                return true;
            }

            // Se falhar, remove o número e tenta outro
            tabuleiro[row][col] = 0;
        }
    }

    // Se nenhum número puder ser colocado, retrocede (backtrack)
    return false;
}
void imprimir_tabuleiro_cliente(int tabuleiro[TAMANHO][TAMANHO]) {
    printf("---------------\n");
    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            // Imprime o número se não for zero, caso contrário imprime um ponto
            if (tabuleiro[i][j] == 0) {
                printf("_ ");
            } else {
                printf("%d ", tabuleiro[i][j]);
            }

            // Adiciona uma linha vertical após cada 3 colunas
            if ((j + 1) % 3 == 0 && j != TAMANHO - 1) {
                printf("| ");
            }
        }
        printf("\n");

        // Adiciona uma linha horizontal após cada 3 linhas
        if ((i + 1) % 3 == 0 && i != TAMANHO - 1) {
            printf("---------------\n");
        }
    }
    printf("---------------\n");
}