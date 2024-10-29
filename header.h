#ifndef HEADER_H
#define HEADER_H

#include <stdbool.h>  // Inclui a definição de bool
#include <stdio.h>    // Para printf, fprintf, etc.
#include <stdlib.h>   // Para malloc, free, etc.
#include <string.h>   // Para manipulação de strings (strcat, strcmp, etc.)
#include <unistd.h>   // Para close (encerrar sockets)
#include <arpa/inet.h> // Para definições de sockets (struct sockaddr_in, htons, etc.)
#include <sys/types.h>  // Para tipos de dados de sockets
#include <sys/socket.h> // Para funções de sockets
#include <errno.h>      // Para tratamento de erros
#include <time.h>       // Para manipulação de tempo

#define TAMANHO 9

// Funções relacionadas ao jogo de Sudoku
void jogar_sudoku(int tabuleiro[TAMANHO][TAMANHO], int sock);
void escrever_log_cliente(const char *mensagem);
bool verificar_vitoria(int tabuleiro[TAMANHO][TAMANHO]);
bool isValid(int tabuleiro[TAMANHO][TAMANHO], int linha, int col, int num);
bool findEmptyCell(int tabuleiro[TAMANHO][TAMANHO], int *linha, int *col);
bool pode_colocar(int tabuleiro[TAMANHO][TAMANHO], int linha, int col, int num);
char* formatar_tabuleiro(int tabuleiro[TAMANHO][TAMANHO]);
void imprimir_tabuleiro_cliente(int tabuleiro[TAMANHO][TAMANHO]);

#endif // HEADER_H
