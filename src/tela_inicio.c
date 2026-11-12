/* tela_inicio.c: fundo, planeta, título, subtítulo e menu (Jogar, Regras, Créditos, Sair). */

#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "tela_inicio.h"
#include "fundo.h"
#include "animacao.h"

/* static: só este arquivo enxerga essas variáveis.
   Elas guardam o valor entre uma chamada e outra. */
static const char *opcoes[] = { "Jogar", "Regras", "Créditos", "Sair" };
static const int totalOpcoes = 4;
static int selecionada = 0;   /* índice da opção em destaque */
static Animacao planeta;      /* planeta do Ser girando à direita */

void iniciarTelaInicio(void)
{
    carregarAnimacao(&planeta, "assets/imagens/planeta_ser.png",
                     TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
}

void encerrarTelaInicio(void)
{
    descarregarAnimacao(&planeta);
}

Tela atualizarTelaInicio(void)
{
    atualizarFundo();
    atualizarAnimacao(&planeta);

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
    desenharFundo();
    desenharAnimacao(&planeta, X_PLANETA_INICIO, Y_PLANETA_INICIO, ESCALA_PLANETA_INICIO);

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
            /* Centraliza o quadradinho na altura da letra. */
            int yMarcador = y + (TAMANHO_MENU - TAMANHO_MARCADOR) / 2;
            DrawRectangle(MARGEM_ESQUERDA, yMarcador, TAMANHO_MARCADOR, TAMANHO_MARCADOR, COR_MARCADOR);
        }

        DrawTextEx(fonte, opcoes[i], (Vector2){ MARGEM_ESQUERDA + RECUO_MENU, y },
                   TAMANHO_MENU, 0, cor);
    }
}
