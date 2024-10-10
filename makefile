# Definir os executáveis
all: projeto servidor cliente

# Regras para compilar o projeto principal
projeto: projeto.o
	gcc -o projeto projeto.o -lpthread

projeto.o: projeto.c
	gcc -c projeto.c -o projeto.o

# Regras para compilar o servidor
servidor: Servidor.o
	gcc -o servidor Servidor.o -lpthread

Servidor.o: Servidor.c
	gcc -c Servidor.c -o Servidor.o

# Regras para compilar o cliente
cliente: Cliente.o
	gcc -o cliente Cliente.o -lpthread

Cliente.o: Cliente.c
	gcc -c Cliente.c -o Cliente.o

# Limpar ficheiros objeto e executáveis
clean:
	rm -f *.o *~ projeto servidor cliente
