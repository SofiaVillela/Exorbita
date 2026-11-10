/* estrelas.c: fundo de estrelas. Algumas ficam fixas e outras piscam. */

#include <math.h>
#include "raylib.h"
#include "constantes.h"
#include "estrelas.h"

typedef struct {
    int x;
    int y;
    int brilho;        /* 0 = fraca, 1 = média, 2 = forte */
    bool pisca;        /* se esta estrela pisca ou fica fixa */
    float velocidade;  /* rapidez da piscada */
    float fase;        /* ponto de partida da piscada */
} Estrela;

static Estrela estrelas[NUM_ESTRELAS];

void iniciarEstrelas(void)
{
    for (int i = 0; i < NUM_ESTRELAS; i++) {
        estrelas[i].x = GetRandomValue(0, LARGURA_VIRTUAL - 1);
        estrelas[i].y = GetRandomValue(0, ALTURA_VIRTUAL - 1);

        /* Sorteia de 0 a 9: 60% fracas, 30% médias, 10% fortes. */
        int sorteio = GetRandomValue(0, 9);
        if (sorteio < 6) {
            estrelas[i].brilho = 0;
        } else if (sorteio < 9) {
            estrelas[i].brilho = 1;
        } else {
            estrelas[i].brilho = 2;
        }

        /* 1 em cada 4 estrelas pisca. */
        estrelas[i].pisca = (GetRandomValue(0, 3) == 0);

        /* Velocidade entre 1.0 e 3.0 (GetRandomValue só sorteia inteiros). */
        estrelas[i].velocidade = GetRandomValue(10, 30) / 10.0f;

        /* Fase entre 0 e 6.28 (uma volta completa do seno), para não
           piscarem todas ao mesmo tempo. */
        estrelas[i].fase = GetRandomValue(0, 628) / 100.0f;
    }
}

void desenharEstrelas(void)
{
    for (int i = 0; i < NUM_ESTRELAS; i++) {
        int nivel = estrelas[i].brilho;

        if (estrelas[i].pisca) {
            /* sinf vai e volta entre -1 e 1 conforme o tempo passa. */
            float onda = sinf((float)GetTime() * estrelas[i].velocidade + estrelas[i].fase);
            if (onda > 0.5f) {
                nivel++;        /* parte alta da onda: fica mais forte */
            } else if (onda < -0.5f) {
                nivel--;        /* parte baixa da onda: fica mais fraca */
            }
        }

        if (nivel > 2) {
            nivel = 2;
        }
        if (nivel < 0) {
            continue;           /* abaixo da fraca: some por um instante */
        }

        Color cor = COR_ESTRELA_FRACA;
        if (nivel == 1) {
            cor = COR_ESTRELA_MEDIA;
        } else if (nivel == 2) {
            cor = COR_ESTRELA_FORTE;
        }
        DrawPixel(estrelas[i].x, estrelas[i].y, cor);
    }
}
