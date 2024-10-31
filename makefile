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

# Alvo para limpar os arquivos de compilação e configurações
clean:
	rm -f $(SERVER) $(CLIENT)

# Alvo para limpeza com mensagem adicional
clear: clean
	@echo "Ficheiros e configurações limpos"

# Força a execução do "clean" e "clear"
.PHONY: all clean clear
