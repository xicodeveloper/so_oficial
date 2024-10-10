all: main

main: projeto.o Servidor.o Cliente.o
	gcc -o main projeto.o Servidor.o Cliente.o -lpthread

projeto.o: projeto.c
	gcc -c projeto.c -o projeto.o

Servidor.o: Servidor.c
	gcc -c Servidor.c -o Servidor.o

Cliente.o: Cliente.c
	gcc -c Cliente.c -o Cliente.o

clean:
	rm -f *.o *~ main
