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

    const char *opcoes[] = { "Jogar", "Regras", "Créditos", "Sair" };
    int totalOpcoes = 4;
    int selecionada = 0;   /* índice da opção em destaque */
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_DOWN)) {
            selecionada++;
            if (selecionada >= totalOpcoes) {
                selecionada = 0;              /* passou da última: volta para a primeira */
            }
        }
        if (IsKeyPressed(KEY_UP)) {
            selecionada--;
            if (selecionada < 0) {
                selecionada = totalOpcoes - 1; /* antes da primeira: vai para a última */
            }
        }
        BeginTextureMode(telaJogo);
        ClearBackground(COR_FUNDO);

        Font fonte = fontePadrao();
        DrawTextEx(fonte, "Exórbita", (Vector2){ MARGEM_ESQUERDA, Y_TITULO },
        TAMANHO_TITULO, ESPACAMENTO_TITULO, COR_TITULO);
        DrawTextEx(fonte, "Os Anos Perdidos", (Vector2){ MARGEM_ESQUERDA, Y_SUBTITULO },
        TAMANHO_SUBTITULO, ESPACAMENTO_SUBTITULO, COR_SUBTITULO);
        
        for (int i = 0; i < totalOpcoes; i++) {
            int y = Y_MENU + i * ESPACO_MENU;
            Color cor = COR_MENU;

            if (i == selecionada) {
                cor = COR_SELECIONADO;
                DrawRectangle(MARGEM_ESQUERDA, y + 2, TAMANHO_MARCADOR, TAMANHO_MARCADOR, COR_MARCADOR);
            }

            DrawTextEx(fonte, opcoes[i], (Vector2){ MARGEM_ESQUERDA + RECUO_MENU, y },
                    TAMANHO_MENU, 0, cor);
        }
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