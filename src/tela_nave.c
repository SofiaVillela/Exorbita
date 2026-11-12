#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "tela_nave.h"

static Texture2D sala;    /* desenho da sala do painel (640x360, janela transparente) */
static Image mascara;     /* paredes e áreas da sala; fica na RAM, não é desenhada */
static Texture2D tripulantes[TOTAL_TRIPULANTES];
static const char *arquivosTripulantes[TOTAL_TRIPULANTES] = {
    "assets/imagens/tripulante_castanho.png",
    "assets/imagens/tripulante_rabo_de_cavalo.png",
    "assets/imagens/tripulante_cacheado.png",
    "assets/imagens/tripulante_ruivo.png",
    "assets/imagens/tripulante_loiro.png",
    "assets/imagens/tripulante_preto.png"
};
/* posição dos pés de cada um, em pixels da tela de 640x360 */
static const Vector2 pesTripulantes[TOTAL_TRIPULANTES] = {
    { 120, 260 }, { 200, 300 }, { 270, 250 },
    { 380, 300 }, { 450, 255 }, { 540, 290 }
};

void iniciarTelaNave(void)
{
    sala = LoadTexture("assets/imagens/sala_painel_fundo.png");
    mascara = LoadImage("assets/mascaras/sala_painel_mascara.png");
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        tripulantes[i] = LoadTexture(arquivosTripulantes[i]);
    }
}

Tela atualizarTelaNave(void)
{
    atualizarFundo();

    if (IsKeyPressed(KEY_ESCAPE)) {
        return TELA_INICIO;
    }
    return TELA_NAVE;
}

void desenharTelaNave(void)
{
    desenharFundo();
    DrawTexture(sala, 0, 0, WHITE);
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        /* quadro da coluna 0 (parado), linha 0 (de frente) */
        Rectangle quadro = { 0, 0, LARGURA_PERSONAGEM, ALTURA_PERSONAGEM };
        /* a posição guardada é a dos pés: sobe a altura e volta meia largura */
        Vector2 canto = { pesTripulantes[i].x - LARGURA_PERSONAGEM / 2,
                          pesTripulantes[i].y - ALTURA_PERSONAGEM };
        DrawTextureRec(tripulantes[i], quadro, canto, WHITE);
    }
}

void encerrarTelaNave(void)
{
    UnloadTexture(sala);
    UnloadImage(mascara);
    for (int i = 0; i < TOTAL_TRIPULANTES; i++) {
        UnloadTexture(tripulantes[i]);
    }
}