/* tela_final.c: finais do jogo.

   Final bom: a nave atravessa o espaço em direção à Via Láctea,
   que cresce na tela enquanto a nave fica menor (indo para longe).

   Final ruim: ninguém restou a bordo, ou o combustível não dá para
   nenhum planeta (tela_nave.c avisa por marcarFinalSemCombustivel). O motor falha e apaga, as luzes
   da nave escurecem e ela gira devagar, à deriva, se afastando. A Via
   Láctea fica pequena no canto, longe demais.

   Nos dois, a tela escurece e aparece "FIM". Enter pula a animação;
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

/* Final ruim */
#define DURACAO_DERIVA   10.0f   /* segundos da nave à deriva */
#define MOTOR_FALHANDO   2.5f    /* até aqui a chama ainda pisca, depois apaga */
#define LUZES_APAGANDO   4.0f    /* segundos até a nave escurecer de vez */
#define FRASE_1          3.0f    /* quando aparece a primeira frase */
#define FRASE_2          5.5f    /* quando aparece a segunda */
#define X_DERIVA_INICIO  300.0f
#define Y_DERIVA_INICIO  190.0f
#define X_DERIVA_FIM     200.0f
#define Y_DERIVA_FIM     230.0f
#define ESCALA_DERIVA_INICIO 3.0f
#define ESCALA_DERIVA_FIM    1.5f
#define GIRO_POR_SEGUNDO 9.0f    /* graus */

typedef enum { FINAL_BOM, FINAL_RUIM } TipoFinal;

static Animacao galaxia;
static Texture2D nave;
static bool iniciou = false;
static float tempo;
static TipoFinal final;
static bool semCombustivel = false;   /* final ruim por falta de combustível */
static bool pedidoSemCombustivel = false;

void marcarFinalSemCombustivel(void)
{
    pedidoSemCombustivel = true;
}

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

    /* Ninguém vivo a bordo = final ruim. */
    final = FINAL_RUIM;
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        if (progressoPartida()->tripulacao[i].vivo) {
            final = FINAL_BOM;
        }
    }

    /* Sem combustível para ir a lugar nenhum = final ruim também. */
    semCombustivel = pedidoSemCombustivel && final == FINAL_BOM;
    pedidoSemCombustivel = false;
    if (semCombustivel) {
        final = FINAL_RUIM;
    }
}

/* Quanto dura a animação do final escolhido, antes do FIM. */
static float duracaoAnimacao(void)
{
    if (final == FINAL_RUIM) {
        return DURACAO_DERIVA + DURACAO_FADE;
    }
    return DURACAO_VIAGEM + DURACAO_FADE;
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

    float fimDaAnimacao = duracaoAnimacao();
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

static void desenharGalaxia(float x, float y, float escala, Color cor)
{
    if (galaxia.folha.id == 0 || galaxia.colunas == 0) {
        return;
    }
    int coluna = galaxia.quadroAtual % galaxia.colunas;
    int linha = galaxia.quadroAtual / galaxia.colunas;
    Rectangle origem = { (float)(coluna * galaxia.larguraQuadro), (float)(linha * galaxia.alturaQuadro),
                         (float)galaxia.larguraQuadro, (float)galaxia.alturaQuadro };
    float tamanho = galaxia.larguraQuadro * escala;
    Rectangle destino = { x - tamanho / 2, y - tamanho / 2, tamanho, tamanho };
    DrawTexturePro(galaxia.folha, origem, destino, (Vector2){ 0, 0 }, 0, cor);
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

/* Frase centralizada embaixo, aparecendo aos poucos a partir de "inicio". */
static void desenharFrase(const char *frase, float inicio, float y)
{
    if (tempo < inicio) {
        return;
    }
    float alfa = (tempo - inicio) / 1.0f;
    if (alfa > 1) {
        alfa = 1;
    }
    int largura = (int)MeasureTextEx(fontePadrao(), frase, 8, 0).x;
    DrawTextEx(fontePadrao(), frase, (Vector2){ (float)(LARGURA_VIRTUAL / 2 - largura / 2), y },
               8, 0, Fade(COR_MENU, alfa));
}

/* A chama do motor falha: pisca de forma irregular e depois apaga. */
static bool motorAceso(void)
{
    if (tempo > MOTOR_FALHANDO) {
        return false;
    }
    /* Duas ondas de velocidades diferentes: o pisca fica irregular. */
    float onda = sinf(tempo * 23) + sinf(tempo * 9);
    float limite = tempo / MOTOR_FALHANDO;   /* vai apagando mais com o tempo */
    return onda > limite;
}

static void desenharNaveDeriva(float x, float y, float escala, float angulo, Color cor)
{
    if (nave.id == 0) {
        return;
    }
    float largura = nave.width * escala;
    float altura = nave.height * escala;

    if (motorAceso()) {
        /* A traseira da nave, girada junto com ela. */
        float radianos = angulo * DEG2RAD;
        float trasX = x - cosf(radianos) * largura / 2;
        float trasY = y - sinf(radianos) * largura / 2;
        DrawCircle((int)trasX, (int)trasY, 3 * escala, Fade(GetColor(0xFFD08AFF), 0.7f));
    }

    Rectangle origem = { 0, 0, (float)nave.width, (float)nave.height };
    Rectangle destino = { x, y, largura, altura };
    Vector2 centro = { largura / 2, altura / 2 };   /* gira em volta do meio */
    DrawTexturePro(nave, origem, destino, centro, angulo, cor);
}

static void desenharFinalRuim(void)
{
    float t = tempo / DURACAO_DERIVA;
    if (t > 1) {
        t = 1;
    }
    desenharFundo();

    /* A Via Láctea pequena e apagada no canto: longe demais. */
    desenharGalaxia(560, 70, 0.6f, Fade(WHITE, 0.6f));

    /* As luzes apagam: a cor vai de branco a cinza escuro. */
    float apagado = tempo / LUZES_APAGANDO;
    if (apagado > 1) {
        apagado = 1;
    }
    unsigned char brilho = (unsigned char)misturar(255, 90, apagado);
    Color cor = { brilho, brilho, brilho, 255 };

    desenharNaveDeriva(misturar(X_DERIVA_INICIO, X_DERIVA_FIM, t),
                       misturar(Y_DERIVA_INICIO, Y_DERIVA_FIM, t),
                       misturar(ESCALA_DERIVA_INICIO, ESCALA_DERIVA_FIM, t),
                       tempo * GIRO_POR_SEGUNDO, cor);

    if (semCombustivel) {
        desenharFrase("O combustível acabou.", FRASE_1, ALTURA_VIRTUAL - 52);
        desenharFrase("Sem ter para onde ir, a nave fica à deriva, longe de casa.",
                      FRASE_2, ALTURA_VIRTUAL - 36);
    } else {
        desenharFrase("Ninguém restou a bordo.", FRASE_1, ALTURA_VIRTUAL - 52);
        desenharFrase("A nave segue à deriva, longe de casa.", FRASE_2, ALTURA_VIRTUAL - 36);
    }

    if (tempo > DURACAO_DERIVA) {
        float escuro = (tempo - DURACAO_DERIVA) / DURACAO_FADE;
        DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, escuro));
    }
}

static void desenharFinalBom(void)
{
    float t = suavizar(tempo / DURACAO_VIAGEM);
    desenharFundo();
    desenharGalaxia(X_GALAXIA, Y_GALAXIA, misturar(ESCALA_GALAXIA_INICIO, ESCALA_GALAXIA_FIM, t), WHITE);
    desenharNave(misturar(X_NAVE_INICIO, X_GALAXIA, t),
                 misturar(Y_NAVE_INICIO, Y_GALAXIA, t),
                 misturar(ESCALA_NAVE_INICIO, ESCALA_NAVE_FIM, t));

    /* Escurece até o preto nos últimos DURACAO_FADE segundos. */
    if (tempo > DURACAO_VIAGEM) {
        float escuro = (tempo - DURACAO_VIAGEM) / DURACAO_FADE;
        DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, escuro));
    }
}

void desenharTelaFinal(void)
{
    if (!iniciou) {
        return;
    }
    float fimDaAnimacao = duracaoAnimacao();

    if (tempo < fimDaAnimacao) {
        if (final == FINAL_RUIM) {
            desenharFinalRuim();
        } else {
            desenharFinalBom();
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
