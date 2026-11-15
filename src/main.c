#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "telas.h"
#include "tela_inicio.h"
#include "tela_nave.h"
#include "fundo.h"
#include "tela_extraterrestre.h"
#include "tela_gigante.h"
#include "tela_robo.h"
#include "tela_ser.h"
#include "tela_final.h"
#include "tela_intro.h"
#include "partida.h"
#include "mapa_estelar.h"

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(LARGURA_VIRTUAL * ESCALA_INICIAL, ALTURA_VIRTUAL * ESCALA_INICIAL,
               "Exorbita: Os Anos Perdidos - Planetas jogaveis");
    if (!IsWindowReady()) return 1;
    /* Permite abrir pelo gerenciador de arquivos ou a partir de outra pasta. */
    ChangeDirectory(GetApplicationDirectory());
    SetWindowMinSize(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTargetFPS(FPS);
    SetExitKey(KEY_NULL);
    carregarRecursos();
    iniciarFundo();
    iniciarTelaInicio();
    iniciarTelaNave();
    iniciarTelaExtraterrestre();
    iniciarTelaGigante();
    iniciarTelaRobo();
    iniciarTelaSer();
    iniciarTelaFinal();
    iniciarPartida();

    RenderTexture2D telaJogo = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    SetTextureFilter(telaJogo.texture, TEXTURE_FILTER_POINT);

    Tela tela = TELA_INICIO;
    bool rodando = true;
    while (rodando && !WindowShouldClose()) {
        Tela anterior = tela;
        /* 1. Lógica: cada tela devolve para onde o jogo deve ir. */
        if (tela == TELA_INICIO) {
            tela = atualizarTelaInicio();
        } else if (tela == TELA_INTRO) {
            tela = atualizarTelaIntro();
        } else if (tela == TELA_NAVE) {
            tela = atualizarTelaNave();
        } else if (tela == TELA_EXTRATERRESTRE) {
            tela = atualizarTelaExtraterrestre();
        } else if (tela == TELA_GIGANTE) {
            tela = atualizarTelaGigante();
        } else if (tela == TELA_ROBO) {
            tela = atualizarTelaRobo();
        } else if (tela == TELA_SER) {
            tela = atualizarTelaSer();
        } else if (tela == TELA_FINAL) {
            tela = atualizarTelaFinal();
        } else {
            if (IsKeyPressed(KEY_ESCAPE)) {
                tela = TELA_INICIO;
            }
        }
        if (tela == TELA_SAIR) {
            rodando = false;
        }
        if (tela != anterior) {
            if (tela == TELA_EXTRATERRESTRE) entrarTelaExtraterrestre();
            if (tela == TELA_GIGANTE) entrarTelaGigante();
            if (tela == TELA_ROBO) entrarTelaRobo();
            if (tela == TELA_SER) entrarTelaSer();
            if (tela == TELA_FINAL) entrarTelaFinal();
            if (tela == TELA_INTRO) {
                /* Jogar no menu começa uma partida nova do zero. */
                iniciarPartida();
                reiniciarMapaEstelar();
                reiniciarTelaNave();
                reiniciarTelaRobo();
                entrarTelaIntro();
            }
            /* Voltou de um planeta para a nave (não do menu): confere
               quem está vivo e quem passa a ser controlado. */
            if (tela == TELA_NAVE && anterior != TELA_INICIO && anterior != TELA_INTRO) entrarTelaNave();
        }

        /* 2. Desenho na tela pequena. */
        BeginTextureMode(telaJogo);
        ClearBackground(COR_FUNDO);

        Font fonte = fontePadrao();

        if (tela == TELA_INICIO) {
            desenharTelaInicio();
        } else if (tela == TELA_INTRO) {
            desenharTelaIntro();
        } else if (tela == TELA_NAVE) {
            desenharTelaNave();
        } else if (tela == TELA_EXTRATERRESTRE) {
            desenharTelaExtraterrestre();
        } else if (tela == TELA_GIGANTE) {
            desenharTelaGigante();
        } else if (tela == TELA_ROBO) {
            desenharTelaRobo();
        } else if (tela == TELA_SER) {
            desenharTelaSer();
        } else if (tela == TELA_FINAL) {
            desenharTelaFinal();
        } else if (tela == TELA_REGRAS) {
            DrawTextEx(fonte, "Regras (em construção)", (Vector2){ MARGEM_ESQUERDA, 32 }, 16, 1, COR_TITULO);
        } else if (tela == TELA_CREDITOS) {
            DrawTextEx(fonte, "Créditos (em construção)", (Vector2){ MARGEM_ESQUERDA, 32 }, 16, 1, COR_TITULO);
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
    encerrarTelaInicio();
    encerrarTelaNave();
    encerrarTelaExtraterrestre();
    encerrarTelaGigante();
    encerrarTelaRobo();
    encerrarTelaSer();
    encerrarTelaFinal();
    encerrarFundo();
    descarregarRecursos();
    CloseWindow();
    return 0;
}
