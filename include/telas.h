/* telas.h: lista das telas do jogo. Fica num arquivo próprio porque
   o main.c e os arquivos de cada tela precisam conhecer esses nomes. */

#ifndef TELAS_H
#define TELAS_H

typedef enum {
    TELA_INICIO,
    TELA_NAVE,
    TELA_EXTRATERRESTRE,
    TELA_GIGANTE,
    TELA_ROBO,
    TELA_SER,
    TELA_REGRAS,
    TELA_CREDITOS,
    TELA_SAIR
} Tela;

#endif
