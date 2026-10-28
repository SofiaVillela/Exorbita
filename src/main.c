#include "raylib.h"   /* funções da raylib (janela, desenho, teclado...) */
#include "config.h"   /* nossas constantes: LARGURA_VIRTUAL e ALTURA_VIRTUAL */
#include "recursos.h" /* funções que carregam a fonte */

int main(void)
{
    /* Janela redimensionável, começando com 3 vezes o tamanho do jogo. */
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(LARGURA_VIRTUAL * 3, ALTURA_VIRTUAL * 3, "Exórbita");

    SetExitKey(KEY_NULL); /* Esc não fecha o jogo (no futuro vai fechar painéis). */
    SetTargetFPS(60);     /* 60 quadros por segundo. */

    carregarRecursos();   /* Carrega a fonte com acentos (função do recursos.c). */

    /* Tela do jogo: uma textura de 384x224 onde desenhamos tudo.
       Depois ela é ampliada inteira para a janela. */
    RenderTexture2D telaJogo = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTextureFilter(telaJogo.texture, TEXTURE_FILTER_POINT); /* amplia sem borrar */

    while (!WindowShouldClose()) {

        /* Maior escala inteira que cabe na janela (1x, 2x, 3x...). */
        int escala = GetScreenWidth() / LARGURA_VIRTUAL;
        if (GetScreenHeight() / ALTURA_VIRTUAL < escala) {
            escala = GetScreenHeight() / ALTURA_VIRTUAL;
        }
        if (escala < 1) {
            escala = 1;
        }

        /* Sobra de espaço da janela, dividida para centralizar o jogo. */
        int margemX = (GetScreenWidth() - LARGURA_VIRTUAL * escala) / 2;
        int margemY = (GetScreenHeight() - ALTURA_VIRTUAL * escala) / 2;

        /* 1. Desenha o jogo na textura pequena, em coordenadas de 384x224. */
        BeginTextureMode(telaJogo);
        ClearBackground(GetColor(0x25222BFF));
        DrawTextEx(fontePadrao(), "Exórbita: Os Anos Perdidos", (Vector2){ 16, 16 }, 16, 1, GetColor(0xE8C86AFF));
        EndTextureMode();

        /* 2. Desenha a textura ampliada na janela.
              A altura negativa desvira a imagem (a raylib guarda a textura de cabeça para baixo). */
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle origem = { 0, 0, LARGURA_VIRTUAL, -ALTURA_VIRTUAL };
        Rectangle destino = { margemX, margemY, LARGURA_VIRTUAL * escala, ALTURA_VIRTUAL * escala };
        DrawTexturePro(telaJogo.texture, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
        EndDrawing();
    }

    /* Fechou a janela: limpa o que carregou e encerra. */
    UnloadRenderTexture(telaJogo);
    descarregarRecursos();
    CloseWindow();

    return 0;
} 