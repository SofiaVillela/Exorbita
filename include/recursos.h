#ifndef RECURSOS_H
#define RECURSOS_H

#include "raylib.h" 


void carregarRecursos(void);
void descarregarRecursos(void);
Font fontePadrao(void);

/* O tradutor é mostrado em várias telas (laboratório, nave, cidade),
   então o desenho dele fica aqui. Desenha o aparelho (32x32 vezes a
   escala) com o canto de cima em (x, y); apagado = ainda não obtido. */
void desenharTradutor(float x, float y, float escala, bool aceso);

#endif
