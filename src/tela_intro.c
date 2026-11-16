/* tela_intro.c: introdução da história.

   Texto sobre o fundo de estrelas (uma página; dá para dividir em mais
   aumentando TOTAL_PAGINAS). As letras
   aparecem aos poucos (efeito máquina de escrever).
   Enter: se a página ainda está aparecendo, mostra tudo; se já está
   completa, vai para a próxima. Depois da última, a nave.
   Esc pula a introdução inteira. */

#include <math.h>
#include <string.h>
#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "tela_intro.h"

#define TOTAL_PAGINAS     1
#define LETRAS_POR_SEGUNDO 40.0f
#define TAMANHO_TEXTO     8
#define ALTURA_LINHA      16
#define COR_SETA_INTRO    GetColor(0x68D8CDFF)   /* verde-água das caixas de diálogo */

/* Texto da história: uma string por página, linhas separadas por \n. */
static const char *paginas[TOTAL_PAGINAS] = {
    "Seis tripulantes partiram em uma missão de rotina.\n"
    "No meio do salto, algo deu errado.\n"
    "\n"
    "Quando acordaram, as estrelas eram outras.\n"
    "A nave estava em uma galáxia distante,\n"
    "longe da Via Láctea.\n"
    "\n"
    "O combustível é pouco.\n"
    "O caminho de volta é desconhecido.\n"
    "Os planetas desta galáxia podem ter as respostas.\n"
    "\n"
    "Juntem os pedaços do mapa estelar.\n"
    "Só assim a rota para casa vai se abrir."
};

static int pagina;
static float tempoPagina;   /* segundos desde que a página começou */

/* Quantos caracteres (codepoints, não bytes) a página tem. Letras com
   acento ocupam mais de um byte em UTF-8, por isso não dá para usar strlen. */
static int contarLetras(const char *texto)
{
    int total = 0;
    int i = 0;
    while (texto[i] != '\0') {
        int tamanho = 0;
        GetCodepointNext(&texto[i], &tamanho);
        i += tamanho;
        total++;
    }
    return total;
}

static int letrasVisiveis(void)
{
    return (int)(tempoPagina * LETRAS_POR_SEGUNDO);
}

static bool paginaCompleta(void)
{
    return letrasVisiveis() >= contarLetras(paginas[pagina]);
}

void entrarTelaIntro(void)
{
    pagina = 0;
    tempoPagina = 0;
}

Tela atualizarTelaIntro(void)
{
    float dt = GetFrameTime();
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    tempoPagina += dt;
    atualizarFundo();

    if (IsKeyPressed(KEY_ESCAPE)) {
        return TELA_NAVE;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE)) {
        if (!paginaCompleta()) {
            tempoPagina = 1000;            /* mostra a página inteira */
        } else {
            pagina++;
            tempoPagina = 0;
            if (pagina >= TOTAL_PAGINAS) {
                return TELA_NAVE;
            }
        }
    }
    return TELA_INTRO;
}

/* Desenha a página linha por linha, centralizada, mostrando só as
   primeiras "limite" letras. Cada linha é centralizada pela largura
   completa, para o texto não andar para os lados enquanto aparece. */
static void desenharPagina(const char *texto, int limite)
{
    int linhas = 1;
    for (const char *c = texto; *c != '\0'; c++) {
        if (*c == '\n') {
            linhas++;
        }
    }
    int y = ALTURA_VIRTUAL / 2 - (linhas * ALTURA_LINHA) / 2;

    char linha[128];
    char visivel[128];
    int letrasUsadas = 0;
    const char *inicio = texto;

    while (*inicio != '\0' && letrasUsadas <= limite) {
        /* Copia a linha atual (até o \n ou o fim). */
        const char *fim = strchr(inicio, '\n');
        int bytes = (int)strlen(inicio);
        if (fim != NULL) {
            bytes = (int)(fim - inicio);
        }
        if (bytes > (int)sizeof linha - 1) {
            bytes = (int)sizeof linha - 1;
        }
        memcpy(linha, inicio, (size_t)bytes);
        linha[bytes] = '\0';

        /* Parte visível da linha: anda letra por letra até o limite. */
        int i = 0;
        while (linha[i] != '\0' && letrasUsadas < limite) {
            int tamanho = 0;
            GetCodepointNext(&linha[i], &tamanho);
            i += tamanho;
            letrasUsadas++;
        }
        memcpy(visivel, linha, (size_t)i);
        visivel[i] = '\0';

        int largura = (int)MeasureTextEx(fontePadrao(), linha, TAMANHO_TEXTO, 0).x;
        int x = LARGURA_VIRTUAL / 2 - largura / 2;
        DrawTextEx(fontePadrao(), visivel, (Vector2){ (float)x, (float)y }, TAMANHO_TEXTO, 0, COR_TITULO);

        y += ALTURA_LINHA;
        if (fim == NULL) {
            break;
        }
        inicio = fim + 1;
        letrasUsadas++;   /* o \n conta como uma letra, dá uma pausinha entre linhas */
    }
}

void desenharTelaIntro(void)
{
    desenharFundo();
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.35f));

    desenharPagina(paginas[pagina], letrasVisiveis());

    /* Rodapé: número da página (só se houver mais de uma) e teclas. */
    if (TOTAL_PAGINAS > 1) {
        DrawTextEx(fontePadrao(), TextFormat("%d/%d", pagina + 1, TOTAL_PAGINAS),
                   (Vector2){ MARGEM_ESQUERDA, ALTURA_VIRTUAL - 32 }, 8, 0, COR_SUBTITULO);
    }
    const char *dica = "ENTER  Continuar   ESC  Pular";
    int largura = (int)MeasureTextEx(fontePadrao(), dica, 8, 0).x;
    DrawTextEx(fontePadrao(), dica,
               (Vector2){ (float)(LARGURA_VIRTUAL - MARGEM_ESQUERDA - largura), ALTURA_VIRTUAL - 32 },
               8, 0, COR_MENU);

    /* Seta piscando quando a página terminou de aparecer. */
    if (paginaCompleta() && fmod(GetTime(), 1.0) < 0.6) {
        float x = LARGURA_VIRTUAL / 2.0f;
        float y = ALTURA_VIRTUAL - 60.0f;
        DrawTriangle((Vector2){ x - 4, y }, (Vector2){ x, y + 5 }, (Vector2){ x + 4, y }, COR_SETA_INTRO);
    }
}
