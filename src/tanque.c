/* tanque.c: tanque de combustível vertical, em blocos empilhados. */

#include "raylib.h"
#include "constantes.h"
#include "tanque.h"

#define COR_CONTORNO   GetColor(0x9B8CE0FF)
#define COR_SOMBRA     GetColor(0x514464FF)
#define COR_DENTRO     GetColor(0x16121FFF)
#define COR_VAZIO      GetColor(0x2E2640FF)
#define COR_CHEIO      GetColor(0x68D8CDFF)
#define COR_BRILHO     GetColor(0xB8F2ECFF)
#define COR_TAMPA      GetColor(0xE8C86AFF)

static int menor(int a, int b) { return a < b ? a : b; }
static int maior(int a, int b) { return a > b ? a : b; }

/* Pinta, dentro de um bloco, a faixa de combustível [de, ate).
   O bloco vai de "base" (valor do fundo) até base + COMBUSTIVEL_POR_BLOCO
   e ocupa os pixels de yFundo para cima, com altura h. */
static void faixa(int x, int largura, int yFundo, int h, int base, int de, int ate, Color cor)
{
    de = maior(de, base);
    ate = menor(ate, base + COMBUSTIVEL_POR_BLOCO);
    if (ate <= de) {
        return;
    }
    int y0 = yFundo - (de - base) * h / COMBUSTIVEL_POR_BLOCO;     /* baixo da faixa */
    int y1 = yFundo - (ate - base) * h / COMBUSTIVEL_POR_BLOCO;    /* topo da faixa */
    if (y0 - y1 < 1) {
        y1 = y0 - 1;
    }
    DrawRectangle(x, y1, largura, y0 - y1, cor);
    DrawRectangle(x, y1, 1, y0 - y1, Fade(WHITE, 0.25f));   /* reflexo na borda */
}

void desenharTanque(int x, int y, int largura, int altura, int combustivel, int gasto)
{
    int blocos = COMBUSTIVEL_MAXIMO / COMBUSTIVEL_POR_BLOCO;
    int restante = maior(combustivel - gasto, 0);

    /* Tampa e gargalo, centralizados em cima. */
    int alturaTampa = maior(4, altura / 14);
    int larguraTampa = maior(6, largura / 3);
    int xTampa = x + (largura - larguraTampa) / 2;
    DrawRectangle(xTampa - 1, y, larguraTampa + 2, alturaTampa, COR_SOMBRA);
    DrawRectangle(xTampa, y + 1, larguraTampa, alturaTampa - 2, COR_TAMPA);
    int yCorpo = y + alturaTampa;
    int alturaCorpo = altura - alturaTampa;

    /* Corpo: sombra, contorno de 2 px e fundo escuro. */
    DrawRectangle(x + 2, yCorpo + 2, largura, alturaCorpo, Fade(BLACK, 0.5f));
    DrawRectangle(x, yCorpo, largura, alturaCorpo, COR_CONTORNO);
    DrawRectangle(x + 2, yCorpo + 2, largura - 4, alturaCorpo - 4, COR_DENTRO);

    /* Blocos, de baixo para cima, com 1 px de folga entre eles. */
    int xDentro = x + 3;
    int larguraDentro = largura - 6;
    int alturaDentro = alturaCorpo - 6;
    int alturaBloco = (alturaDentro - (blocos - 1)) / blocos;
    if (alturaBloco < 1) {
        alturaBloco = 1;
    }
    int yFundo = yCorpo + alturaCorpo - 3;

    /* Pouco combustível (depois da viagem) deixa a pilha em pêssego. */
    Color corCheio = restante <= 2 * COMBUSTIVEL_POR_BLOCO ? COR_ALERTA : COR_CHEIO;
    bool piscaAceso = ((int)(GetTime() * 3)) % 2 == 0;
    Color corGasto = piscaAceso ? COR_ALERTA : Fade(COR_ALERTA, 0.35f);

    for (int i = 0; i < blocos; i++) {
        int base = i * COMBUSTIVEL_POR_BLOCO;
        int fundoBloco = yFundo - i * (alturaBloco + 1);
        DrawRectangle(xDentro, fundoBloco - alturaBloco, larguraDentro, alturaBloco, COR_VAZIO);
        faixa(xDentro, larguraDentro, fundoBloco, alturaBloco, base, 0, restante, corCheio);
        faixa(xDentro, larguraDentro, fundoBloco, alturaBloco, base, restante, combustivel, corGasto);
    }

    /* Brilho do vidro: uma linha clara no alto do lado esquerdo. */
    DrawRectangle(x + 2, yCorpo + 2, largura - 4, 1, Fade(COR_BRILHO, 0.4f));
}
