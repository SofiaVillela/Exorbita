/* constante.h: guarda as constantes do jogo (números que o grupo vai querer
   ajustar). Ficam todas aqui para não ficarem espalhadas pelo código. */

#ifndef CONSTANTES_H    
#define CONSTANTES_H

/* Tamanho da janela, em pixels. Desenha nesse tamanho pequeno e
   depois estica (ver main.c). */
#define LARGURA_VIRTUAL 384
#define ALTURA_VIRTUAL  224
#define ESCALA_INICIAL  3
#define FPS            60

#define MARGEM_ESQUERDA     16

#define TAMANHO_TITULO      16
#define ESPACAMENTO_TITULO  1
#define Y_TITULO            56

#define TAMANHO_SUBTITULO      8
#define ESPACAMENTO_SUBTITULO  0
#define Y_SUBTITULO            (Y_TITULO + TAMANHO_TITULO + 8)

#define COR_FUNDO      GetColor(0x25222BFF)
#define COR_TITULO     GetColor(0xEDE6F2FF)
#define COR_SUBTITULO  GetColor(0x9B8CE0FF)

#endif
