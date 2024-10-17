# Compilador e flags
CC = gcc
CFLAGS = -Wall

# Definir o executável principal (servidor)
TARGET = servidor

# Regras para compilar o projeto principal
all: $(TARGET)

$(TARGET): Servidor.o Cliente.o
	$(CC) -o $(TARGET) Servidor.o Cliente.o $(CFLAGS) -lpthread

# Compilar Servidor.o
Servidor.o: Servidor.c header.h
	$(CC) -c Servidor.c -o Servidor.o $(CFLAGS)

# Compilar Cliente.o
Cliente.o: Cliente.c header.h
	$(CC) -c Cliente.c -o Cliente.o $(CFLAGS)

# Limpar ficheiros objeto e executáveis
clean:
	rm -f *.o *~ $(TARGET)
