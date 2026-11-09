#include "raylib.h"
#include "constantes.h"
#include "recursos.h"

int main(void)
{
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, "Exorbita: Os Anos Perdidos");
    SetTargetFPS(FPS);
    carregarRecursos();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){ 10, 10, 25, 255 });

        Font fonte = fontePadrao();
        DrawTextEx(fonte, "Exórbita", (Vector2){ MARGEM_ESQUERDA, Y_TITULO },
        TAMANHO_TITULO, ESPACAMENTO_TITULO, COR_TITULO);
        DrawTextEx(fonte, "Os Anos Perdidos", (Vector2){ MARGEM_ESQUERDA, Y_SUBTITULO },
        TAMANHO_SUBTITULO, ESPACAMENTO_SUBTITULO, COR_SUBTITULO);
        

        EndDrawing();
    }

    descarregarRecursos();
    CloseWindow();
    return 0;
}