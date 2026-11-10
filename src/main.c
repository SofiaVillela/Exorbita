#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "telas.h"
#include "tela_inicio.h"
#include "estrelas.h"

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(LARGURA_VIRTUAL * ESCALA_INICIAL, ALTURA_VIRTUAL * ESCALA_INICIAL, "Exorbita: Os Anos Perdidos");
    SetTargetFPS(FPS);
    SetExitKey(KEY_NULL);
    carregarRecursos();
    iniciarEstrelas();

    RenderTexture2D telaJogo = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTextureFilter(telaJogo.texture, TEXTURE_FILTER_POINT);

    Tela tela = TELA_INICIO;
    bool rodando = true;
    while (rodando && !WindowShouldClose()) {
        /* 1. Lógica: cada tela devolve para onde o jogo deve ir. */
        if (tela == TELA_INICIO) {
            tela = atualizarTelaInicio();
        } else {
            if (IsKeyPressed(KEY_ESCAPE)) {
                tela = TELA_INICIO;
            }
        }
        if (tela == TELA_SAIR) {
            rodando = false;
        }

        /* 2. Desenho na tela pequena. */
        BeginTextureMode(telaJogo);
        ClearBackground(COR_FUNDO);

        Font fonte = fontePadrao();

        if (tela == TELA_INICIO) {
            desenharTelaInicio();
        } else if (tela == TELA_NAVE) {
            DrawTextEx(fonte, "Nave (em construção)", (Vector2){ MARGEM_ESQUERDA, 16 }, 8, 0, COR_TITULO);
        } else if (tela == TELA_REGRAS) {
            DrawTextEx(fonte, "Regras (em construção)", (Vector2){ MARGEM_ESQUERDA, 16 }, 8, 0, COR_TITULO);
        } else if (tela == TELA_CREDITOS) {
            DrawTextEx(fonte, "Créditos (em construção)", (Vector2){ MARGEM_ESQUERDA, 16 }, 8, 0, COR_TITULO);
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