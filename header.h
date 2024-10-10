#ifndef HEADER_H
#define HEADER_H

#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdarg.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/sem.h>

// Sockets --------------------------------------------------------------------------------
#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128
//---------------------------------------------------------------------
#define FILENAME_SIZE 2048
#define MAX_LINE 100
#define MAX_LENGTH 1000
#define MAX_SIZE 100
#define SHM_KEY 0x1234

// Declaração do mutex (sem inicialização)
extern pthread_mutex_t mutex;

#endif
