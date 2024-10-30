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
//-----------------------------------------------------------------------------------------
// Estrutura para armazenar informações do utilizador
typedef struct {
    int id;
    // Adicione outros campos conforme necessário
    char nome[50]; // Exemplo de campo adicional
} cliente_inf;
// Funções relacionadas ao jogo de Sudoku


bool resolver(int tabuleiro[TAMANHO][TAMANHO]);
void imprimir_tabuleiro_cliente(int tabuleiro[TAMANHO][TAMANHO]);
void escrever_log_cliente(const char *mensagem);
int get_new_user_id(); // Declaração da função para obter um novo ID de utilizador

#endif // HEADER_H