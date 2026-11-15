/* dialogo.c: caixa de diálogo padrão do jogo (ver dialogo.h). */

#include <math.h>
#include <stddef.h>
#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "dialogo.h"

#define DIALOGO_X          16
#define DIALOGO_LARGURA    608
#define DIALOGO_BASE       342    /* linha de baixo da caixa (acima da barra de teclas) */
#define DIALOGO_ALTURA_MIN 96
#define DIALOGO_LINHA      12     /* altura de cada linha de texto (fonte 8 + espaço) */
#define RETRATO_TAMANHO    72
#define TEXTO_X            (DIALOGO_X + 16 + RETRATO_TAMANHO + 12)

#define COR_DIALOGO_FUNDO  GetColor(0x211C2EFF)
#define COR_DIALOGO_BORDA  GetColor(0x8C77ADFF)
#define COR_DIALOGO_NOME   GetColor(0x68D8CDFF)   /* verde-água dos terminais */
#define COR_RETRATO_FUNDO  GetColor(0x30283FFF)

static void escrever(const char *s, int x, int y, Color cor)
{
    DrawTextEx(fontePadrao(), s, (Vector2){ (float)x, (float)y }, 8, 0, cor);
}

static int contarLinhas(const char *texto)
{
    int linhas = 1;
    for (const char *c = texto; *c != '\0'; c++) {
        if (*c == '\n') {
            linhas++;
        }
    }
    return linhas;
}

/* Desenha a caixa vazia com a altura pedida (cresce para cima) e o
   retrato. Devolve o y do topo da caixa. */
static int caixaComRetrato(const Falante *falante, int altura)
{
    if (altura < DIALOGO_ALTURA_MIN) {
        altura = DIALOGO_ALTURA_MIN;
    }
    int y = DIALOGO_BASE - altura;

    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.3f));
    DrawRectangle(DIALOGO_X + 3, y + 3, DIALOGO_LARGURA, altura, Fade(BLACK, 0.5f));
    DrawRectangle(DIALOGO_X, y, DIALOGO_LARGURA, altura, COR_DIALOGO_FUNDO);
    DrawRectangleLines(DIALOGO_X, y, DIALOGO_LARGURA, altura, COR_DIALOGO_BORDA);
    DrawRectangle(DIALOGO_X + 2, y + 2, DIALOGO_LARGURA - 4, 1, GetColor(0x514464FF));

    /* Retrato: o rosto ampliado até caber no quadro. */
    int rx = DIALOGO_X + 12;
    int ry = y + 12;
    DrawRectangle(rx, ry, RETRATO_TAMANHO, RETRATO_TAMANHO, COR_RETRATO_FUNDO);
    if (falante->folha.id != 0) {
        Rectangle destino = { (float)rx + 1, (float)ry + 1,
                              RETRATO_TAMANHO - 2, RETRATO_TAMANHO - 2 };
        DrawTexturePro(falante->folha, falante->rosto, destino, (Vector2){ 0, 0 }, 0, WHITE);
    }
    DrawRectangleLines(rx, ry, RETRATO_TAMANHO, RETRATO_TAMANHO, COR_DIALOGO_BORDA);

    escrever(falante->nome, TEXTO_X, y + 12, COR_DIALOGO_NOME);
    return y;
}

static void desenharRodape(int y, int altura, const char *rodape, bool seta)
{
    int base = y + altura - 18;
    if (rodape != NULL) {
        escrever(rodape, TEXTO_X, base, COR_MENU);
    }
    /* A seta pisca: aparece na metade de cada segundo. */
    if (seta && fmod(GetTime(), 1.0) < 0.6) {
        int sx = DIALOGO_X + DIALOGO_LARGURA - 22;
        DrawTriangle((Vector2){ (float)sx, (float)base },
                     (Vector2){ (float)sx + 4, (float)base + 5 },
                     (Vector2){ (float)sx + 8, (float)base }, COR_DIALOGO_NOME);
    }
}

void desenharFala(const Falante *falante, const char *texto,
                  const char *rodape, bool seta)
{
    /* nome + espaço + linhas do texto + espaço + rodapé */
    int altura = 12 + 18 + contarLinhas(texto) * DIALOGO_LINHA + 12 + 18;
    int y = caixaComRetrato(falante, altura);
    if (altura < DIALOGO_ALTURA_MIN) {
        altura = DIALOGO_ALTURA_MIN;
    }
    DrawTextEx(fontePadrao(), texto, (Vector2){ (float)TEXTO_X, (float)y + 30 }, 8, 0, COR_TITULO);
    desenharRodape(y, altura, rodape, seta);
}

void desenharEscolhaDialogo(const Falante *falante, const char *pergunta,
                            const char *opcoes[], int total, int escolhida,
                            const char *detalhe, const char *rodape)
{
    /* nome + pergunta + opções + detalhe + rodapé */
    int altura = 12 + 18 + 16 + total * DIALOGO_LINHA + 8 + 14 + 18 + 4;
    int y = caixaComRetrato(falante, altura);
    if (altura < DIALOGO_ALTURA_MIN) {
        altura = DIALOGO_ALTURA_MIN;
    }
    escrever(pergunta, TEXTO_X, y + 30, COR_TITULO);

    int yLista = y + 46;
    for (int i = 0; i < total; i++) {
        int linha = yLista + i * DIALOGO_LINHA;
        Color cor = COR_MENU;
        if (i == escolhida) {
            cor = COR_SELECIONADO;
            DrawRectangle(TEXTO_X, linha + 1, 6, 6, COR_DIALOGO_NOME);
        }
        escrever(opcoes[i], TEXTO_X + 14, linha, cor);
    }
    if (detalhe != NULL) {
        escrever(detalhe, TEXTO_X + 14, yLista + total * DIALOGO_LINHA + 6, COR_SUBTITULO);
    }
    desenharRodape(y, altura, rodape, false);
}
