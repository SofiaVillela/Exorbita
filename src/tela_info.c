/* tela_info.c: telas de texto do menu (Regras e Créditos).

   Texto fixo sobre o fundo de estrelas, numa caixa. Cada tela é um
   vetor de linhas: linhas que começam com '#' são títulos de seção
   (em lilás); as outras são texto normal. Linha vazia = espaço.
   Os créditos são diferentes: só os nomes da equipe, grandes e
   centralizados, cada um com uma cabeça de astronauta, e um obrigado. */

#include <stddef.h>
#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "tela_info.h"

#define ALTURA_LINHA_INFO 11
#define ALTURA_LINHA_VAZIA 6   /* linha em branco ocupa meia altura */

static const char *regras[] = {
    "#OBJETIVO",
    "A tripulação está perdida em outra galáxia.",
    "Junte os 3 pedaços do mapa estelar: Sem Ar, Gigante e Extraterrestre.",
    "Com eles, a rota de volta à Via Láctea se abre.",
    "",
    "#NA NAVE",
    "WASD / SETAS  andar     SHIFT  correr     E  interagir",
    "TAB troca o tripulante controlado: ele é quem desce nos planetas.",
    "Terminais: tripulação, mapa estelar e comunicações.",
    "",
    "#VIAGENS",
    "Os planetas abrem em ordem: Sem Ar, Gigante, Ser, Robô e",
    "Extraterrestre. A Via Láctea abre com o mapa estelar completo.",
    "Toda viagem gasta combustível. Sem ele, a nave fica à deriva.",
    "",
    "#TRAÇOS",
    "Cada tripulante tem traços (inteligência, sorte, carisma...).",
    "Eles mudam o que acontece nos planetas. Escolha bem quem desce.",
    "",
    "#PLANETAS",
    "Sem Ar: o oxigênio acaba. Abra o portão e volte ao módulo.",
    "Gigante: junte três cristais sem pisar nas plantas brilhantes.",
    "Ser: um desejo por visita. Nem todo pedido é bom.",
    "Robô: vença os circuitos montando uma fila de comandos.",
    "Extraterrestre: jogo de blefe. Um tradutor ajuda a entender."
};

static const char *equipe[] = { "Sofia Villela", "Luis Alves", "Brenno Dornelas" };
#define TOTAL_EQUIPE 3
#define TAMANHO_NOME 16          /* letra dos nomes; a cabeça tem o mesmo tamanho */
#define Y_PRIMEIRO_NOME 100
#define ESPACO_NOMES 40

/* Cabeça de astronauta 8x8, desenhada em "escala" (2 = 16 px, a
   altura da letra dos nomes).
   C = capacete, S = sombra do capacete, V = visor, B = brilho do visor,
   G = gola laranja do traje. */
static const char *cabecaAstronauta[8] = {
    "..CCCC..",
    ".CCCCCS.",
    "CCVVVVCS",
    "CVBVVVVS",
    "CVVVVVVS",
    "CCVVVVSS",
    ".CSSSSS.",
    "..GGGG..",
};

/* Cada pessoa da equipe tem um visor de uma cor. */
static Color corVisor(int pessoa)
{
    static const unsigned int cores[] = { 0x68D8CDFF, 0x9B8CE0FF, 0xE8C86AFF };
    return GetColor(cores[pessoa % 3]);
}

static void desenharCabeca(int x, int y, int escala, Color visor)
{
    for (int linha = 0; linha < 8; linha++) {
        for (int coluna = 0; coluna < 8; coluna++) {
            char c = cabecaAstronauta[linha][coluna];
            Color cor;
            if (c == 'C') cor = COR_TITULO;
            else if (c == 'S') cor = GetColor(0x9C93B0FF);
            else if (c == 'V') cor = visor;
            else if (c == 'B') cor = WHITE;
            else if (c == 'G') cor = GetColor(0xF08A4BFF);
            else continue;
            DrawRectangle(x + coluna * escala, y + linha * escala, escala, escala, cor);
        }
    }
}

/* Desenha uma tela de texto: caixa, título grande e as linhas. */
static void desenharTexto(const char *titulo, const char *linhas[], int total)
{
    desenharFundo();
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.4f));

    int x = 24;
    int y = 16;
    int largura = LARGURA_VIRTUAL - 48;
    int altura = ALTURA_VIRTUAL - 32;
    DrawRectangle(x + 3, y + 3, largura, altura, Fade(BLACK, 0.5f));
    DrawRectangle(x, y, largura, altura, GetColor(0x211C2EFF));
    DrawRectangleLines(x, y, largura, altura, GetColor(0x8C77ADFF));
    DrawRectangle(x + 2, y + 2, largura - 4, 1, GetColor(0x514464FF));

    DrawTextEx(fontePadrao(), titulo, (Vector2){ (float)x + 20, (float)y + 16 }, 16, 1, COR_TITULO);

    int linhaY = y + 44;
    for (int i = 0; i < total; i++) {
        const char *linha = linhas[i];
        if (linha[0] == '#') {
            /* título de seção: pula o '#' e escreve em lilás */
            DrawTextEx(fontePadrao(), linha + 1, (Vector2){ (float)x + 20, (float)linhaY }, 8, 0, COR_SUBTITULO);
        } else if (linha[0] != '\0') {
            DrawTextEx(fontePadrao(), linha, (Vector2){ (float)x + 20, (float)linhaY }, 8, 0, COR_MENU);
        }
        if (linha[0] == '\0') {
            linhaY += ALTURA_LINHA_VAZIA;
        } else {
            linhaY += ALTURA_LINHA_INFO;
        }
    }

    const char *dica = "ESC / ENTER  Voltar";
    int larguraDica = (int)MeasureTextEx(fontePadrao(), dica, 8, 0).x;
    DrawTextEx(fontePadrao(), dica,
               (Vector2){ (float)(x + largura - 20 - larguraDica), (float)(y + altura - 18) }, 8, 0, COR_TITULO);
}

static bool voltou(void)
{
    return IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER);
}

Tela atualizarTelaRegras(void)
{
    atualizarFundo();
    if (voltou()) {
        return TELA_INICIO;
    }
    return TELA_REGRAS;
}

void desenharTelaRegras(void)
{
    int total = (int)(sizeof regras / sizeof regras[0]);   /* quantas linhas o vetor tem */
    desenharTexto("REGRAS", regras, total);
}

Tela atualizarTelaCreditos(void)
{
    atualizarFundo();
    if (voltou()) {
        return TELA_INICIO;
    }
    return TELA_CREDITOS;
}

/* Texto centralizado na tela. */
static void centralizado(const char *texto, int y, int tamanho, Color cor)
{
    int largura = (int)MeasureTextEx(fontePadrao(), texto, tamanho, 0).x;
    DrawTextEx(fontePadrao(), texto, (Vector2){ (float)(LARGURA_VIRTUAL / 2 - largura / 2), (float)y },
               tamanho, 0, cor);
}

void desenharTelaCreditos(void)
{
    desenharTexto("CRÉDITOS", NULL, 0);   /* só a caixa e o título */

    /* Cada nome com a cabeça à esquerda; o par (cabeça + nome) é que
       fica centralizado. */
    int espacoCabeca = TAMANHO_NOME + 10;
    for (int i = 0; i < TOTAL_EQUIPE; i++) {
        int y = Y_PRIMEIRO_NOME + i * ESPACO_NOMES;
        int larguraNome = (int)MeasureTextEx(fontePadrao(), equipe[i], TAMANHO_NOME, 0).x;
        int xInicio = LARGURA_VIRTUAL / 2 - (espacoCabeca + larguraNome) / 2;
        desenharCabeca(xInicio, y, TAMANHO_NOME / 8, corVisor(i));
        DrawTextEx(fontePadrao(), equipe[i], (Vector2){ (float)(xInicio + espacoCabeca), (float)y },
                   TAMANHO_NOME, 0, COR_TITULO);
    }

    int yObrigado = Y_PRIMEIRO_NOME + TOTAL_EQUIPE * ESPACO_NOMES + 30;
    centralizado("OBRIGADO POR JOGAR!", yObrigado, TAMANHO_NOME, COR_MARCADOR);
}
