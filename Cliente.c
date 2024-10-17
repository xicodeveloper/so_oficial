#include "header.h"
#define TAMANHO 9


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