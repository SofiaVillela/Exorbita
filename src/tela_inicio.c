/* tela_inicio.c: título, subtítulo e menu (Jogar, Regras, Créditos, Sair). */

#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "tela_inicio.h"
#include "estrelas.h"

/* static: só este arquivo enxerga essas variáveis.
   Elas guardam o valor entre uma chamada e outra. */
static const char *opcoes[] = { "Jogar", "Regras", "Créditos", "Sair" };
static const int totalOpcoes = 4;
static int selecionada = 0;   /* índice da opção em destaque */

Tela atualizarTelaInicio(void)
{
    if (IsKeyPressed(KEY_DOWN)) {
        selecionada++;
        if (selecionada >= totalOpcoes) {
            selecionada = 0;              /* passou da última: volta para a primeira */
        }
    }
    if (IsKeyPressed(KEY_UP)) {
        selecionada--;
        if (selecionada < 0) {
            selecionada = totalOpcoes - 1; /* antes da primeira: vai para a última */
        }
    }
    if (IsKeyPressed(KEY_ENTER)) {
        if (selecionada == 0) {
            return TELA_NAVE;
        } else if (selecionada == 1) {
            return TELA_REGRAS;
        } else if (selecionada == 2) {
            return TELA_CREDITOS;
        } else if (selecionada == 3) {
            return TELA_SAIR;
        }
    }
    return TELA_INICIO;
}

void desenharTelaInicio(void)
{
    desenharEstrelas();
    Font fonte = fontePadrao();

    DrawTextEx(fonte, "Exórbita", (Vector2){ MARGEM_ESQUERDA, Y_TITULO },
               TAMANHO_TITULO, ESPACAMENTO_TITULO, COR_TITULO);
    DrawTextEx(fonte, "Os Anos Perdidos", (Vector2){ MARGEM_ESQUERDA, Y_SUBTITULO },
               TAMANHO_SUBTITULO, ESPACAMENTO_SUBTITULO, COR_SUBTITULO);

    for (int i = 0; i < totalOpcoes; i++) {
        int y = Y_MENU + i * ESPACO_MENU;
        Color cor = COR_MENU;

        if (i == selecionada) {
            cor = COR_SELECIONADO;
            DrawRectangle(MARGEM_ESQUERDA, y + 2, TAMANHO_MARCADOR, TAMANHO_MARCADOR, COR_MARCADOR);
        }

        DrawTextEx(fonte, opcoes[i], (Vector2){ MARGEM_ESQUERDA + RECUO_MENU, y },
                   TAMANHO_MENU, 0, cor);
    }
}
