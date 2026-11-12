#ifndef MAPA_ESTELAR_H
#define MAPA_ESTELAR_H

#include <stdbool.h>
#include "telas.h"

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

#endif
