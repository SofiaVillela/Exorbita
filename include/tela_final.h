/* tela_final.h: finais do jogo.

   Bom: a nave chega à Via Láctea.
   Ruim: todos morreram, ou o combustível não dá para ir a lugar
   nenhum, e a nave fica à deriva. O final é escolhido ao entrar na
   tela, olhando quem está vivo e o pedido de marcarFinalSemCombustivel. */

#ifndef TELA_FINAL_H
#define TELA_FINAL_H

#include "telas.h"

/* Carrega a nave e a Via Láctea. Chamar uma vez, depois do InitWindow. */
void iniciarTelaFinal(void);

/* Chamar ao chegar no final: escolhe o final e começa a animação do zero. */
void entrarTelaFinal(void);

/* Pede o final ruim por falta de combustível (a nave presa no espaço).
   Chamar antes de ir para TELA_FINAL; vale só para a próxima entrada. */
void marcarFinalSemCombustivel(void);

Tela atualizarTelaFinal(void);
void desenharTelaFinal(void);

/* Libera o que foi carregado. Chamar antes do CloseWindow. */
void encerrarTelaFinal(void);

#endif
