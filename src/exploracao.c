#include "exploracao.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>

static bool piso(PixelExploracao pixel)
{
    if (pixel.a != 255) return false;
    return (pixel.r == 255 && pixel.g == 255 && pixel.b == 255) ||
           (pixel.r == 255 && pixel.g == 0 && pixel.b == 0) ||
           (pixel.r == 0 && pixel.g == 255 && pixel.b == 0) ||
           (pixel.r == 0 && pixel.g == 0 && pixel.b == 255);
}

bool posicaoLivreExploracao(const MapaExploracao *mapa, PontoExploracao pes,
                           const ObstaculoExploracao *obstaculos, int quantidade)
{
    if (!mapa || !mapa->pixels || mapa->largura <= 0 || mapa->altura <= 0 ||
        !isfinite(pes.x) || !isfinite(pes.y) || quantidade < 0 ||
        (quantidade > 0 && !obstaculos)) return false;

    float esquerda = pes.x - EXPLORACAO_MEIA_LARGURA;
    float direita = pes.x + EXPLORACAO_MEIA_LARGURA;
    float cima = pes.y - EXPLORACAO_ALTURA;
    float baixo = pes.y;

    if (esquerda < 0 || cima < 0 || direita > mapa->largura || baixo > mapa->altura)
        return false;

    for (int i = 0; i < quantidade; i++) {
        ObstaculoExploracao obstaculo = obstaculos[i];
        if (!isfinite(obstaculo.x) || !isfinite(obstaculo.y) ||
            !isfinite(obstaculo.largura) || !isfinite(obstaculo.altura)) return false;
        if (obstaculo.largura <= 0 || obstaculo.altura <= 0) continue;
        if (esquerda < obstaculo.x + obstaculo.largura && direita > obstaculo.x &&
            cima < obstaculo.y + obstaculo.altura && baixo > obstaculo.y)
            return false;
    }

    int primeiroX = (int)floorf(esquerda);
    int ultimoX = (int)ceilf(direita) - 1;
    int primeiroY = (int)floorf(cima);
    int ultimoY = (int)ceilf(baixo) - 1;
    for (int y = primeiroY; y <= ultimoY; y++) {
        for (int x = primeiroX; x <= ultimoX; x++) {
            size_t indice = (size_t)y * (size_t)mapa->largura + (size_t)x;
            if (!piso(mapa->pixels[indice])) return false;
        }
    }
    return true;
}

PontoExploracao moverExploracao(const MapaExploracao *mapa, PontoExploracao pes,
                               float eixoX, float eixoY, float velocidade, float dt,
                               const ObstaculoExploracao *obstaculos, int quantidade)
{
    if (!isfinite(eixoX) || !isfinite(eixoY) || !isfinite(velocidade) ||
        !isfinite(dt) || velocidade <= 0 || dt <= 0 ||
        !posicaoLivreExploracao(mapa, pes, obstaculos, quantidade)) return pes;

    float comprimento = hypotf(eixoX, eixoY);
    if (!isfinite(comprimento) || comprimento == 0) return pes;
    if (comprimento > 1) {
        eixoX /= comprimento;
        eixoY /= comprimento;
    }

    float deltaX = eixoX * velocidade * dt;
    float deltaY = eixoY * velocidade * dt;
    float maior = fmaxf(fabsf(deltaX), fabsf(deltaY));
    /* Evita converter um deslocamento não representável para int. */
    if (!isfinite(maior) || maior >= (float)INT_MAX || maior == 0) return pes;
    int passos = (int)ceilf(maior);
    float passoX = deltaX / passos;
    float passoY = deltaY / passos;

    for (int i = 0; i < passos; i++) {
        PontoExploracao anterior = pes;
        PontoExploracao tentativa = { pes.x + passoX, pes.y };
        if (posicaoLivreExploracao(mapa, tentativa, obstaculos, quantidade))
            pes.x = tentativa.x;
        tentativa = (PontoExploracao){ pes.x, pes.y + passoY };
        if (posicaoLivreExploracao(mapa, tentativa, obstaculos, quantidade))
            pes.y = tentativa.y;
        /* Com direção constante, não há saída se ambos os eixos bloquearam. */
        if (pes.x == anterior.x && pes.y == anterior.y) break;
    }
    return pes;
}
