#include "tela_gigante.h"
#include "gigante_logica.h"
#include "cenario_planeta.h"
#include "constantes.h"
#include "partida.h"

#include <math.h>

enum { MAX_COBERTURAS = 3, MAX_OLHOS = 2 };
typedef struct {
    const char *nome;
    const char *imagem;
    ObstaculoExploracao coberturas[MAX_COBERTURAS];
    int totalCoberturas;
    OlhoGigante olhos[MAX_OLHOS];
    int totalOlhos;
    Rectangle restrita;
} AreaGigante;

static const AreaGigante areas[GIGANTE_TOTAL_AREAS] = {
    { "ENTRADA", "assets/imagens/planetas/gigante_entrada.png",
      {{186,250,34,28}, {402,250,34,28}}, 2,
      {{{355,182},1.5707963f,0.52f,236,4,2.5f,0}}, 1, {0,0,0,0} },
    { "PASSAGENS", "assets/imagens/planetas/gigante_passagens.png",
      {{205,240,30,52}, {393,265,40,28}}, 2,
      {{{218,182},1.28f,0.42f,220,4.2f,2.2f,0},
       {{465,182},1.85f,0.42f,220,4.2f,2.2f,3.2f}}, 2, {296,210,44,36} },
    { "PÁTIO", "assets/imagens/planetas/gigante_patio.png",
      {{160,235,40,44}, {290,270,44,38}, {450,235,40,44}}, 3,
      {{{200,182},1.35f,0.58f,230,4.4f,2.8f,0},
       {{440,182},1.80f,0.58f,230,4.4f,2.8f,3.6f}}, 2, {385,286,52,48} },
    { "CÂMARA", "assets/imagens/planetas/gigante_camara.png",
      {{230,255,32,36}, {378,255,32,36}}, 2,
      {{{320,182},1.5707963f,0.64f,240,4.8f,3,0}}, 1, {520,226,60,48} }
};

static CenarioPlaneta cenarios[GIGANTE_TOTAL_AREAS];
static ExploradorPlaneta explorador;
static VigilanciaGigante vigilancia;
static bool iniciou, ajudaAberta, explicou, marcoAberto, detectado;
static float tempoAviso;

static void sincronizarProgresso(void)
{
    ProgressoPartida *progresso = progressoPartida();
    progresso->areasGiganteVisitadas = vigilancia.areasVisitadas;
    progresso->capturasGigante = vigilancia.capturas;
    progresso->giganteConcluido = vigilancia.concluido;
}

void iniciarTelaGigante(void)
{
    if (iniciou) return;
    for (int i = 0; i < GIGANTE_TOTAL_AREAS; i++)
        iniciarCenarioPlaneta(&cenarios[i], areas[i].imagem);
    iniciarExploradorPlaneta(&explorador, (PontoExploracao){56,316});
    iniciou = true;
}

void entrarTelaGigante(void)
{
    if (!iniciou) iniciarTelaGigante();
    ProgressoPartida *progresso = progressoPartida();
    iniciarVigilanciaGigante(&vigilancia, progresso->areasGiganteVisitadas,
                            progresso->capturasGigante, progresso->giganteConcluido);
    explorador.pes = (PontoExploracao){56,316};
    explorador.direcao = 3;
    explorador.quadro = 0;
    explorador.tempo = 0;
    explorador.correndo = false;
    ajudaAberta = !explicou;
    marcoAberto = false;
    detectado = false;
    tempoAviso = 0;
    sincronizarProgresso();
}

static OlhoGigante olhoNoInstante(int indice, float tempo)
{
    OlhoGigante olho = areas[vigilancia.area].olhos[indice];
    /* Movimento lento e periódico, compartilhado pelo desenho e detecção. */
    olho.angulo += sinf(tempo * 0.45f + (float)indice) * 0.12f;
    return olho;
}

static OlhoGigante olhoAtual(int indice)
{
    return olhoNoInstante(indice, vigilancia.tempo);
}

static bool confirmar(void)
{
    return IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE);
}

static bool pertoDoMarco(void)
{
    return vigilancia.area == 3 && hypotf(explorador.pes.x - 320,
                                           explorador.pes.y - 224) < 42;
}

Tela atualizarTelaGigante(void)
{
    if (ajudaAberta) {
        if (confirmar()) { ajudaAberta = false; explicou = true; }
        return TELA_GIGANTE;
    }
    if (marcoAberto) {
        if (confirmar()) marcoAberto = false;
        return TELA_GIGANTE;
    }
    if (vigilancia.estado == GIGANTE_CAPTURADO) {
        if (confirmar()) {
            confirmarCapturaGigante(&vigilancia);
            explorador.pes = (PontoExploracao){56,316};
            explorador.quadro = 0;
            explorador.tempo = 0;
            explorador.correndo = false;
            detectado = false;
            tempoAviso = 0;
            sincronizarProgresso();
        }
        return TELA_GIGANTE;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        sincronizarProgresso();
        return TELA_NAVE;
    }

    float dt = GetFrameTime();
    if (!isfinite(dt) || dt < 0) dt = 0;
    if (dt > 0.1f) dt = 0.1f;
    if (tempoAviso > 0) tempoAviso -= dt;
    const AreaGigante *area = &areas[vigilancia.area];
    bool moveu = atualizarExploradorPlaneta(&explorador, &cenarios[vigilancia.area].piso,
        area->coberturas, area->totalCoberturas, dt, true);
    detectado = false;
    float instante = fmodf(vigilancia.tempo + dt, 3600.0f);
    for (int i = 0; i < area->totalOlhos; i++) {
        OlhoGigante olho = olhoNoInstante(i, instante);
        if (olhoGiganteVe(&olho, explorador.pes, instante,
                          area->coberturas, area->totalCoberturas)) detectado = true;
    }
    bool restrito = area->restrita.width > 0 &&
        CheckCollisionPointRec((Vector2){explorador.pes.x,explorador.pes.y - 4}, area->restrita);
    int antes = vigilancia.infracoes;
    bool capturou = atualizarVigilanciaGigante(&vigilancia, detectado,
                                               moveu && explorador.correndo, restrito, dt);
    if (vigilancia.infracoes != antes) tempoAviso = 3;
    sincronizarProgresso();
    if (capturou) return TELA_GIGANTE;

    if (IsKeyPressed(KEY_E)) {
        if (pertoDoMarco()) {
            concluirObjetivoGigante(&vigilancia);
            sincronizarProgresso();
            marcoAberto = true;
        } else if (explorador.pes.x <= 54) {
            if (vigilancia.area == 0) return TELA_NAVE;
            if (mudarAreaGigante(&vigilancia, -1)) {
                explorador.pes = (PontoExploracao){586,316};
                tempoAviso = 0;
                sincronizarProgresso();
            }
        } else if (explorador.pes.x >= 574 && vigilancia.area < 3) {
            if (mudarAreaGigante(&vigilancia, 1)) {
                explorador.pes = (PontoExploracao){54,316};
                tempoAviso = 0;
                sincronizarProgresso();
            }
        }
    }
    return TELA_GIGANTE;
}

static Color corAlerta(void)
{
    return vigilancia.estado >= GIGANTE_ALERTA ? GetColor(0xEF986EFF) : COR_MARCADOR;
}

static void desenharCampoDeVisao(OlhoGigante olho)
{
    if (!olhoGiganteAberto(&olho, vigilancia.tempo)) return;
    const AreaGigante *area = &areas[vigilancia.area];
    enum { RAIOS = 28 };
    Vector2 pontos[RAIOS + 1];
    for (int i = 0; i <= RAIOS; i++) {
        float angulo = olho.angulo - olho.meioAngulo + 2 * olho.meioAngulo * i / RAIOS;
        float distancia = alcanceRaioGigante(olho.origem, angulo, olho.alcance,
                                            area->coberturas, area->totalCoberturas);
        pontos[i] = (Vector2){ olho.origem.x + cosf(angulo) * distancia,
                              olho.origem.y + sinf(angulo) * distancia };
    }
    Color cor = Fade(corAlerta(), detectado ? 0.14f : 0.075f);
    for (int i = 0; i < RAIOS; i++)
        DrawTriangle((Vector2){olho.origem.x,olho.origem.y}, pontos[i + 1], pontos[i], cor);
}

static void desenharOlho(OlhoGigante olho)
{
    int x = (int)olho.origem.x;
    int y = (int)olho.origem.y - 3;
    bool aberto = olhoGiganteAberto(&olho, vigilancia.tempo);
    const int larguras[] = {12,28,40,48,40,28,12};
    for (int i = 0; i < 7; i++) {
        int largura = larguras[i];
        DrawRectangle(x - largura / 2, y - 14 + i * 4, largura, 4, GetColor(0x1B1726FF));
    }
    if (aberto) {
        DrawRectangle(x - 14, y - 5, 28, 10, GetColor(0xAD8752FF));
        DrawRectangle(x - 9, y - 9, 18, 18, corAlerta());
        DrawRectangle(x - 3, y - 10, 6, 20, GetColor(0x302332FF));
        DrawRectangle(x + 5, y - 7, 3, 3, COR_TITULO);
    } else {
        DrawRectangle(x - 19, y - 2, 38, 4, GetColor(0x796894FF));
        DrawRectangle(x - 13, y + 2, 26, 2, GetColor(0x514563FF));
    }
}

static void desenharRestrita(Rectangle r)
{
    if (r.width <= 0) return;
    DrawRectangleRec(r, Fade(GetColor(0xE6A074FF),0.08f));
    DrawRectangleLinesEx(r, 1, Fade(COR_MARCADOR,0.55f));
    for (int x = (int)r.x + 4; x < (int)(r.x + r.width) - 2; x += 10)
        DrawLine(x, (int)r.y + 2, x - 2, (int)r.y + 7, Fade(COR_MARCADOR,0.65f));
}

static void desenharSaida(int x, bool direita)
{
    Color cor = GetColor(0x68D8CDFF);
    DrawRectangle(x - 9, 280, 18, 42, GetColor(0x292234FF));
    DrawRectangle(x - 9, 280, 2, 42, cor);
    DrawRectangle(x + 7, 280, 2, 42, cor);
    int sentido = direita ? 1 : -1;
    DrawLine(x - sentido * 5, 300, x + sentido * 5, 300, cor);
    DrawLine(x + sentido * 5, 300, x, 295, cor);
    DrawLine(x + sentido * 5, 300, x, 305, cor);
}

static void desenharMarco(void)
{
    Color cor = vigilancia.concluido ? GetColor(0x68D8CDFF) : COR_MARCADOR;
    DrawRectangle(296, 211, 48, 18, GetColor(0x3D344FFF));
    DrawRectangle(302, 201, 36, 16, GetColor(0x5A4A70FF));
    DrawRectangleLines(302,201,36,16,cor);
    DrawRectangle(313,205,14,2,cor);
    DrawRectangle(319,203,2,10,cor);
    DrawRectangle(313,211,14,2,cor);
}

static void desenharInterfaceGigante(void)
{
    caixaPlaneta(16,12,352,56);
    textoPlaneta("PLANETA DO GIGANTE",26,21,16,COR_TITULO);
    textoPlaneta(TextFormat("ÁREA %d / 4  %s",vigilancia.area+1,areas[vigilancia.area].nome),26,48,8,COR_SUBTITULO);
    caixaPlaneta(400,12,224,56);
    textoPlaneta(nomeEstadoGigante(vigilancia.estado),412,20,8,corAlerta());
    textoPlaneta(TextFormat("%03d%%",(int)vigilancia.alerta),574,20,8,COR_MENU);
    DrawRectangle(412,36,200,5,GetColor(0x40334FFF));
    DrawRectangle(412,36,(int)(2*vigilancia.alerta),5,corAlerta());
    textoPlaneta(TextFormat("INFRAÇÕES %02d",vigilancia.infracoes),412,50,8,COR_MENU);
    caixaPlaneta(16,72,440,22);
    textoPlaneta(vigilancia.concluido ? "Marco registrado. Você pode voltar à nave." :
                  "Atravesse as quatro áreas e consulte o marco.",26,79,8,
                  vigilancia.concluido ? GetColor(0x68D8CDFF) : COR_MENU);
    if (tempoAviso > 0) {
        caixaPlaneta(16,100,360,24);
        textoPlaneta(nomeInfracaoGigante(vigilancia.ultimaInfracao),26,108,8,COR_MARCADOR);
    }
    DrawRectangle(0,346,640,14,GetColor(0x17131FFF));
    textoPlaneta("WASD/SETAS Mover   SHIFT Correr   E Interagir   ESC Nave",16,350,8,COR_MENU);
    if (pertoDoMarco()) {
        caixaPlaneta(244,166,152,22);
        textoPlaneta("E  Consultar marco",252,173,8,COR_MARCADOR);
    } else if (explorador.pes.x <= 54) {
        int y=(int)explorador.pes.y-96;
        caixaPlaneta(24,y,152,22);
        textoPlaneta(vigilancia.area == 0 ? "E  Voltar à nave" : "E  Área anterior",32,(float)y+7,8,COR_MARCADOR);
    } else if (explorador.pes.x >= 574 && vigilancia.area < 3) {
        int y=(int)explorador.pes.y-96;
        caixaPlaneta(456,y,160,22);
        textoPlaneta("E  Próxima área",464,(float)y+7,8,COR_MARCADOR);
    }
}

static void desenharOverlay(void)
{
    DrawRectangle(0,0,640,360,Fade(BLACK,0.68f));
    caixaPlaneta(40,62,560,242);
    if (vigilancia.estado == GIGANTE_CAPTURADO) {
        textoPlaneta("VOCÊ FOI CAPTURADO",64,88,16,COR_MARCADOR);
        textoPlaneta("Os olhos atingiram o limite de alerta.\n\nVocê retorna à entrada do planeta.\nAs áreas visitadas e o marco foram preservados.",
                      64,132,8,COR_TITULO);
        textoPlaneta(TextFormat("Capturas nesta partida: %d",vigilancia.capturas),64,222,8,COR_SUBTITULO);
    } else if (marcoAberto) {
        textoPlaneta("MARCO REGISTRADO",64,88,16,GetColor(0x68D8CDFF));
        textoPlaneta("Você alcançou a câmara e consultou o marco.\n\nA exploração das quatro áreas foi concluída.\nO registro permanece na sua partida.\n\nVolte pela passagem ou pressione Esc na área\npara retornar à nave.",64,132,8,COR_TITULO);
    } else {
        textoPlaneta("VIGILÂNCIA DO GIGANTE",64,88,16,COR_TITULO);
        textoPlaneta("Os olhos abrem e fecham em ciclos.\nUse os pilares como cobertura.\n\nSer visto, correr diante de um olho ou entrar\nem uma região marcada aumenta o alerta.\n\nNa quarta área, consulte o marco com E.\nCapturas devolvem você à entrada sem apagar progresso.",64,126,8,COR_MENU);
    }
    textoPlaneta("ENTER / E / ESC  Continuar",64,280,8,COR_TITULO);
}

void desenharTelaGigante(void)
{
    if (!iniciou) return;
    const AreaGigante *area = &areas[vigilancia.area];
    desenharCenarioPlaneta(&cenarios[vigilancia.area],GetColor(0x30283FFF),GetColor(0x8E78AEFF));
    desenharRestrita(area->restrita);
    for (int i = 0; i < area->totalOlhos; i++) desenharCampoDeVisao(olhoAtual(i));
    desenharSaida(32,false);
    if (vigilancia.area < 3) desenharSaida(608,true);
    for (int i = 0; i < area->totalCoberturas; i++)
        desenharObstaculoPlaneta(area->coberturas[i],GetColor(0x6F5A88FF));
    for (int i = 0; i < area->totalOlhos; i++) desenharOlho(olhoAtual(i));
    if (vigilancia.area == 3) desenharMarco();
    desenharExploradorPlaneta(&explorador);
    desenharInterfaceGigante();
    if (ajudaAberta || marcoAberto || vigilancia.estado == GIGANTE_CAPTURADO) desenharOverlay();
}

void encerrarTelaGigante(void)
{
    if (!iniciou) return;
    for (int i = 0; i < GIGANTE_TOTAL_AREAS; i++) encerrarCenarioPlaneta(&cenarios[i]);
    encerrarExploradorPlaneta(&explorador);
    iniciou = false;
    ajudaAberta = marcoAberto = false;
}
