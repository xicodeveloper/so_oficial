// header.h
#ifndef HEADER_H
#define HEADER_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define TAMANHO 9
// Sockets --------------------------------------------------------------------------------
#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128
//---------------------------------------------------------------------

// Funções relacionadas ao jogo de Sudoku
void jogar_sudoku(int tabuleiro[TAMANHO][TAMANHO]);
void gerar_sudoku(int tabuleiro[TAMANHO][TAMANHO], int dificuldade);
int resolver_sudoku(int tabuleiro[TAMANHO][TAMANHO], int linha, int col);
void imprimir_tabuleiro(int tabuleiro[TAMANHO][TAMANHO]);
int verificar_vitoria(int tabuleiro[TAMANHO][TAMANHO]);
int pode_colocar(int tabuleiro[TAMANHO][TAMANHO], int linha, int col, int num);

#endif
