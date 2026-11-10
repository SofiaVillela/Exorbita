/* animacao.h: animação a partir de um sprite sheet (todos os quadros
   numa imagem só, organizados em grade, da esquerda para a direita e
   de cima para baixo). */

#ifndef ANIMACAO_H
#define ANIMACAO_H

#include "raylib.h"

typedef struct {
    Texture2D folha;     /* a imagem com todos os quadros */
    int larguraQuadro;
    int alturaQuadro;
    int colunas;         /* quadros por linha da grade */
    int totalQuadros;
    int quadroAtual;
    float acumulado;     /* tempo desde a última troca de quadro */
} Animacao;

/* Carrega o sprite sheet. Chamar depois do InitWindow. */
void carregarAnimacao(Animacao *a, const char *caminho,
                      int larguraQuadro, int alturaQuadro, int totalQuadros);

/* Avança o quadro conforme o tempo. Chamar uma vez por quadro do jogo. */
void atualizarAnimacao(Animacao *a);

/* Desenha o quadro atual na posição (x, y), ampliado pela escala. */
void desenharAnimacao(const Animacao *a, int x, int y, int escala);

/* Libera a textura. Chamar antes do CloseWindow. */
void descarregarAnimacao(Animacao *a);

#endif
