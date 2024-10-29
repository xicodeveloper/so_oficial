# Nome dos executáveis
SERVER = server
CLIENT = client

# Compilador e flags de compilação
CC = gcc
CFLAGS = -Wall -pthread

# Alvo para compilar todos os executáveis
all: $(SERVER) $(CLIENT)

# Compilação do servidor
$(SERVER): Servidor.c
	$(CC) $(CFLAGS) -o $(SERVER) Servidor.c

# Compilação do cliente
$(CLIENT): Cliente.c
	$(CC) $(CFLAGS) -o $(CLIENT) Cliente.c

# Alvo para limpar os arquivos de compilação
clean:
	rm -f $(SERVER) $(CLIENT)

clean:
	rm -f *.o *~ $(TARGET)

# Adicionar a regra clear
clear: clean
	@echo "Ficheiros limpos!"
