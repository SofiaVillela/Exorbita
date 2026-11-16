/* fliperama.h: o fliperama do bar do planeta extraterrestre.

   Mostra o gabinete na tela inteira e, dentro dele, o jogo de naves
   (as regras ficam em fliperama_logica.c). WASD move a nave, ESPAÇO
   atira e ESC volta para o bar. A arte é gerada por
   ferramentas/gerar_extraterrestre.py. */

#ifndef FLIPERAMA_H
#define FLIPERAMA_H

#include <stdbool.h>

/* Carrega as imagens. Chamar depois do InitWindow. */
void carregarFliperama(void);

/* Liga a máquina: mostra a tela de título. */
void abrirFliperama(void);

/* Um quadro do fliperama. Devolve false quando o jogador sai (ESC). */
bool atualizarTelaFliperama(void);

void desenharTelaFliperama(void);

void descarregarFliperama(void);

#endif
