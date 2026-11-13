#include "tela_extraterrestre.h"

#include "raylib.h"
#include "cartas.h"
#include "cenario_planeta.h"
#include "constantes.h"
#include "partida.h"
#include "recursos.h"
#include "tradutor.h"

#include <math.h>
#include <stdio.h>

typedef enum { ET_CIDADE, ET_CONVERSA, ET_REGRAS, ET_CARTAS } ModoExtraterrestre;
typedef enum { ET_SEM_ALVO, ET_VISITANTE, ET_MESA, ET_NAVE } AlvoExtraterrestre;

static CenarioPlaneta cidade;
static ExploradorPlaneta explorador;
static ModoExtraterrestre modo;
static AlvoExtraterrestre alvo;
static PartidaCartas cartas;
static FalaTraduzida fala;
static int paginaConversa, cartaSelecionada, acaoSelecionada;
static float tempoCidade;
static bool iniciou;
static const PontoExploracao pesVisitante = { 430, 254 };
static const Rectangle mesa = { 492, 270, 88, 20 };
static const Rectangle acessoNave = { 34, 286, 68, 48 };
static const ObstaculoExploracao obstaculos[] = {
    { 174, 222, 38, 25 }, { 292, 293, 42, 24 },
    { 416, 246, 28, 8 }, { 492, 270, 88, 20 }
};
static const char *saudacoes[] = {
    "Bem-vindo à nossa cidade.\nAceita uma partida de cartas?",
    "Na mesa, escolha uma carta.\nVocê pode comparar ou desconfiar."
};

static Color ciano(void) { return GetColor(0x68D8CDFF); }
static Color violeta(void) { return GetColor(0x554571FF); }

static void traduzirFala(const char *mensagem, bool blefando)
{
    ProgressoPartida *progresso = progressoPartida();
    FalaAlien original = { mensagem, blefando };
    float sorteio = (float)GetRandomValue(0, 9999) / 10000;
    traduzir(&original, progresso->temTradutor, tripulanteControlado()->tracos.inteligencia, sorteio, &fala);
}

static void anunciarCarta(void)
{
    char mensagem[64];
    snprintf(mensagem, sizeof(mensagem), "Minha carta vale %d.", cartas.anuncioAlien);
    traduzirFala(mensagem, cartas.anuncioAlien != cartas.maoAlien[cartas.indiceAlien]);
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        if (!cartas.usadasJogador[i]) { cartaSelecionada = i; break; }
    }
    acaoSelecionada = 0;
}

static void novaPartida(void)
{
    iniciarCartas(&cartas, (uint32_t)GetRandomValue(1, 1000000));
    anunciarCarta();
    modo = ET_CARTAS;
}

static float distanciaRetangulo(Rectangle r)
{
    float x = fmaxf(r.x, fminf(explorador.pes.x, r.x + r.width));
    float y = fmaxf(r.y, fminf(explorador.pes.y, r.y + r.height));
    return hypotf(explorador.pes.x - x, explorador.pes.y - y);
}

static AlvoExtraterrestre encontrarAlvo(void)
{
    float menor = 43;
    AlvoExtraterrestre encontrado = ET_SEM_ALVO;
    float d = hypotf(explorador.pes.x - pesVisitante.x, explorador.pes.y - pesVisitante.y);
    if (d < menor) { menor = d; encontrado = ET_VISITANTE; }
    d = distanciaRetangulo(mesa);
    if (d < menor) { menor = d; encontrado = ET_MESA; }
    d = distanciaRetangulo(acessoNave);
    if (d < menor && d < 18) encontrado = ET_NAVE;
    return encontrado;
}

void iniciarTelaExtraterrestre(void)
{
    if (iniciou) return;
    iniciarCenarioPlaneta(&cidade, "assets/imagens/planetas/cidade_cyberpunk.png");
    iniciarExploradorPlaneta(&explorador, (PontoExploracao){ 78, 316 });
    iniciou = true;
}

void entrarTelaExtraterrestre(void)
{
    explorador.pes = (PontoExploracao){ 78, 316 };
    explorador.direcao = 3;
    explorador.quadro = 0;
    explorador.tempo = 0;
    explorador.correndo = false;
    modo = ET_CIDADE;
    alvo = encontrarAlvo();
}

static void atualizarEscolha(void)
{
    int movimento = (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) -
                    (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT));
    if (movimento) {
        for (int n = 0; n < CARTAS_NA_MAO; n++) {
            cartaSelecionada = (cartaSelecionada + movimento + CARTAS_NA_MAO) % CARTAS_NA_MAO;
            if (!cartas.usadasJogador[cartaSelecionada]) break;
        }
    }
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        if (IsKeyPressed(KEY_ONE + i) && !cartas.usadasJogador[i]) cartaSelecionada = i;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
        IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S)) acaoSelecionada = 1 - acaoSelecionada;
    if (IsKeyPressed(KEY_ENTER) && jogarCartas(&cartas, cartaSelecionada, acaoSelecionada == 1)) {
        if (consumirVitoriaCartas(&cartas)) {
            ProgressoPartida *progresso = progressoPartida();
            progresso->vitoriasCartas++;
            progresso->extraterrestreConcluido = true;
        }
    }
}

Tela atualizarTelaExtraterrestre(void)
{
    float dt = GetFrameTime();
    if (dt > 0.1f) dt = 0.1f;
    tempoCidade += dt;
    if (modo == ET_CONVERSA) {
        if (IsKeyPressed(KEY_ESCAPE)) modo = ET_CIDADE;
        else if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER)) {
            if (++paginaConversa >= 2) modo = ET_CIDADE;
            else traduzirFala(saudacoes[paginaConversa], false);
        }
        return TELA_EXTRATERRESTRE;
    }
    if (modo == ET_REGRAS) {
        if (IsKeyPressed(KEY_ESCAPE)) modo = ET_CIDADE;
        else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) novaPartida();
        return TELA_EXTRATERRESTRE;
    }
    if (modo == ET_CARTAS) {
        if (IsKeyPressed(KEY_ESCAPE)) modo = ET_CIDADE;
        else if (cartas.fase == CARTAS_ESCOLHENDO) atualizarEscolha();
        else if (IsKeyPressed(KEY_ENTER)) {
            if (cartas.fase == CARTAS_ENCERRADAS) novaPartida();
            else if (avancarRodadaCartas(&cartas)) anunciarCarta();
        }
        return TELA_EXTRATERRESTRE;
    }
    if (IsKeyPressed(KEY_ESCAPE)) return TELA_NAVE;
    atualizarExploradorPlaneta(&explorador, &cidade.piso, obstaculos,
        (int)(sizeof(obstaculos) / sizeof(obstaculos[0])), dt, true);
    alvo = encontrarAlvo();
    if (IsKeyPressed(KEY_E)) {
        if (alvo == ET_NAVE) return TELA_NAVE;
        if (alvo == ET_VISITANTE) {
            paginaConversa = 0;
            traduzirFala(saudacoes[0], false);
            modo = ET_CONVERSA;
        } else if (alvo == ET_MESA) modo = ET_REGRAS;
    }
    return TELA_EXTRATERRESTRE;
}

/* Visitante e mesa são arte de jogo nativa, com pixels inteiros. */
static void desenharVisitante(int x, int y)
{
    Color contorno = GetColor(0x191522FF), pele = GetColor(0x67CAB6FF);
    Color roupa = GetColor(0x79639FFF);
    DrawEllipse(x, y - 2, 16, 4, Fade(BLACK, 0.35f));
    DrawRectangle(x - 13, y - 47, 26, 24, contorno);
    DrawRectangle(x - 16, y - 42, 32, 14, contorno);
    DrawRectangle(x - 11, y - 45, 22, 20, pele);
    DrawRectangle(x - 14, y - 40, 28, 10, pele);
    DrawRectangle(x - 8, y - 39, 6, 8, contorno);
    DrawRectangle(x + 3, y - 39, 6, 8, contorno);
    if (((int)(tempoCidade * 2) % 7) == 0) {
        DrawRectangle(x - 8, y - 39, 6, 5, pele);
        DrawRectangle(x + 3, y - 39, 6, 5, pele);
    }
    DrawRectangle(x - 11, y - 25, 22, 23, contorno);
    DrawRectangle(x - 9, y - 23, 18, 19, roupa);
    DrawRectangle(x - 2, y - 22, 4, 17, COR_MARCADOR);
    DrawRectangle(x - 15, y - 22, 5, 15, pele);
    DrawRectangle(x + 11, y - 22, 5, 15, pele);
    DrawRectangle(x - 10, y - 4, 8, 4, contorno);
    DrawRectangle(x + 3, y - 4, 8, 4, contorno);
}

static void desenharMesa(void)
{
    DrawRectangle(492, 260, 88, 30, GetColor(0x181520FF));
    DrawRectangle(494, 253, 84, 28, violeta());
    DrawRectangleLines(494, 253, 84, 28, COR_SUBTITULO);
    DrawRectangle(498, 257, 76, 2, ciano());
    for (int i = 0; i < 3; i++) {
        DrawRectangle(508 + 18 * i, 262, 12, 16, COR_TITULO);
        DrawRectangle(510 + 18 * i, 264, 8, 12, GetColor(0x392C54FF));
        DrawRectangle(513 + 18 * i, 268, 2, 4, COR_MARCADOR);
    }
}

static void desenharCidade(void)
{
    desenharCenarioPlaneta(&cidade, GetColor(0x2B2738FF), ciano());
    DrawRectangleLines((int)acessoNave.x, (int)acessoNave.y,
                       (int)acessoNave.width, (int)acessoNave.height, ciano());
    textoPlaneta("NAVE", 52, 307, 8, ciano());
    desenharObstaculoPlaneta(obstaculos[0], violeta());
    desenharObstaculoPlaneta(obstaculos[1], GetColor(0x46766DFF));
    if (explorador.pes.y < pesVisitante.y) desenharExploradorPlaneta(&explorador);
    desenharVisitante((int)pesVisitante.x, (int)pesVisitante.y);
    if (explorador.pes.y >= pesVisitante.y && explorador.pes.y < mesa.y + mesa.height)
        desenharExploradorPlaneta(&explorador);
    desenharMesa();
    if (explorador.pes.y >= mesa.y + mesa.height) desenharExploradorPlaneta(&explorador);
    caixaPlaneta(16, 12, 352, 46);
    textoPlaneta("CIDADE CYBERPUNK", 28, 20, 16, COR_TITULO);
    textoPlaneta(progressoPartida()->extraterrestreConcluido ?
        "Encontro concluído" : "Converse e visite a mesa de cartas", 28, 44, 8, COR_SUBTITULO);
    caixaPlaneta(408, 12, 216, 46);
    textoPlaneta("EXTRATERRESTRE", 420, 22, 8, ciano());
    textoPlaneta(progressoPartida()->temTradutor ? "Tradutor: equipado" : "Tradutor: ausente",
                  420, 40, 8, COR_MENU);
    if (modo == ET_CIDADE && alvo != ET_SEM_ALVO) {
        const char *mensagem = alvo == ET_VISITANTE ? "E  Conversar" :
            alvo == ET_MESA ? "E  Jogar cartas" : "E  Voltar à nave";
        int largura = (int)MeasureTextEx(fontePadrao(), mensagem, 8, 0).x + 20;
        caixaPlaneta(320 - largura / 2, 166, largura, 22);
        textoPlaneta(mensagem, (float)(330 - largura / 2), 173, 8, COR_MARCADOR);
    }
    textoPlaneta("WASD  Mover   SHIFT  Correr   E  Interagir   ESC  Nave", 24, 350, 8, COR_MENU);
}

static void desenharConversa(void)
{
    DrawRectangle(0, 0, 640, 360, Fade(BLACK, 0.3f));
    caixaPlaneta(16, 216, 608, 124);
    desenharVisitante(54, 290);
    textoPlaneta("VISITANTE", 98, 230, 8, ciano());
    textoPlaneta(fala.texto, 98, 250, 8, COR_TITULO);
    textoPlaneta(progressoPartida()->temTradutor ?
        "Tradução ativa" : "Sem tradutor: a fala é desconhecida", 98, 294, 8, COR_SUBTITULO);
    textoPlaneta(TextFormat("%d / 2", paginaConversa + 1), 98, 320, 8, COR_MENU);
    textoPlaneta("E / ENTER  Continuar   ESC  Fechar", 340, 320, 8, COR_MENU);
}

static void desenharRegras(void)
{
    DrawRectangle(0, 0, 640, 360, Fade(BLACK, 0.7f));
    caixaPlaneta(24, 26, 592, 308);
    textoPlaneta("BLEFE ORBITAL", 44, 46, 16, COR_TITULO);
    static const char *linhas[] = {
        "Baralho de 1 a 9, sem repetição.",
        "Você e o visitante recebem 3 cartas.",
        "Ele escolhe uma carta oculta e anuncia um valor.",
        "Escolha sua carta e uma ação:",
        "COMPARAR: vence a maior carta real.",
        "DESCONFIAR: você vence se ele mentiu.",
        "Cada rodada consome uma carta de cada mão.",
        "Vence quem somar 2 pontos; até 3 rodadas.",
        "Sem tradutor, a fala vira símbolos; números ficam.",
        "Com tradutor, a leitura do tom ainda pode errar."
    };
    for (int i = 0; i < 10; i++) textoPlaneta(linhas[i], 44, (float)(82 + i * 19), 8, COR_MENU);
    textoPlaneta("ENTER  Iniciar partida", 44, 306, 8, COR_MARCADOR);
    textoPlaneta("ESC  Voltar à cidade", 432, 306, 8, COR_MENU);
}

static void desenharCarta(int indice)
{
    int x = 50 + indice * 102;
    bool usada = cartas.usadasJogador[indice];
    DrawRectangle(x, 183, 76, 103, usada ? GetColor(0x342C45FF) : COR_TITULO);
    DrawRectangle(x + 3, 186, 70, 97, usada ? GetColor(0x241F30FF) : GetColor(0x3F3157FF));
    if (indice == cartaSelecionada && !usada && cartas.fase == CARTAS_ESCOLHENDO)
        DrawRectangleLinesEx((Rectangle){ (float)x - 3, 180, 82, 109 }, 2, COR_MARCADOR);
    textoPlaneta(TextFormat("%d", indice + 1), (float)x + 7, 192, 8, COR_SUBTITULO);
    textoPlaneta(usada ? "-" : TextFormat("%d", cartas.maoJogador[indice]),
                  (float)x + 22, 216, 32, usada ? COR_SUBTITULO : COR_MARCADOR);
    if (usada) textoPlaneta("USADA", (float)x + 18, 266, 8, COR_SUBTITULO);
}

static void desenharResultado(void)
{
    DrawRectangle(0, 154, 640, 160, Fade(BLACK, 0.7f));
    caixaPlaneta(56, 164, 528, 146);
    const char *titulo = cartas.venceuRodada ? "RODADA VENCIDA" : "RODADA PERDIDA";
    if (cartas.fase == CARTAS_ENCERRADAS)
        titulo = cartas.pontosJogador >= 2 ? "VOCÊ VENCEU" : "O VISITANTE VENCEU";
    textoPlaneta(titulo, 76, 180, 16, COR_MARCADOR);
    textoPlaneta(TextFormat("Sua carta: %d    Carta real dele: %d",
        cartas.cartaJogada, cartas.maoAlien[cartas.indiceAlien]), 76, 211, 8, COR_TITULO);
    textoPlaneta(TextFormat("Anúncio: %d. %s", cartas.anuncioAlien,
        cartas.anuncioAlien != cartas.maoAlien[cartas.indiceAlien] ? "Era blefe." : "Era verdade."),
        76, 229, 8, COR_MENU);
    textoPlaneta(cartas.desconfiou ? "Você decidiu desconfiar." : "Você decidiu comparar os valores.",
                  76, 247, 8, COR_SUBTITULO);
    textoPlaneta(cartas.fase == CARTAS_ENCERRADAS ? "ENTER  Jogar novamente" : "ENTER  Próxima rodada",
                  76, 285, 8, COR_MARCADOR);
    textoPlaneta("ESC  Cidade", 468, 285, 8, COR_MENU);
}

static void desenharCartas(void)
{
    DrawRectangle(0, 0, 640, 360, GetColor(0x15111FFF));
    textoPlaneta("BLEFE ORBITAL", 24, 16, 16, COR_TITULO);
    textoPlaneta(TextFormat("VOCÊ %d x %d VISITANTE", cartas.pontosJogador, cartas.pontosAlien),
                  432, 22, 8, COR_MARCADOR);
    textoPlaneta(TextFormat("Rodada %d / 3", cartas.rodada), 24, 46, 8, COR_SUBTITULO);
    textoPlaneta(progressoPartida()->temTradutor ? "TRADUTOR EQUIPADO" : "SEM TRADUTOR",
                  432, 46, 8, ciano());
    caixaPlaneta(24, 66, 592, 88);
    desenharVisitante(58, 132);
    textoPlaneta("ANÚNCIO DO VISITANTE", 98, 78, 8, ciano());
    textoPlaneta(fala.texto, 98, 99, 16, COR_TITULO);
    textoPlaneta(progressoPartida()->temTradutor ? textoLeitura(fala.leitura) :
                  "Sem tradutor: leitura de tom indisponível.", 98, 132, 8, COR_SUBTITULO);
    caixaPlaneta(24, 166, 340, 140);
    caixaPlaneta(380, 166, 236, 140);
    for (int i = 0; i < CARTAS_NA_MAO; i++) desenharCarta(i);
    textoPlaneta("1 / 2 / 3  ou  A / D", 48, 293, 8, COR_MENU);
    const char *acoes[] = { "COMPARAR", "DESCONFIAR" };
    for (int i = 0; i < 2; i++) {
        Color cor = acaoSelecionada == i ? COR_MARCADOR : COR_MENU;
        DrawRectangle(396, 187 + i * 55, 204, 43, GetColor(0x30283FFF));
        if (acaoSelecionada == i) DrawRectangleLines(396, 187 + i * 55, 204, 43, cor);
        textoPlaneta(acoes[i], 416, (float)(201 + i * 55), 16, cor);
    }
    textoPlaneta("A/D  Carta   W/S  Ação   ENTER  Jogar   ESC  Cidade", 24, 330, 8, COR_MENU);
    if (cartas.fase != CARTAS_ESCOLHENDO) desenharResultado();
}

void desenharTelaExtraterrestre(void)
{
    if (modo == ET_CARTAS) { desenharCartas(); return; }
    desenharCidade();
    if (modo == ET_CONVERSA) desenharConversa();
    else if (modo == ET_REGRAS) desenharRegras();
}

void encerrarTelaExtraterrestre(void)
{
    encerrarCenarioPlaneta(&cidade);
    encerrarExploradorPlaneta(&explorador);
    iniciou = false;
}
