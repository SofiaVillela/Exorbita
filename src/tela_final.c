/* tela_final.c: final do jogo.

   Animação curta: a nave atravessa o espaço em direção à Via Láctea,
   que cresce na tela enquanto a nave fica menor (indo para longe).
   Depois a tela escurece e aparece "FIM". Enter pula a animação;
   na tela de FIM, Enter volta ao menu. */

#include <math.h>
#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "animacao.h"
#include "tela_final.h"
#include "partida.h"

#define DURACAO_VIAGEM   7.0f    /* segundos da nave indo até a Via Láctea */
#define DURACAO_FADE     1.5f    /* segundos escurecendo até o preto */
#define ESPERA_TECLA     1.5f    /* depois do FIM, quanto esperar para mostrar a tecla */

#define X_GALAXIA        440     /* centro da Via Láctea na tela */
#define Y_GALAXIA        160
#define ESCALA_GALAXIA_INICIO 1.0f
#define ESCALA_GALAXIA_FIM    4.0f

#define X_NAVE_INICIO    -60.0f  /* a nave entra pela esquerda... */
#define Y_NAVE_INICIO    250.0f
#define ESCALA_NAVE_INICIO 3.0f  /* ...grande, perto da câmera... */
#define ESCALA_NAVE_FIM    0.5f  /* ...e termina pequena, dentro da galáxia */

static Animacao galaxia;
static Texture2D nave;
static bool iniciou = false;
static float tempo;

/* Interpolação linear: t = 0 dá a, t = 1 dá b. */
static float misturar(float a, float b, float t)
{
    return a + (b - a) * t;
}

/* Suaviza o movimento: começa e termina devagar. */
static float suavizar(float t)
{
    if (t < 0) {
        t = 0;
    }
    if (t > 1) {
        t = 1;
    }
    return t * t * (3 - 2 * t);
}

void iniciarTelaFinal(void)
{
    if (iniciou) {
        return;
    }
    carregarAnimacao(&galaxia, "assets/imagens/via_lactea.png",
                     TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
    nave = LoadTexture("assets/imagens/nave_exterior.png");
    if (nave.id != 0) {
        SetTextureFilter(nave, TEXTURE_FILTER_POINT);
    }
    iniciou = true;
}

void entrarTelaFinal(void)
{
    if (!iniciou) {
        iniciarTelaFinal();
    }
    tempo = 0;
}

Tela atualizarTelaFinal(void)
{
    float dt = GetFrameTime();
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    tempo += dt;
    atualizarFundo();
    atualizarAnimacao(&galaxia);

    float fimDaAnimacao = DURACAO_VIAGEM + DURACAO_FADE;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_E)) {
        if (tempo < fimDaAnimacao) {
            tempo = fimDaAnimacao;       /* pula direto para o FIM */
        } else if (tempo >= fimDaAnimacao + ESPERA_TECLA) {
            marcarPartidaEmAndamento(false);   /* a partida acabou: nada para continuar */
            return TELA_INICIO;
        }
    }
    return TELA_FINAL;
}

static void desenharGalaxia(float escala)
{
    if (galaxia.folha.id == 0 || galaxia.colunas == 0) {
        return;
    }
    int coluna = galaxia.quadroAtual % galaxia.colunas;
    int linha = galaxia.quadroAtual / galaxia.colunas;
    Rectangle origem = { (float)(coluna * galaxia.larguraQuadro), (float)(linha * galaxia.alturaQuadro),
                         (float)galaxia.larguraQuadro, (float)galaxia.alturaQuadro };
    float tamanho = galaxia.larguraQuadro * escala;
    Rectangle destino = { X_GALAXIA - tamanho / 2, Y_GALAXIA - tamanho / 2, tamanho, tamanho };
    DrawTexturePro(galaxia.folha, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

static void desenharNave(float x, float y, float escala)
{
    if (nave.id == 0) {
        return;
    }
    float largura = nave.width * escala;
    float altura = nave.height * escala;
    Rectangle origem = { 0, 0, (float)nave.width, (float)nave.height };
    Rectangle destino = { x - largura / 2, y - altura / 2, largura, altura };

    /* Chama do motor piscando atrás da nave. */
    float chama = 3 + 2 * sinf(tempo * 30);
    DrawCircle((int)(x - largura / 2), (int)y, chama * escala, Fade(GetColor(0xFFD08AFF), 0.7f));

    DrawTexturePro(nave, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

void desenharTelaFinal(void)
{
    if (!iniciou) {
        return;
    }
    float fimDaAnimacao = DURACAO_VIAGEM + DURACAO_FADE;

    if (tempo < fimDaAnimacao) {
        float t = suavizar(tempo / DURACAO_VIAGEM);
        desenharFundo();
        desenharGalaxia(misturar(ESCALA_GALAXIA_INICIO, ESCALA_GALAXIA_FIM, t));
        desenharNave(misturar(X_NAVE_INICIO, X_GALAXIA, t),
                     misturar(Y_NAVE_INICIO, Y_GALAXIA, t),
                     misturar(ESCALA_NAVE_INICIO, ESCALA_NAVE_FIM, t));

        /* Escurece até o preto nos últimos DURACAO_FADE segundos. */
        if (tempo > DURACAO_VIAGEM) {
            float escuro = (tempo - DURACAO_VIAGEM) / DURACAO_FADE;
            DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, escuro));
        }
        return;
    }

    ClearBackground(BLACK);
    const char *fim = "FIM";
    int largura = (int)MeasureTextEx(fontePadrao(), fim, 32, 2).x;
    DrawTextEx(fontePadrao(), fim,
               (Vector2){ (float)(LARGURA_VIRTUAL / 2 - largura / 2), (float)(ALTURA_VIRTUAL / 2 - 16) },
               32, 2, COR_TITULO);

    if (tempo >= fimDaAnimacao + ESPERA_TECLA) {
        const char *dica = "ENTER  Voltar ao menu";
        int larguraDica = (int)MeasureTextEx(fontePadrao(), dica, 8, 0).x;
        DrawTextEx(fontePadrao(), dica,
                   (Vector2){ (float)(LARGURA_VIRTUAL / 2 - larguraDica / 2), (float)(ALTURA_VIRTUAL - 40) },
                   8, 0, COR_MENU);
    }
}

void encerrarTelaFinal(void)
{
    if (!iniciou) {
        return;
    }
    descarregarAnimacao(&galaxia);
    if (nave.id != 0) {
        UnloadTexture(nave);
    }
    iniciou = false;
}
