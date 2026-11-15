/* memorial.c: tela de despedida de um tripulante (ver memorial.h). */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "memorial.h"

#define X_TEXTO           280
#define Y_TITULO_MEMORIAL 48
#define Y_SINAL           74     /* caixa do sinal do coração */
#define LARGURA_SINAL     320
#define ALTURA_SINAL      30
#define VELOCIDADE_SINAL  80.0f  /* pixels por segundo da linha do sinal */
#define Y_NOME            124
#define LETRAS_POR_SEGUNDO 42.0f
#define PAUSA_ENTRE_LINHAS 0.30f
#define INICIO_TEXTO      2.2f   /* o texto espera o sinal começar a falhar */
#define TEMPO_SURGIR      0.8f   /* a tela surge do preto */
#define TOTAL_DESTROCOS   12
#define MAX_TEXTOS        (3 + MEMORIAL_MAX_LINHAS)

typedef struct {
    char texto[MEMORIAL_TAM_LINHA];
    int y;
    int tamanho;     /* 8 ou 16 */
    Color cor;
    float inicio;    /* instante em que começa a aparecer */
    int letras;
} TextoMemorial;

static TextoMemorial textos[MAX_TEXTOS];
static int totalTextos;
static float tempo;
static float fimDoTexto;     /* instante em que a última letra aparece */

/* Conta letras, não bytes (em UTF-8 as acentuadas ocupam 2 bytes). */
static int contarLetras(const char *s)
{
    int letras = 0;
    for (const char *c = s; *c != '\0'; c++) {
        if (((unsigned char)*c & 0xC0) != 0x80) {
            letras++;
        }
    }
    return letras;
}

/* Copia só as primeiras "letras" letras de origem para destino. */
static void copiarLetras(char *destino, size_t tamanho, const char *origem, int letras)
{
    size_t i = 0;
    int vistas = 0;
    while (origem[i] != '\0' && i + 1 < tamanho) {
        if (((unsigned char)origem[i] & 0xC0) != 0x80) {
            if (vistas == letras) {
                break;
            }
            vistas++;
        }
        destino[i] = origem[i];
        i++;
    }
    destino[i] = '\0';
}

static void acrescentar(const char *texto, int y, int tamanho, Color cor, float *relogio)
{
    if (texto == NULL || texto[0] == '\0' || totalTextos >= MAX_TEXTOS) {
        return;
    }
    TextoMemorial *t = &textos[totalTextos];
    snprintf(t->texto, sizeof t->texto, "%s", texto);
    t->y = y;
    t->tamanho = tamanho;
    t->cor = cor;
    t->inicio = *relogio;
    t->letras = contarLetras(t->texto);
    *relogio += (float)t->letras / LETRAS_POR_SEGUNDO + PAUSA_ENTRE_LINHAS;
    totalTextos++;
}

void abrirMemorial(const DadosMemorial *dados)
{
    totalTextos = 0;
    tempo = 0;
    float relogio = INICIO_TEXTO;
    if (dados == NULL) {
        fimDoTexto = relogio;
        return;
    }

    char nome[MEMORIAL_TAM_LINHA];
    snprintf(nome, sizeof nome, "%s, %d anos", dados->nome, dados->idade);
    acrescentar(nome, Y_NOME, 16, COR_TITULO, &relogio);
    acrescentar(dados->causa, Y_NOME + 28, 8, COR_TITULO, &relogio);
    if (dados->destaque != NULL) {
        char frase[MEMORIAL_TAM_LINHA];
        snprintf(frase, sizeof frase, "\"%s\"", dados->destaque);
        acrescentar(frase, Y_NOME + 44, 8, COR_SUBTITULO, &relogio);
    }
    for (int i = 0; i < dados->totalLinhas && i < MEMORIAL_MAX_LINHAS; i++) {
        acrescentar(dados->linhas[i], Y_NOME + 72 + i * 16, 8, COR_MENU, &relogio);
    }
    fimDoTexto = relogio;
}

bool atualizarMemorial(float dt)
{
    atualizarFundo();
    tempo += dt;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) {
        if (tempo < fimDoTexto) {
            tempo = fimDoTexto;   /* primeiro aperto: mostra tudo de uma vez */
        } else {
            return true;
        }
    }
    return false;
}

/* Altura da linha do sinal em cada ponto: três batidas, cada uma mais
   fraca, e depois a linha reta. */
static float alturaSinal(float x)
{
    static const float inicios[3] = { 26, 104, 182 };
    static const float forcas[3] = { 1.0f, 0.6f, 0.25f };
    for (int i = 0; i < 3; i++) {
        float d = x - inicios[i];
        if (d < 0 || d >= 20) {
            continue;
        }
        float f = forcas[i];
        if (d < 4)  return -3 * f * d / 4;                 /* sobe um pouco */
        if (d < 8)  return (-3 + 15 * (d - 4) / 4) * f;    /* desce fundo */
        if (d < 12) return (12 - 23 * (d - 8) / 4) * f;    /* pico */
        return (-11 + 11 * (d - 12) / 8) * f;              /* volta ao meio */
    }
    return 0;
}

static void desenharSinal(void)
{
    int x0 = X_TEXTO;
    int meio = Y_SINAL + ALTURA_SINAL / 2;
    DrawRectangle(x0, Y_SINAL, LARGURA_SINAL, ALTURA_SINAL, Fade(GetColor(0x211C2EFF), 0.85f));
    DrawRectangleLines(x0, Y_SINAL, LARGURA_SINAL, ALTURA_SINAL, GetColor(0x514464FF));
    for (int x = x0 + 16; x < x0 + LARGURA_SINAL; x += 16) {
        DrawRectangle(x, Y_SINAL + 1, 1, ALTURA_SINAL - 2, Fade(GetColor(0x514464FF), 0.4f));
    }

    int ponta = (int)(tempo * VELOCIDADE_SINAL);
    if (ponta > LARGURA_SINAL - 2) {
        ponta = LARGURA_SINAL - 2;
    }
    for (int x = 1; x < ponta; x++) {
        int ya = meio - (int)alturaSinal((float)x - 1);
        int yb = meio - (int)alturaSinal((float)x);
        DrawLine(x0 + x - 1, ya, x0 + x, yb, COR_ALERTA);
    }
    /* A ponta brilha enquanto anda; parada, pisca devagar. */
    bool andando = ponta < LARGURA_SINAL - 2;
    if (andando || fmodf(tempo, 1.0f) < 0.5f) {
        DrawRectangle(x0 + ponta - 1, meio - (int)alturaSinal((float)ponta) - 1, 3, 3, COR_TITULO);
    }
    if (!andando) {
        DrawTextEx(fontePadrao(), "0 BPM", (Vector2){ (float)(x0 + LARGURA_SINAL - 48), (float)(Y_SINAL + 4) },
                   8, 0, COR_ALERTA);
    }
}

static void desenharTraje(Texture2D folha)
{
    /* Destroços pequenos girando devagar em volta do traje. */
    for (int i = 0; i < TOTAL_DESTROCOS; i++) {
        float angulo = tempo * (0.10f + 0.02f * (float)(i % 4)) + (float)i * 0.9f;
        float raio = 70 + (float)((i * 37) % 60);
        int x = 140 + (int)(cosf(angulo) * raio);
        int y = 184 + (int)(sinf(angulo) * raio * 0.7f);
        int lado = 2 + i % 3;
        DrawRectangle(x, y, lado, lado, Fade(GetColor(0x8E7BD9FF), 0.55f));
        DrawRectangle(x, y, 1, 1, Fade(GetColor(0xD6CBF8FF), 0.8f));
    }
    if (folha.id == 0) {
        return;
    }
    /* O traje à deriva: gira devagar e sobe e desce um pouco. */
    float x = 140 + sinf(tempo * 0.5f) * 6;
    float y = 184 + sinf(tempo * 0.8f) * 8;
    Rectangle origem = { 0, 0, LARGURA_PERSONAGEM, ALTURA_PERSONAGEM };
    Rectangle destino = { x, y, LARGURA_PERSONAGEM * 3, ALTURA_PERSONAGEM * 3 };
    Vector2 centro = { LARGURA_PERSONAGEM * 1.5f, ALTURA_PERSONAGEM * 1.5f };
    DrawTexturePro(folha, origem, destino, centro, tempo * 9.0f, GetColor(0xB9B2CCFF));
}

void desenharMemorial(Texture2D folhaTraje, const Animacao *planeta)
{
    desenharFundo();
    /* Escurece o espaço, para a tela ficar mais séria que o menu. */
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(GetColor(0x100D16FF), 0.6f));
    if (planeta != NULL && planeta->folha.id != 0) {
        /* O planeta fica no canto, atrás do traje. */
        desenharAnimacao(planeta, 8, ALTURA_VIRTUAL - TAMANHO_PLANETA * 2 + 40, 2);
    }
    desenharTraje(folhaTraje);

    /* Título: pisca nos primeiros instantes, como um alarme. */
    if (tempo > 1.2f || fmodf(tempo, 0.4f) < 0.25f) {
        DrawTextEx(fontePadrao(), "SINAL PERDIDO", (Vector2){ X_TEXTO, Y_TITULO_MEMORIAL }, 16, 0, COR_ALERTA);
    }
    desenharSinal();

    for (int i = 0; i < totalTextos; i++) {
        const TextoMemorial *t = &textos[i];
        if (tempo < t->inicio) {
            continue;
        }
        int letras = (int)((tempo - t->inicio) * LETRAS_POR_SEGUNDO);
        char parte[MEMORIAL_TAM_LINHA];
        copiarLetras(parte, sizeof parte, t->texto, letras);
        DrawTextEx(fontePadrao(), parte, (Vector2){ X_TEXTO, (float)t->y }, (float)t->tamanho, 0, t->cor);
    }

    if (tempo >= fimDoTexto && fmodf(tempo, 1.0f) < 0.65f) {
        DrawRectangle(X_TEXTO, ALTURA_VIRTUAL - 40, 6, 6, COR_MARCADOR);
        DrawTextEx(fontePadrao(), "ENTER  Voltar à nave", (Vector2){ X_TEXTO + 14, ALTURA_VIRTUAL - 41 },
                   8, 0, COR_SELECIONADO);
    }

    if (tempo < TEMPO_SURGIR) {
        DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 1 - tempo / TEMPO_SURGIR));
    }
}
