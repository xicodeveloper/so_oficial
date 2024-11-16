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

int zero_1=32;
int zero_2=41;
int zero_3=32;
int zero_4=41;



#endif // HEADER_H