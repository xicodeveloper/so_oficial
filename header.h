// header.h
#ifndef HEADER_H
#define HEADER_H
#include <string.h>
#include <stdio.h>
#include <stdbool.h>  
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


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
void gravar_solucao(int tabuleiro[TAMANHO][TAMANHO], const char *nome_ficheiro_solucoes);
void escrever_log(const char *mensagem);

bool resolver(int tabuleiro[TAMANHO][TAMANHO]);
void imprimir_tabuleiro_cliente(int tabuleiro[TAMANHO][TAMANHO]);
void escrever_log_cliente(const char *mensagem);
//void connect_server();
#endif
