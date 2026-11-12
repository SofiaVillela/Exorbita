#include "mapa_estelar.h"

#include "raylib.h"
#include "constantes.h"
#include "recursos.h"

enum { TOTAL_DESTINOS = 5, COLUNAS_FOLHA = 12, LINHAS_FOLHA = 8 };

typedef struct {
    const char *nome;
    const char *arquivo;
    const char *descricao;
    Vector2 centro;
} DestinoEstelar;

static const DestinoEstelar destinos[TOTAL_DESTINOS] = {
    { "SER", "assets/imagens/planeta_ser.png",
      "Atmosfera violeta.\nRelevo acidentado.", { 82, 133 } },
    { "ROBÔ", "assets/imagens/planeta_robo.png",
      "Superfície irregular.\nReflexos azulados.", { 207, 122 } },
    { "EXTRATERRESTRE", "assets/imagens/planeta_extraterrestre.png",
      "Anéis luminosos.\nTons violetas e ciano.", { 320, 167 } },
    { "GIGANTE", "assets/imagens/planeta_gigante.png",
      "Picos e crateras.\nGrandes formações.", { 264, 259 } },
    { "SEM AR", "assets/imagens/planeta_sem_ar.png",
      "Solo rochoso.\nCrateras visíveis.", { 108, 260 } }
};

static Texture2D planetas[TOTAL_DESTINOS];
static bool iniciado;
static bool aberto;
static int selecionado;
static int quadro;
static float tempoQuadro;

static Color ciano(void)
{
    return GetColor(0x68D8CDFF);
}

static Color borda(void)
{
    return GetColor(0x665783FF);
}

static void texto(const char *mensagem, float x, float y, int tamanho, Color cor)
{
    DrawTextEx(fontePadrao(), mensagem, (Vector2){ x, y }, (float)tamanho, 0, cor);
}

static void painel(int x, int y, int largura, int altura)
{
    DrawRectangle(x, y, largura, altura, GetColor(0x1D192AFF));
    DrawRectangleLines(x, y, largura, altura, borda());
    DrawRectangle(x + 1, y + 1, largura - 2, 2, GetColor(0x342C48FF));
}

static void desenharPlaneta(int indice, Vector2 centro, int tamanho)
{
    Texture2D folha = planetas[indice];
    if (folha.id == 0) {
        /* A consulta permanece utilizável mesmo se uma imagem estiver ausente. */
        DrawCircle((int)centro.x, (int)centro.y, (float)tamanho / 3, borda());
        DrawCircleLines((int)centro.x, (int)centro.y, (float)tamanho / 3, COR_SUBTITULO);
        return;
    }

    int atual = (quadro + indice * 11) % QUADROS_PLANETA;
    Rectangle origem = {
        (float)(atual % COLUNAS_FOLHA * TAMANHO_PLANETA),
        (float)(atual / COLUNAS_FOLHA * TAMANHO_PLANETA),
        TAMANHO_PLANETA, TAMANHO_PLANETA
    };
    Rectangle destino = {
        centro.x - tamanho / 2, centro.y - tamanho / 2,
        (float)tamanho, (float)tamanho
    };
    DrawTexturePro(folha, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

static void desenharSelecao(Vector2 centro)
{
    int x = (int)centro.x - 31;
    int y = (int)centro.y - 31;
    Color cor = COR_MARCADOR;
    for (int i = 0; i < 2; i++) {
        int cx = x + i * 62;
        DrawRectangle(cx - i * 8, y, 9, 2, cor);
        DrawRectangle(cx, y, 2, 8, cor);
        DrawRectangle(cx - i * 8, y + 62, 9, 2, cor);
        DrawRectangle(cx, y + 56, 2, 8, cor);
    }
}

void iniciarMapaEstelar(void)
{
    if (iniciado) return;

    for (int i = 0; i < TOTAL_DESTINOS; i++) {
        if (FileExists(destinos[i].arquivo)) {
            planetas[i] = LoadTexture(destinos[i].arquivo);
        }
        if (planetas[i].id == 0 ||
            planetas[i].width < COLUNAS_FOLHA * TAMANHO_PLANETA ||
            planetas[i].height < LINHAS_FOLHA * TAMANHO_PLANETA) {
            TraceLog(LOG_WARNING, "MAPA: imagem de planeta indisponivel: %s", destinos[i].arquivo);
            if (planetas[i].id != 0) UnloadTexture(planetas[i]);
            planetas[i] = (Texture2D){ 0 };
        } else {
            SetTextureFilter(planetas[i], TEXTURE_FILTER_POINT);
        }
    }
    iniciado = true;
}

void abrirMapaEstelar(void)
{
    aberto = true;
    tempoQuadro = 0;
}

bool atualizarMapaEstelar(void)
{
    if (!aberto) return false;
    if (IsKeyPressed(KEY_ESCAPE)) {
        aberto = false;
        return true;
    }

    bool anterior = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP) ||
                    IsKeyPressed(KEY_A) || IsKeyPressed(KEY_W);
    bool proximo = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN) ||
                   IsKeyPressed(KEY_D) || IsKeyPressed(KEY_S);
    if (anterior != proximo) {
        selecionado = (selecionado + (proximo ? 1 : TOTAL_DESTINOS - 1)) % TOTAL_DESTINOS;
    }

    float dt = GetFrameTime();
    if (dt > 0 && dt < 1) {
        tempoQuadro += dt;
        while (tempoQuadro >= TEMPO_QUADRO) {
            tempoQuadro -= TEMPO_QUADRO;
            quadro = (quadro + 1) % QUADROS_PLANETA;
        }
    }
    return false;
}

void desenharMapaEstelar(void)
{
    if (!aberto) return;

    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, GetColor(0x13101DFF));
    DrawRectangle(24, 18, 3, 16, COR_MARCADOR);
    texto("MAPA ESTELAR", 36, 18, 16, COR_TITULO);
    texto("CONSOLE DE NAVEGAÇÃO", 464, 22, 8, ciano());
    texto("Selecione um destino para consultar.", 24, 45, 8, COR_MENU);

    painel(24, 68, 360, 244);
    painel(400, 68, 216, 244);

    /* Estrelas fixas: o mapa não troca de aparência ao abrir novamente. */
    for (int i = 0; i < 54; i++) {
        int x = 34 + (i * 73 + 19) % 340;
        int y = 80 + (i * 47 + 11) % 220;
        DrawPixel(x, y, i % 5 == 0 ? COR_MENU : GetColor(0x49405FFF));
    }

    for (int i = 0; i < TOTAL_DESTINOS; i++) {
        Vector2 a = destinos[i].centro;
        Vector2 b = destinos[(i + 1) % TOTAL_DESTINOS].centro;
        DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, GetColor(0x3C4258FF));
    }
    for (int i = 0; i < TOTAL_DESTINOS; i++) {
        Vector2 centro = destinos[i].centro;
        DrawCircleLines((int)centro.x, (int)centro.y, 29, GetColor(0x41374FFF));
        desenharPlaneta(i, centro, 48);
        float largura = MeasureTextEx(fontePadrao(), destinos[i].nome, 8, 0).x;
        texto(destinos[i].nome, centro.x - largura / 2, centro.y + 36, 8,
              i == selecionado ? COR_MARCADOR : COR_MENU);
    }
    desenharSelecao(destinos[selecionado].centro);

    texto(TextFormat("DESTINO %02d / %02d", selecionado + 1, TOTAL_DESTINOS), 416, 84, 8, ciano());
    const char *nome = destinos[selecionado].nome;
    int tamanhoNome = MeasureTextEx(fontePadrao(), nome, 16, 0).x <= 184 ? 16 : 8;
    texto(nome, 416, 104, tamanhoNome, COR_TITULO);
    desenharPlaneta(selecionado, (Vector2){ 508, 183 }, 96);
    DrawLine(416, 242, 600, 242, borda());
    texto(destinos[selecionado].descricao, 416, 252, 8, COR_MENU);
    DrawRectangle(416, 292, 4, 8, COR_MARCADOR);
    texto("Pouso indisponível", 428, 292, 8, COR_MARCADOR);

    texto("SETAS / WASD  Selecionar", 24, 332, 8, COR_MENU);
    texto("ESC  Voltar à nave", 480, 332, 8, COR_TITULO);
}

void encerrarMapaEstelar(void)
{
    for (int i = 0; i < TOTAL_DESTINOS; i++) {
        if (planetas[i].id != 0) UnloadTexture(planetas[i]);
        planetas[i] = (Texture2D){ 0 };
    }
    iniciado = false;
    aberto = false;
}
