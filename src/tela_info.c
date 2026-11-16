/* tela_info.c: telas de texto do menu (Regras e Créditos).

   Texto fixo sobre o fundo de estrelas, numa caixa. Cada tela é um
   vetor de linhas: linhas que começam com '#' são títulos de seção
   (em lilás); as outras são texto normal. Linha vazia = espaço. */

#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "tela_info.h"

#define ALTURA_LINHA_INFO 12
#define ALTURA_LINHA_VAZIA 6   /* linha em branco ocupa meia altura */

static const char *regras[] = {
    "#OBJETIVO",
    "A tripulação está perdida em outra galáxia.",
    "Visite todos os planetas para abrir a rota de volta à Via Láctea.",
    "",
    "#NA NAVE",
    "WASD / SETAS  andar     SHIFT  correr     E  interagir",
    "TAB troca o tripulante controlado: ele é quem desce nos planetas.",
    "Terminais: tripulação, mapa estelar e comunicações.",
    "",
    "#VIAGENS",
    "Toda viagem gasta combustível, mas sempre sobra o necessário",
    "para a Via Láctea, que abre quando todos forem visitados.",
    "",
    "#TRAÇOS",
    "Cada tripulante tem traços (inteligência, sorte, carisma...).",
    "Eles mudam o que acontece nos planetas. Escolha bem quem desce.",
    "",
    "#PLANETAS",
    "Robô: programe o caminho do robô com uma fila de comandos.",
    "Extraterrestre: jogo de blefe. Um tradutor ajuda a entender.",
    "Gigante: atravesse as áreas sem ser visto pelos olhos.",
    "Ser: um desejo por visita. Nem todo pedido é bom."
};

static const char *creditos[] = {
    "#EXÓRBITA: OS ANOS PERDIDOS",
    "Jogo da disciplina Algoritmos e Estruturas de Dados",
    "CESAR School, 2026",
    "",
    "#EQUIPE",
    "Sofia Villela",
    "  Nave, painel de planetas, planeta do Ser, história e final",
    "Luis Alves",
    "  Planeta do robô e planeta sem ar",
    "Brenno Dornelas",
    "  Planeta extraterrestre e planeta gigante",
    "",
    "#FERRAMENTAS",
    "Linguagem C (C99) e raylib 5.5",
    "Fonte Press Start 2P, de CodeMan38 (licença OFL)",
    "",
    "Obrigado por jogar!"
};

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

void desenharTelaCreditos(void)
{
    int total = (int)(sizeof creditos / sizeof creditos[0]);
    desenharTexto("CRÉDITOS", creditos, total);
}
