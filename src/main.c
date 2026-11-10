#include "raylib.h"
#include "constantes.h"
#include "recursos.h"

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(LARGURA_VIRTUAL * ESCALA_INICIAL, ALTURA_VIRTUAL * ESCALA_INICIAL, "Exorbita: Os Anos Perdidos");
    SetTargetFPS(FPS);
    carregarRecursos();

    RenderTexture2D telaJogo = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTextureFilter(telaJogo.texture, TEXTURE_FILTER_POINT);

    while (!WindowShouldClose()) {
        BeginTextureMode(telaJogo);
        ClearBackground(COR_FUNDO);

        Font fonte = fontePadrao();
        DrawTextEx(fonte, "Exórbita", (Vector2){ MARGEM_ESQUERDA, Y_TITULO },
        TAMANHO_TITULO, ESPACAMENTO_TITULO, COR_TITULO);
        DrawTextEx(fonte, "Os Anos Perdidos", (Vector2){ MARGEM_ESQUERDA, Y_SUBTITULO },
        TAMANHO_SUBTITULO, ESPACAMENTO_SUBTITULO, COR_SUBTITULO);
        

        EndTextureMode();

        int escala = GetScreenWidth() / LARGURA_VIRTUAL;
        if (GetScreenHeight() / ALTURA_VIRTUAL < escala) {
            escala = GetScreenHeight() / ALTURA_VIRTUAL;
        }
        if (escala < 1) {
            escala = 1;
        }

        /* 3. O que sobra da janela é dividido por 2 para centralizar. */
        int margemX = (GetScreenWidth() - LARGURA_VIRTUAL * escala) / 2;
        int margemY = (GetScreenHeight() - ALTURA_VIRTUAL * escala) / 2;

        /* 4. Desenha a tela pequena ampliada na janela. */
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle origem = { 0, 0, LARGURA_VIRTUAL, -ALTURA_VIRTUAL };
        Rectangle destino = { margemX, margemY, LARGURA_VIRTUAL * escala, ALTURA_VIRTUAL * escala };
        DrawTexturePro(telaJogo.texture, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
        EndDrawing();
    
    }
    UnloadRenderTexture(telaJogo);
    descarregarRecursos();
    CloseWindow();
    return 0;
}