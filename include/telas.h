/* telas.h: lista das telas do jogo. Fica num arquivo próprio porque
   o main.c e os arquivos de cada tela precisam conhecer esses nomes. */

#ifndef TELAS_H
#define TELAS_H

typedef enum {
    TELA_INICIO,
    TELA_INTRO,
    TELA_NAVE,
    TELA_EXTRATERRESTRE,
    TELA_GIGANTE,
    TELA_ROBO,
    TELA_SER,
    TELA_FINAL,
    TELA_SEM_AR,
    TELA_REGRAS,
    TELA_CREDITOS,
    TELA_SAIR
} Tela;

#endif
