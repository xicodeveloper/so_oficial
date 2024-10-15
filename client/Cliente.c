#include "header.h"

//O Cliente deverá ser lançado tendo como parâmetro o ficheiro de configuração. Os dados
//no ficheiro de configuração que deverão estar presentes, no mínimo, para o Cliente são os
//seguintes: IP do Servidor, ID do Cliente

struct jogador{ //array clientes o que cada um vai ter
	int id;
    int qntd_vencedor; // cada vez que ganha é incrementado um!
};
struct jogador array_jogadores[500];
//• pedeJogo(IdCliente)
//• enviaSolucao(idCliente, idJogo, Solucao)
//• recebeResposta(idCliente, idJogo, "Certo/Errado")

