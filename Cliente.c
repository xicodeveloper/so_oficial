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