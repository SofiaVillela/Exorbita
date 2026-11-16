/* tela_ser.c: planeta do Ser dos desejos.

   Um santuário de gelo. O tripulante que desceu anda até o altar no
   centro. Só ao chegar perto descobre se o Ser está lá (o sorteio é
   feito no pouso, em entrarTelaSer). Se estiver, E abre a conversa e o
   Ser oferece os desejos que esse tripulante pode pedir (filtro em
   ser_logica.c). Um desejo por visita; depois do resultado, volta à nave.
   Neste planeta o oxigênio não é gasto. */

#include <math.h>
#include <string.h>
#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "cenario_planeta.h"
#include "partida.h"
#include "ser_logica.h"
#include "mapa_estelar.h"
#include "dialogo.h"
#include "tela_ser.h"
#include "sons.h"

typedef enum {
    SER_EXPLORANDO,   /* andando pelo santuário */
    SER_AUSENTE,      /* chegou ao altar e o Ser não estava */
    SER_FALANDO,      /* fala de abertura do Ser */
    SER_ESCOLHENDO,   /* lista de desejos */
    SER_RESULTADO     /* o que o desejo causou; depois volta à nave */
} EstadoSer;

#define X_SER            320     /* pés do Ser, no centro do altar */
#define Y_SER            292     /* baixo o bastante para a cabeça caber na tela */
#define X_REVELA         200     /* passando daqui, o jogador "chega" ao santuário */
#define DISTANCIA_FALAR  100     /* distância dos pés até o Ser para o E funcionar */
#define BASE_SPRITE_SER  96      /* última linha do manto no ser.png (linhas 0 a 95) */
#define ORBE_X           32      /* centro do orbe dentro do ser.png */
#define ORBE_Y           58
#define TOTAL_PARTICULAS 28

static CenarioPlaneta cenario;
static ExploradorPlaneta explorador;
static Texture2D spriteSer;
static bool iniciou = false;

static EstadoSer estado;
static bool presente;        /* resultado do sorteio deste pouso */
static bool revelado;        /* o jogador já chegou ao santuário? */
static float tempo;          /* relógio para as animações */
static float aparecer;       /* 0 a 1: o Ser surge aos poucos */

static TipoDesejo opcoes[TOTAL_DESEJOS];
static int totalOpcoes;
static int escolhida;
static char resultado[256];

/* Base do manto do Ser: bloqueia a passagem quando ele está visível.
   O manto tem 64 px de largura no sprite; em 3x, 192. A base fica
   mais estreita para o jogador conseguir chegar perto. */
static const ObstaculoExploracao obstaculoSer = { X_SER - 76, Y_SER - 14, 152, 14 };

/* ---------- Funções de apoio ---------- */

static bool serVisivel(void)
{
    return presente && revelado;
}

static bool pertoDoSer(void)
{
    float dx = explorador.pes.x - X_SER;
    float dy = explorador.pes.y - Y_SER;
    return dx * dx + dy * dy <= DISTANCIA_FALAR * DISTANCIA_FALAR;
}

static bool confirmou(void)
{
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E);
}

/* ---------- Funções do tela_ser.h ---------- */

void iniciarTelaSer(void)
{
    if (iniciou) {
        return;
    }
    iniciarCenarioPlaneta(&cenario, "assets/imagens/planetas/ser_santuario.png");
    iniciarExploradorPlaneta(&explorador, (PontoExploracao){ 56, 316 });
    spriteSer = LoadTexture("assets/imagens/ser.png");
    if (spriteSer.id != 0) {
        SetTextureFilter(spriteSer, TEXTURE_FILTER_POINT);
    }
    iniciou = true;
}

void entrarTelaSer(void)
{
    if (!iniciou) {
        iniciarTelaSer();
    }
    ProgressoPartida *jogo = progressoPartida();
    const Tripulante *quem = tripulanteControlado();

    /* O sorteio acontece no pouso, mas o jogador só vê ao chegar ao altar. */
    int chance = chanceSer(jogo->visitasSer, quem->tracos.sorte);
    presente = serApareceu(chance, GetRandomValue(1, 100));
    jogo->visitasSer++;

    explorador.pes = (PontoExploracao){ 56, 316 };
    explorador.direcao = 3;
    explorador.quadro = 0;
    explorador.tempo = 0;
    explorador.correndo = false;

    estado = SER_EXPLORANDO;
    revelado = false;
    aparecer = 0;
    tempo = 0;
    escolhida = 0;
    totalOpcoes = 0;
    resultado[0] = '\0';
}

Tela atualizarTelaSer(void)
{
    float dt = GetFrameTime();
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    tempo += dt;
    if (serVisivel() && aparecer < 1) {
        if (aparecer == 0) {
            tocarSom(SOM_SER_APARECE);     /* o Ser começa a surgir */
        }
        aparecer += dt;
        if (aparecer > 1) {
            aparecer = 1;
        }
    }

    if (estado == SER_AUSENTE) {
        if (confirmou() || IsKeyPressed(KEY_ESCAPE)) {
            estado = SER_EXPLORANDO;
        }
        return TELA_SER;
    }

    if (estado == SER_FALANDO) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            estado = SER_EXPLORANDO;
        } else if (confirmou()) {
            ProgressoPartida *jogo = progressoPartida();
            totalOpcoes = desejosDisponiveis(jogo, jogo->controlado, listaPlanetasMapa(), opcoes);
            escolhida = 0;
            estado = SER_ESCOLHENDO;
        }
        return TELA_SER;
    }

    if (estado == SER_ESCOLHENDO) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            estado = SER_EXPLORANDO;   /* desistiu: o desejo continua disponível */
            return TELA_SER;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            tocarSom(SOM_UI_MOVER);
            escolhida++;
            if (escolhida >= totalOpcoes) {
                escolhida = 0;
            }
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            tocarSom(SOM_UI_MOVER);
            escolhida--;
            if (escolhida < 0) {
                escolhida = totalOpcoes - 1;
            }
        }
        if (IsKeyPressed(KEY_ENTER) && totalOpcoes > 0) {
            ProgressoPartida *jogo = progressoPartida();
            const char *texto = aplicarDesejo(jogo, jogo->controlado, listaPlanetasMapa(),
                                              opcoes[escolhida]);
            strncpy(resultado, texto, sizeof resultado - 1);
            resultado[sizeof resultado - 1] = '\0';
            tocarSom(SOM_DESEJO);
            estado = SER_RESULTADO;
        }
        return TELA_SER;
    }

    if (estado == SER_RESULTADO) {
        if (confirmou() || IsKeyPressed(KEY_ESCAPE)) {
            return TELA_NAVE;          /* um desejo por visita */
        }
        return TELA_SER;
    }

    /* SER_EXPLORANDO */
    if (IsKeyPressed(KEY_ESCAPE)) {
        return TELA_NAVE;
    }

    int quantidade = 0;
    if (serVisivel()) {
        quantidade = 1;
    }
    atualizarExploradorPlaneta(&explorador, &cenario.piso, &obstaculoSer, quantidade, dt, true);

    /* Chegou ao santuário: agora o jogador descobre se o Ser está lá. */
    if (!revelado && explorador.pes.x >= X_REVELA) {
        revelado = true;
        if (!presente) {
            estado = SER_AUSENTE;
        }
    }

    if (IsKeyPressed(KEY_E) && serVisivel() && pertoDoSer()) {
        estado = SER_FALANDO;
    }
    return TELA_SER;
}

/* ---------- Desenho ---------- */

static void desenharParticulas(void)
{
    for (int i = 0; i < TOTAL_PARTICULAS; i++) {
        /* Cada partícula sobe devagar e recomeça embaixo; x e velocidade
           saem do índice, então não precisam ser guardados. */
        float x = 24 + (float)((i * 97) % 592);
        float velocidade = 6 + (float)(i % 5) * 3;
        float altura = 330;
        float y = 340 - fmodf(tempo * velocidade + (float)(i * 41), altura);
        float brilho = 0.35f + 0.35f * sinf(tempo * 2 + (float)i);
        DrawRectangle((int)x, (int)y, 2, 2, Fade(GetColor(0xD8F2FFFF), brilho));
    }
}

static void desenharSer(void)
{
    if (!serVisivel()) {
        return;
    }
    float flutua = sinf(tempo * 2.0f) * 2.0f;
    Color cor = Fade(WHITE, aparecer);

    if (spriteSer.id == 0) {
        /* Sem a imagem: um retângulo no lugar, para o jogo continuar. */
        DrawRectangle(X_SER - 20, Y_SER - 120, 40, 120, Fade(GetColor(0xE9E6F4FF), aparecer));
        return;
    }

    /* Ampliado ESCALA_SER vezes, com a base do manto nos pés (X_SER, Y_SER). */
    int largura = spriteSer.width * ESCALA_SER;
    int altura = spriteSer.height * ESCALA_SER;
    int x = X_SER - largura / 2;
    int y = Y_SER - BASE_SPRITE_SER * ESCALA_SER + (int)flutua;
    Rectangle origem = { 0, 0, (float)spriteSer.width, (float)spriteSer.height };
    Rectangle destino = { (float)x, (float)y, (float)largura, (float)altura };
    DrawTexturePro(spriteSer, origem, destino, (Vector2){ 0, 0 }, 0, cor);

    /* Brilho do orbe, pulsando por cima do desenho. */
    float pulso = 0.15f + 0.10f * sinf(tempo * 3.0f);
    int orbeX = x + ORBE_X * ESCALA_SER;
    int orbeY = y + ORBE_Y * ESCALA_SER;
    DrawCircle(orbeX, orbeY, 22, Fade(GetColor(0xF6F2A0FF), pulso * aparecer));
    DrawCircle(orbeX, orbeY, 10, Fade(GetColor(0xFFFFFFFF), (pulso + 0.15f) * aparecer));
}

static void desenharInterface(void)
{
    ProgressoPartida *jogo = progressoPartida();
    caixaPlaneta(16, 12, 300, 44);
    textoPlaneta("PLANETA DO SER", 26, 20, 16, COR_TITULO);
    textoPlaneta(TextFormat("Visitante: %s", tripulanteControlado()->nome), 26, 41, 8, COR_SUBTITULO);

    caixaPlaneta(456, 12, 168, 28);
    textoPlaneta(TextFormat("COMBUSTÍVEL %d", jogo->combustivel), 468, 22, 8, COR_MENU);

    if (estado == SER_EXPLORANDO) {
        if (!revelado) {
            caixaPlaneta(16, 64, 312, 22);
            textoPlaneta("Siga até o altar no centro do santuário.", 26, 71, 8, COR_MENU);
        } else if (serVisivel() && pertoDoSer()) {
            /* A dica fica acima da cabeça do jogador, para não cobrir o Ser. */
            int dicaX = (int)explorador.pes.x - 76;
            int dicaY = (int)explorador.pes.y - 90;
            caixaPlaneta(dicaX, dicaY, 152, 22);
            textoPlaneta("E  Falar com o Ser", (float)(dicaX + 8), (float)(dicaY + 7), 8, COR_MARCADOR);
        } else if (!presente) {
            caixaPlaneta(16, 64, 296, 22);
            textoPlaneta("O altar está vazio. ESC volta à nave.", 26, 71, 8, COR_MENU);
        }
    }

    DrawRectangle(0, 346, LARGURA_VIRTUAL, 14, GetColor(0x17131FFF));
    textoPlaneta("WASD/SETAS Mover   SHIFT Correr   E Interagir   ESC Nave", 16, 350, 8, COR_MENU);
}

/* Falantes do planeta: o Ser (rosto dentro do ser.png) e quem desceu
   (rosto do astronauta, primeiro quadro de frente). */
static Falante falanteSer(void)
{
    Falante f = { "O Ser", spriteSer, (Rectangle){ 14, 6, 36, 36 } };
    return f;
}

static Falante falanteVisitante(void)
{
    Falante f = { tripulanteControlado()->nome, explorador.sprite, (Rectangle){ 4, 0, 36, 36 } };
    return f;
}

static void desenharEscolha(void)
{
    const char *textos[TOTAL_DESEJOS];
    for (int i = 0; i < totalOpcoes; i++) {
        textos[i] = desejoDoTipo(opcoes[i])->pedido;
    }
    const char *detalhe = NULL;
    if (totalOpcoes > 0) {
        detalhe = desejoDoTipo(opcoes[escolhida])->detalhe;
    }
    Falante ser = falanteSer();
    desenharEscolhaDialogo(&ser, "Escolha um desejo.", textos, totalOpcoes, escolhida,
                           detalhe, "SETAS  Escolher   ENTER  Pedir   ESC  Desistir");
}

void desenharTelaSer(void)
{
    if (!iniciou) {
        return;
    }
    desenharCenarioPlaneta(&cenario, GetColor(0x34507FFF), GetColor(0x9FD6FFFF));
    desenharParticulas();

    /* Quem está mais para cima na tela é desenhado primeiro (fica atrás). */
    if (explorador.pes.y < Y_SER) {
        desenharExploradorPlaneta(&explorador);
        desenharSer();
    } else {
        desenharSer();
        desenharExploradorPlaneta(&explorador);
    }
    desenharInterface();

    if (estado == SER_AUSENTE) {
        Falante visitante = falanteVisitante();
        desenharFala(&visitante,
                     "O altar está vazio.\nSó o vento atravessa os pilares.\nO Ser não apareceu desta vez.",
                     "E / ENTER  Continuar", false);
    } else if (estado == SER_FALANDO) {
        Falante ser = falanteSer();
        desenharFala(&ser, "Viajante. Faça um pedido.\nSó um. Escolha bem.",
                     "E / ENTER  Continuar   ESC  Sair", true);
    } else if (estado == SER_ESCOLHENDO) {
        desenharEscolha();
    } else if (estado == SER_RESULTADO) {
        Falante ser = falanteSer();
        desenharFala(&ser, resultado, "E / ENTER  Voltar à nave", false);
    }
}

void encerrarTelaSer(void)
{
    if (!iniciou) {
        return;
    }
    encerrarCenarioPlaneta(&cenario);
    encerrarExploradorPlaneta(&explorador);
    if (spriteSer.id != 0) {
        UnloadTexture(spriteSer);
    }
    iniciou = false;
}
