#include "cartas.h"

#include <string.h>

static uint32_t sortear(PartidaCartas *p)
{
    p->aleatorio = p->aleatorio * UINT32_C(1664525) + UINT32_C(1013904223);
    return p->aleatorio;
}

static void prepararRodada(PartidaCartas *p)
{
    int disponiveis[CARTAS_NA_MAO], quantidade = 0;
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        if (!p->usadasAlien[i]) disponiveis[quantidade++] = i;
    }
    p->indiceAlien = disponiveis[sortear(p) % (uint32_t)quantidade];
    int real = p->maoAlien[p->indiceAlien];
    p->anuncioAlien = real;
    /* O anúncio falso pode ser maior ou menor; nunca coincide com a carta. */
    if ((sortear(p) >> 8) & 1u) {
        int outro = (int)(sortear(p) % (CARTAS_NO_BARALHO - 1)) + 1;
        p->anuncioAlien = outro >= real ? outro + 1 : outro;
    }
    p->cartaJogada = 0;
    p->desconfiou = false;
    p->venceuRodada = false;
    p->fase = CARTAS_ESCOLHENDO;
}

bool iniciarCartas(PartidaCartas *p, uint32_t semente)
{
    if (!p) return false;
    memset(p, 0, sizeof(*p));
    p->aleatorio = semente;
    int baralho[CARTAS_NO_BARALHO];
    for (int i = 0; i < CARTAS_NO_BARALHO; i++) baralho[i] = i + 1;
    for (int i = CARTAS_NO_BARALHO - 1; i > 0; i--) {
        int j = (int)(sortear(p) % (uint32_t)(i + 1));
        int troca = baralho[i]; baralho[i] = baralho[j]; baralho[j] = troca;
    }
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        p->maoJogador[i] = baralho[i];
        p->maoAlien[i] = baralho[CARTAS_NA_MAO + i];
    }
    p->rodada = 1;
    prepararRodada(p);
    return true;
}

bool jogarCartas(PartidaCartas *p, int indiceCarta, bool desconfiar)
{
    if (!p || p->fase != CARTAS_ESCOLHENDO || p->rodada < 1 || p->rodada > 3 ||
        indiceCarta < 0 || indiceCarta >= CARTAS_NA_MAO || p->usadasJogador[indiceCarta] ||
        p->indiceAlien < 0 || p->indiceAlien >= CARTAS_NA_MAO || p->usadasAlien[p->indiceAlien])
        return false;

    int alien = p->maoAlien[p->indiceAlien];
    p->cartaJogada = p->maoJogador[indiceCarta];
    p->desconfiou = desconfiar;
    p->venceuRodada = desconfiar ? p->anuncioAlien != alien : p->cartaJogada > alien;
    p->usadasJogador[indiceCarta] = true;
    p->usadasAlien[p->indiceAlien] = true;
    if (p->venceuRodada) p->pontosJogador++;
    else p->pontosAlien++;
    p->fase = p->pontosJogador >= 2 || p->pontosAlien >= 2 || p->rodada == 3 ?
              CARTAS_ENCERRADAS : CARTAS_RESULTADO;
    return true;
}

bool avancarRodadaCartas(PartidaCartas *p)
{
    if (!p || p->fase != CARTAS_RESULTADO || p->rodada < 1 || p->rodada >= 3 ||
        p->pontosJogador >= 2 || p->pontosAlien >= 2) return false;
    p->rodada++;
    prepararRodada(p);
    return true;
}

bool consumirVitoriaCartas(PartidaCartas *p)
{
    if (!p || p->fase != CARTAS_ENCERRADAS || p->pontosJogador < 2 || p->vitoriaConsumida)
        return false;
    p->vitoriaConsumida = true;
    return true;
}
