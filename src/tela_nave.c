#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "tela_nave.h"

static Texture2D sala;   /* desenho da sala de controle (384x224) */

void iniciarTelaNave(void)
{
    sala = LoadTexture("assets/imagens/sala_controle_fundo.png");
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
}

void encerrarTelaNave(void)
{
    UnloadTexture(sala);
}