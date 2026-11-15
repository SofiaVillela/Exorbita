/* tela_final.h: final do jogo, ao viajar para a Via Láctea. */

#ifndef TELA_FINAL_H
#define TELA_FINAL_H

#include "telas.h"

/* Carrega a nave e a Via Láctea. Chamar uma vez, depois do InitWindow. */
void iniciarTelaFinal(void);

/* Chamar ao chegar no final: a animação começa do zero. */
void entrarTelaFinal(void);

Tela atualizarTelaFinal(void);
void desenharTelaFinal(void);

/* Libera o que foi carregado. Chamar antes do CloseWindow. */
void encerrarTelaFinal(void);

#endif
