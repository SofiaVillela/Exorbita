#ifndef MAPA_ESTELAR_H
#define MAPA_ESTELAR_H

#include <stdbool.h>
#include "telas.h"
#include "lista_planetas.h"

/* Consulta de destinos pelo painel da nave. A seleção fica guardada ao fechar. */
void iniciarMapaEstelar(void);
void abrirMapaEstelar(void);
/* Ao fechar pelo comando de pouso, informa a tela solicitada. */
Tela destinoEscolhidoMapaEstelar(void);
/* Devolve true quando o jogador pede para voltar à nave com Esc. */
bool atualizarMapaEstelar(void);
/* Desenha na tela virtual já aberta pelo jogo, sem iniciar outro frame. */
void desenharMapaEstelar(void);
void encerrarMapaEstelar(void);

/* Nova partida: nenhum planeta visitado, Via Láctea bloqueada de novo
   e o painel volta a abrir no primeiro planeta. */
void reiniciarMapaEstelar(void);

/* Início da lista circular de planetas (NULL antes de iniciar).
   O planeta do Ser usa para o desejo "Mostre o caminho". */
Planeta *listaPlanetasMapa(void);

#endif
