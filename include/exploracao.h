/* Movimento e colisão em coordenadas do cenário, sem depender da raylib. */
#ifndef EXPLORACAO_H
#define EXPLORACAO_H

#include <stdbool.h>

#define EXPLORACAO_MEIA_LARGURA 8
#define EXPLORACAO_ALTURA 8

typedef struct {
    float x, y;
} PontoExploracao;

typedef struct {
    unsigned char r, g, b, a;
} PixelExploracao;

typedef struct {
    const PixelExploracao *pixels;
    int largura, altura;
} MapaExploracao;

typedef struct {
    float x, y, largura, altura;
} ObstaculoExploracao;

/* O retângulo dos pés mede 16x8; pes é o centro da sua borda inferior.
   Branco e as três cores primárias da máscara são piso; preto e cores
   desconhecidas bloqueiam. As cores primárias podem marcar interações.
   Obstáculos adicionais usam o canto superior esquerdo e dimensões. */
bool posicaoLivreExploracao(const MapaExploracao *mapa, PontoExploracao pes,
                           const ObstaculoExploracao *obstaculos, int quantidade);

/* Eixos entre -1 e 1, velocidade em pixels/s e dt em segundos.
   Normaliza diagonais e divide o deslocamento em passos de até 1 pixel,
   resolvendo os eixos separadamente para deslizar pelas paredes.
   Dados inválidos ou tempo/velocidade não positivos mantêm a posição. */
PontoExploracao moverExploracao(const MapaExploracao *mapa, PontoExploracao pes,
                               float eixoX, float eixoY, float velocidade, float dt,
                               const ObstaculoExploracao *obstaculos, int quantidade);

#endif
