/* tela_inicio.c: fundo, planeta, título, subtítulo e menu.

   Sem partida em andamento: Jogar, Regras, Créditos, Sair.
   Com partida em andamento: Continuar, Nova partida, Regras, Créditos, Sair. */

#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "tela_inicio.h"
#include "fundo.h"
#include "animacao.h"
#include "partida.h"

/* static: só este arquivo enxerga essas variáveis.
   Elas guardam o valor entre uma chamada e outra. */
typedef enum {
    OPCAO_CONTINUAR,
    OPCAO_NOVA,        /* "Jogar" ou "Nova partida" */
    OPCAO_REGRAS,
    OPCAO_CREDITOS,
    OPCAO_SAIR,
    TOTAL_OPCOES_MENU
} OpcaoMenu;

static OpcaoMenu opcoes[TOTAL_OPCOES_MENU];  /* opções visíveis agora, em ordem */
static int totalOpcoes = 0;
static int selecionada = 0;   /* índice da opção em destaque */

/* Monta a lista do menu: "Continuar" só aparece com partida em andamento. */
static void montarMenu(void)
{
    totalOpcoes = 0;
    if (partidaEmAndamento()) {
        opcoes[totalOpcoes] = OPCAO_CONTINUAR;
        totalOpcoes++;
    }
    opcoes[totalOpcoes] = OPCAO_NOVA;
    totalOpcoes++;
    opcoes[totalOpcoes] = OPCAO_REGRAS;
    totalOpcoes++;
    opcoes[totalOpcoes] = OPCAO_CREDITOS;
    totalOpcoes++;
    opcoes[totalOpcoes] = OPCAO_SAIR;
    totalOpcoes++;

    if (selecionada >= totalOpcoes) {
        selecionada = 0;
    }
}

static const char *textoDaOpcao(OpcaoMenu opcao)
{
    if (opcao == OPCAO_CONTINUAR) {
        return "Continuar";
    }
    if (opcao == OPCAO_NOVA) {
        if (partidaEmAndamento()) {
            return "Nova partida";
        }
        return "Jogar";
    }
    if (opcao == OPCAO_REGRAS) {
        return "Regras";
    }
    if (opcao == OPCAO_CREDITOS) {
        return "Créditos";
    }
    return "Sair";
}

void entrarTelaInicio(void)
{
    selecionada = 0;   /* volta destacando a primeira (Continuar, se houver) */
    montarMenu();
}
static Animacao planeta;      /* planeta do Ser girando à direita */

void iniciarTelaInicio(void)
{
    carregarAnimacao(&planeta, "assets/imagens/planeta_ser.png",
                     TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
    montarMenu();
}

void encerrarTelaInicio(void)
{
    descarregarAnimacao(&planeta);
}

Tela atualizarTelaInicio(void)
{
    atualizarFundo();
    atualizarAnimacao(&planeta);
    montarMenu();

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
        OpcaoMenu opcao = opcoes[selecionada];
        if (opcao == OPCAO_CONTINUAR) {
            return TELA_NAVE;      /* volta para a nave como estava */
        } else if (opcao == OPCAO_NOVA) {
            return TELA_INTRO;     /* partida nova: primeiro a história, depois a nave */
        } else if (opcao == OPCAO_REGRAS) {
            return TELA_REGRAS;
        } else if (opcao == OPCAO_CREDITOS) {
            return TELA_CREDITOS;
        } else if (opcao == OPCAO_SAIR) {
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

        DrawTextEx(fonte, textoDaOpcao(opcoes[i]), (Vector2){ MARGEM_ESQUERDA + RECUO_MENU, y },
                   TAMANHO_MENU, 0, cor);
    }
    DrawTextEx(fonte, "PLANETAS JOGÁVEIS",
               (Vector2){ MARGEM_ESQUERDA, ALTURA_VIRTUAL - 24 }, 8, 0, COR_SUBTITULO);
}
