Instruções para correr a aplicação pela primeira vez:
1: Abrir terminal
2: make clear
3: make
4: Executar o servidor
5: Executar os clientes
_________________________________________________________________

Para voltar a executar é recomendavel limpar os ficheiros de Log
Para limpar os ficherios de log:
1: make text
2: Executar o servidor
3: Executar os clientes
_________________________________________________________________

"make clear" é para limpar os executaveis e os ficheiros de log
"make text" é para limpar os ficheiros de log
_________________________________________________________________

EXECUTAR O SERVIDOR:
./server ./configuracoes/config_server.txt


EXECUTAR UM CLIENTE:
./client ./configuracoes/clientes/client_config_1.txt
ou
./client ./configuracoes/clientes/client_config_2.txt
ou
./client ./configuracoes/clientes/client_config_3.txt
_________________________________________________________________

Cliente_config_1 contém a config com: Prioridade = 1 e será Resolvedor no modo de jogo 2
Cliente_config_2 contém a config com: Prioridade = 2 e será Apagador no modo de jogo 2
Cliente_config_3 contém a config com: Prioridade = 3 e será Resolvedor no modo de jogo 2