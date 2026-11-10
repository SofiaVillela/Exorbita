/* animacao.c: animação a partir de um sprite sheet. */

#include "raylib.h"
#include "constantes.h"
#include "animacao.h"

void carregarAnimacao(Animacao *a, const char *caminho,
                      int larguraQuadro, int alturaQuadro, int totalQuadros)
{
    a->folha = LoadTexture(caminho);
    SetTextureFilter(a->folha, TEXTURE_FILTER_POINT);

    a->larguraQuadro = larguraQuadro;
    a->alturaQuadro = alturaQuadro;
    a->colunas = a->folha.width / larguraQuadro;   /* ex.: 1152 / 96 = 12 */
    a->totalQuadros = totalQuadros;
    a->quadroAtual = 0;
    a->acumulado = 0;
}

void atualizarAnimacao(Animacao *a)
{
    a->acumulado += GetFrameTime();
    if (a->acumulado >= TEMPO_QUADRO) {
        a->acumulado = 0;
        a->quadroAtual = (a->quadroAtual + 1) % a->totalQuadros;
    }
}

void desenharAnimacao(const Animacao *a, int x, int y, int escala)
{
    /* Índice do quadro -> posição na grade (como índice de vetor -> matriz).
       Ex.: grade de 12 colunas, quadro 30 -> coluna 6, linha 2. */
    int coluna = a->quadroAtual % a->colunas;
    int linha = a->quadroAtual / a->colunas;

    Rectangle origem = { coluna * a->larguraQuadro, linha * a->alturaQuadro,
                         a->larguraQuadro, a->alturaQuadro };
    Rectangle destino = { x, y, a->larguraQuadro * escala, a->alturaQuadro * escala };
    DrawTexturePro(a->folha, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

void descarregarAnimacao(Animacao *a)
{
    UnloadTexture(a->folha);
}
