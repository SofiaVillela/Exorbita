/* tela_inicio.h: o que o main.c pode chamar da tela de início. */

#ifndef TELA_INICIO_H
#define TELA_INICIO_H

#include "telas.h"

/* Lê o teclado e devolve para qual tela o jogo deve ir
   (TELA_INICIO se nada mudou). */
Tela atualizarTelaInicio(void);

/* Desenha fundo, planeta, título, subtítulo e menu. Chamar dentro do BeginTextureMode. */
void desenharTelaInicio(void);

/* Carrega o GIF do planeta. Chamar uma vez, depois do InitWindow. */
void iniciarTelaInicio(void);

/* Chamar ao voltar para o menu: refaz a lista (Continuar aparece ou some). */
void entrarTelaInicio(void);

/* Libera o GIF do planeta. Chamar antes do CloseWindow. */
void encerrarTelaInicio(void);

#endif
