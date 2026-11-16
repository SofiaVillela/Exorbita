/* tela_final.h: finais do jogo.

   Bom: a nave chega à Via Láctea.
   Ruim: todos morreram e a nave fica à deriva, sem ninguém para voltar.
   O final é escolhido sozinho ao entrar na tela, olhando quem está vivo. */

#ifndef TELA_FINAL_H
#define TELA_FINAL_H

#include "telas.h"

/* Carrega a nave e a Via Láctea. Chamar uma vez, depois do InitWindow. */
void iniciarTelaFinal(void);

/* Chamar ao chegar no final: escolhe o final e começa a animação do zero. */
void entrarTelaFinal(void);

Tela atualizarTelaFinal(void);
void desenharTelaFinal(void);

/* Libera o que foi carregado. Chamar antes do CloseWindow. */
void encerrarTelaFinal(void);

#endif
