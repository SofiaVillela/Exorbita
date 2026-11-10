#include "raylib.h"
#include "constantes.h"
#include "recursos.h"

typedef enum {
    TELA_INICIO,
    TELA_NAVE,
    TELA_REGRAS,
    TELA_CREDITOS
} Tela;

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(LARGURA_VIRTUAL * ESCALA_INICIAL, ALTURA_VIRTUAL * ESCALA_INICIAL, "Exorbita: Os Anos Perdidos");
    SetTargetFPS(FPS);
    SetExitKey(KEY_NULL);
    carregarRecursos();

    RenderTexture2D telaJogo = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTextureFilter(telaJogo.texture, TEXTURE_FILTER_POINT);

    const char *opcoes[] = { "Jogar", "Regras", "Créditos", "Sair" };
    int totalOpcoes = 4;
    int selecionada = 0;   /* índice da opção em destaque */
    Tela tela = TELA_INICIO;
    bool rodando = true;
    while (rodando && !WindowShouldClose()) {
        if (tela == TELA_INICIO) {
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
            if (IsKeyPressed(KEY_ENTER)) {
                if (selecionada == 0) {
                    tela = TELA_NAVE;
                } else if (selecionada == 1) {
                    tela = TELA_REGRAS;
                } else if (selecionada == 2) {
                    tela = TELA_CREDITOS;
                } else if (selecionada == 3) {
                    rodando = false;
                }
            }
        } else {
            if (IsKeyPressed(KEY_ESCAPE)) {
                tela = TELA_INICIO;
            }
        }
        BeginTextureMode(telaJogo);
        ClearBackground(COR_FUNDO);

        Font fonte = fontePadrao();

        if (tela == TELA_INICIO) {
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