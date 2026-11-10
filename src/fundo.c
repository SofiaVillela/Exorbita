/* fundo.c: fundo do espaço em três camadas (nebulosa, estrelas longe e
   estrelas perto). Cada camada anda numa velocidade: as de longe andam
   mais devagar, e isso dá sensação de profundidade (paralaxe). */

#include "raylib.h"
#include "constantes.h"
#include "fundo.h"

#define TOTAL_CAMADAS 3

static Texture2D camadas[TOTAL_CAMADAS];
static float deslocamento[TOTAL_CAMADAS];   /* quanto cada camada já andou */
static const float velocidade[TOTAL_CAMADAS] = {
    VELOCIDADE_NEBULOSA,
    VELOCIDADE_ESTRELAS_LONGE,
    VELOCIDADE_ESTRELAS_PERTO
};

void iniciarFundo(void)
{
    camadas[0] = LoadTexture("assets/imagens/fundo_nebulosa.png");
    camadas[1] = LoadTexture("assets/imagens/estrelas_longe.png");
    camadas[2] = LoadTexture("assets/imagens/estrelas_perto.png");

    for (int i = 0; i < TOTAL_CAMADAS; i++) {
        deslocamento[i] = 0;
    }
}

void atualizarFundo(void)
{
    float tempo = GetFrameTime();

    for (int i = 0; i < TOTAL_CAMADAS; i++) {
        deslocamento[i] += velocidade[i] * tempo;
        if (deslocamento[i] >= LARGURA_VIRTUAL) {
            deslocamento[i] -= LARGURA_VIRTUAL;   /* volta ao começo sem emenda */
        }
    }
}

void desenharFundo(void)
{
    for (int i = 0; i < TOTAL_CAMADAS; i++) {
        int x = (int)deslocamento[i];   /* posição inteira mantém os pixels nítidos */

        /* Duas cópias lado a lado: quando a primeira sai pela esquerda,
           a segunda já está cobrindo a tela. */
        DrawTexture(camadas[i], -x, 0, WHITE);
        DrawTexture(camadas[i], LARGURA_VIRTUAL - x, 0, WHITE);
    }
}

void encerrarFundo(void)
{
    for (int i = 0; i < TOTAL_CAMADAS; i++) {
        UnloadTexture(camadas[i]);
    }
}
