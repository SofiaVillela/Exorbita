/* tela_gigante.c: planeta do gigante.

   Um planalto de pedra com o rosto do gigante ao fundo. Os olhos dele
   seguem, devagar, quem desceu. Pisar numa planta brilhante ou esbarrar
   em um habitante (o bichinho que anda aos pulos) é uma infração. Com
   infrações demais o gigante pega o tripulante, que tenta se explicar:
   cada resposta depende de um traço (regras em gigante_logica.c).
   O objetivo é chegar ao posto do outro lado e registrar o marco.

   O fundo é uma imagem de 640x360; por onde se pode andar vem de uma
   máscara do mesmo tamanho (branco = piso). */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "constantes.h"
#include "cenario_planeta.h"
#include "partida.h"
#include "dialogo.h"
#include "gigante_logica.h"
#include "tela_gigante.h"

typedef enum {
    GIG_AJUDA,        /* explicação, na primeira descida */
    GIG_EXPLORANDO,   /* andando pelo planalto */
    GIG_MARCO,        /* acabou de registrar o marco no posto */
    GIG_PEGO,         /* o gigante pegou o tripulante */
    GIG_ESCOLHENDO,   /* lista de respostas */
    GIG_RESULTADO     /* se a resposta convenceu ou não; depois volta à nave */
} EstadoTelaGigante;

#define ARQUIVO_FUNDO    "assets/imagens/planetas/gigante_planalto.png"
#define ARQUIVO_MASCARA  "assets/mascaras/gigante_planalto_mascara.png"

#define X_INICIO         90      /* onde o tripulante aparece */
#define Y_INICIO         216
#define X_POUSO          83      /* círculo de pouso desenhado no chão */
#define Y_POUSO          214
#define X_POSTO          489     /* chão em frente à porta do posto */
#define Y_POSTO          248
#define DISTANCIA_USAR   40      /* distância dos pés para o E funcionar */
#define EMPURRAO         13      /* o esbarrão afasta o tripulante do habitante */

/* Olhos: os pixels claros do fundo dentro destas faixas são os olhos.
   A pupila é desenhada só por cima deles. */
#define ALTURA_OLHOS     80      /* os olhos ficam nas primeiras 80 linhas */
#define LIMIAR_OLHO      140     /* vermelho acima disto = pixel do olho */
#define VELOCIDADE_OLHAR 1.5f    /* quanto menor, mais devagar o olho segue */
#define TOTAL_OLHOS      2

#define TOTAL_PLANTAS    14
#define TOTAL_HABITANTES 3
#define TOTAL_PEDRAS     14
#define TEMPO_AVISO      3.0f

typedef struct {
    Rectangle busca;     /* onde procurar os pixels do olho */
    float centroX, centroY;
    float raioX, raioY;  /* metade da largura e da altura do olho */
} Olho;

typedef struct {
    PontoExploracao lugar;
    bool pisada;
} Planta;

typedef struct {
    float x, y, velocidade;
} Pedra;

/* Plantas sobre o caminho; dá para desviar de todas. */
static const PontoExploracao lugaresPlantas[TOTAL_PLANTAS] = {
    { 62,178}, {118,200}, {150,236}, {215,258}, {250,230}, {293,213}, {327,250},
    {357,200}, {387,233}, {445,252}, {483,263}, {317,293}, {340,318}, {550,277}
};

/* x, y, minX, maxX, velocidade (o resto começa em zero). */
static const HabitanteGigante habitantesIniciais[TOTAL_HABITANTES] = {
    { 130, 248, 100, 165,  48, 0.00f, 0 },
    { 330, 227, 262, 404,  66, 0.20f, 0 },
    { 570, 282, 530, 615, -54, 0.35f, 0 }
};

static Texture2D fundo;
static Texture2D texturaOlhos;        /* pixels dos olhos em branco, para tingir */
static PixelExploracao *pixelsPiso;
static MapaExploracao piso;
static unsigned char ehOlho[ALTURA_OLHOS][LARGURA_VIRTUAL];
static Olho olhos[TOTAL_OLHOS] = {
    { {190, 40, 82, 38}, 237, 60, 36, 16 },
    { {366, 40, 82, 38}, 400, 60, 36, 16 }
};

static ExploradorPlaneta explorador;
static VigilanciaGigante vigilancia;
static Planta plantas[TOTAL_PLANTAS];
static HabitanteGigante habitantes[TOTAL_HABITANTES];
static Pedra pedras[TOTAL_PEDRAS];

static bool iniciou = false;
static bool explicou = false;     /* a ajuda só aparece na primeira descida */
static EstadoTelaGigante estado;
static float tempo;               /* relógio para as animações */
static float olharX, olharY;      /* para onde os olhos apontam, de -1 a 1 */
static float tremor, lento;       /* segundos que faltam de cada efeito */
static float tempoAviso;
static char aviso[96];
static char textoFala[256];
static int escolhida;
static bool convenceu;

/* ---------- Carregamento ---------- */

static void carregarPiso(void)
{
    Image imagem = LoadImage(ARQUIVO_MASCARA);
    if (imagem.data == NULL || imagem.width != LARGURA_VIRTUAL ||
        imagem.height != ALTURA_VIRTUAL) {
        TraceLog(LOG_ERROR, "GIGANTE: a mascara deve ter 640x360");
        if (imagem.data != NULL) {
            UnloadImage(imagem);
        }
        return;
    }
    Color *cores = LoadImageColors(imagem);
    int quantidade = imagem.width * imagem.height;
    pixelsPiso = malloc((size_t)quantidade * sizeof(*pixelsPiso));
    if (cores != NULL && pixelsPiso != NULL) {
        for (int i = 0; i < quantidade; i++) {
            pixelsPiso[i] = (PixelExploracao){ cores[i].r, cores[i].g, cores[i].b, cores[i].a };
        }
        piso = (MapaExploracao){ pixelsPiso, imagem.width, imagem.height };
    }
    if (cores != NULL) {
        UnloadImageColors(cores);
    }
    UnloadImage(imagem);
}

/* Procura os pixels dos olhos no fundo e guarda duas coisas: a tabela
   ehOlho (para a pupila não sair do olho) e uma textura só com esses
   pixels em branco (para tingir o olho conforme o humor do gigante). */
static void carregarOlhos(void)
{
    Image imagem = LoadImage(ARQUIVO_FUNDO);
    if (imagem.data == NULL || imagem.width != LARGURA_VIRTUAL ||
        imagem.height != ALTURA_VIRTUAL) {
        if (imagem.data != NULL) {
            UnloadImage(imagem);
        }
        return;
    }
    Color *cores = LoadImageColors(imagem);
    Image mascara = GenImageColor(LARGURA_VIRTUAL, ALTURA_OLHOS, BLANK);
    for (int i = 0; i < TOTAL_OLHOS && cores != NULL; i++) {
        Olho *olho = &olhos[i];
        int minX = LARGURA_VIRTUAL, maxX = 0, minY = ALTURA_OLHOS, maxY = 0;
        for (int y = (int)olho->busca.y; y < (int)(olho->busca.y + olho->busca.height); y++) {
            for (int x = (int)olho->busca.x; x < (int)(olho->busca.x + olho->busca.width); x++) {
                if (cores[y * LARGURA_VIRTUAL + x].r <= LIMIAR_OLHO) {
                    continue;
                }
                ehOlho[y][x] = 1;
                ImageDrawPixel(&mascara, x, y, WHITE);
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
        if (maxX > minX && maxY > minY) {
            olho->raioX = (float)(maxX - minX) / 2;
            olho->raioY = (float)(maxY - minY) / 2;
        }
    }
    texturaOlhos = LoadTextureFromImage(mascara);
    if (texturaOlhos.id != 0) {
        SetTextureFilter(texturaOlhos, TEXTURE_FILTER_POINT);
    }
    UnloadImage(mascara);
    if (cores != NULL) {
        UnloadImageColors(cores);
    }
    UnloadImage(imagem);
}

void iniciarTelaGigante(void)
{
    if (iniciou) {
        return;
    }
    fundo = LoadTexture(ARQUIVO_FUNDO);
    if (fundo.id != 0) {
        SetTextureFilter(fundo, TEXTURE_FILTER_POINT);
    }
    carregarPiso();
    carregarOlhos();
    iniciarExploradorPlaneta(&explorador, (PontoExploracao){ X_INICIO, Y_INICIO });
    iniciou = true;
}

void entrarTelaGigante(void)
{
    if (!iniciou) {
        iniciarTelaGigante();
    }
    iniciarVigilanciaGigante(&vigilancia);
    for (int i = 0; i < TOTAL_PLANTAS; i++) {
        plantas[i].lugar = lugaresPlantas[i];
        plantas[i].pisada = false;
    }
    for (int i = 0; i < TOTAL_HABITANTES; i++) {
        habitantes[i] = habitantesIniciais[i];
    }
    for (int i = 0; i < TOTAL_PEDRAS; i++) {
        pedras[i].y = ALTURA_VIRTUAL;   /* fora da tela: nenhuma caindo */
    }

    explorador.pes = (PontoExploracao){ X_INICIO, Y_INICIO };
    explorador.direcao = 3;
    explorador.quadro = 0;
    explorador.tempo = 0;
    explorador.correndo = false;

    tempo = 0;
    olharX = 0;
    olharY = 0;
    tremor = 0;
    lento = 0;
    tempoAviso = 0;
    escolhida = 0;
    convenceu = false;
    if (explicou) {
        estado = GIG_EXPLORANDO;
    } else {
        estado = GIG_AJUDA;
    }
}

/* ---------- Funções de apoio ---------- */

static bool confirmou(void)
{
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E);
}

static bool perto(float x, float y)
{
    float dx = explorador.pes.x - x;
    float dy = explorador.pes.y - y;
    return dx * dx + dy * dy <= DISTANCIA_USAR * DISTANCIA_USAR;
}

static void avisar(const char *mensagem)
{
    snprintf(aviso, sizeof aviso, "%s", mensagem);
    tempoAviso = TEMPO_AVISO;
}

static float limitar(float valor, float minimo, float maximo)
{
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

/* Os olhos giram aos poucos na direção de quem desceu. */
static void seguirComOsOlhos(float dt)
{
    float alvoX = limitar((explorador.pes.x - 320) / 320, -1, 1);
    float alvoY = limitar((explorador.pes.y - 250) / 117, -1, 1);
    float passo = limitar(dt * VELOCIDADE_OLHAR, 0, 1);
    olharX += (alvoX - olharX) * passo;
    olharY += (alvoY - olharY) * passo;
}

static void soltarPedras(void)
{
    for (int i = 0; i < TOTAL_PEDRAS; i++) {
        pedras[i].x = (float)GetRandomValue(0, LARGURA_VIRTUAL - 1);
        pedras[i].y = (float)-GetRandomValue(0, 100);
        pedras[i].velocidade = (float)GetRandomValue(100, 220);
    }
}

/* Registra a infração e mostra a reação do gigante. */
static void cometerInfracao(InfracaoGigante infracao)
{
    EstadoGigante antes = estadoGigante(&vigilancia);
    bool primeira = vigilancia.infracoes == 0;
    bool pego = registrarInfracaoGigante(&vigilancia, infracao);
    const char *nome = nomeInfracaoGigante(infracao);

    if (pego) {
        progressoPartida()->capturasGigante++;
        snprintf(textoFala, sizeof textoFala,
                 "A mão enorme desce e segura %s.\n"
                 "O gigante lembra a última infração:\n"
                 "%s.\n"
                 "%d infrações no total.",
                 tripulanteControlado()->nome, nome, vigilancia.infracoes);
        tremor = 0;
        tempoAviso = 0;
        estado = GIG_PEGO;
        return;
    }
    if (estadoGigante(&vigilancia) == GIGANTE_REACAO) {
        tremor = GIGANTE_TEMPO_TREMOR;
        lento = GIGANTE_TEMPO_LENTO;
        soltarPedras();
        if (antes != GIGANTE_REACAO) {
            avisar("O gigante reage: o chão treme e fica difícil andar.");
        } else {
            avisar(TextFormat("Infração: %s. O chão treme de novo.", nome));
        }
    } else if (primeira) {
        avisar(TextFormat("Infração: %s. O gigante ficou alerta.", nome));
    } else {
        avisar(TextFormat("Infração: %s.", nome));
    }
}

/* ---------- Lógica ---------- */

static void atualizarExploracao(float dt)
{
    if (acalmarGigante(&vigilancia, dt)) {
        if (vigilancia.pontos > 0) {
            avisar("O gigante está se acalmando.");
        } else {
            avisar("O gigante voltou a só observar.");
        }
    }

    /* Enquanto o chão treme, o tripulante anda mais devagar. */
    float dtAndar = dt;
    if (lento > 0) {
        dtAndar = dt * GIGANTE_FATOR_LENTO;
    }
    atualizarExploradorPlaneta(&explorador, &piso, NULL, 0, dtAndar, true);

    for (int i = 0; i < TOTAL_HABITANTES; i++) {
        HabitanteGigante *h = &habitantes[i];
        atualizarHabitanteGigante(h, dt);
        if (!habitanteEncostou(h, explorador.pes)) {
            continue;
        }
        h->atordoado = GIGANTE_TEMPO_ATORDOADO;
        /* Afasta o tripulante para cima ou para baixo, se houver chão. */
        PontoExploracao afastado = explorador.pes;
        if (explorador.pes.y > h->y) {
            afastado.y += EMPURRAO;
        } else {
            afastado.y -= EMPURRAO;
        }
        if (posicaoLivreExploracao(&piso, afastado, NULL, 0)) {
            explorador.pes = afastado;
        }
        cometerInfracao(INFRACAO_HABITANTE);
        if (estado != GIG_EXPLORANDO) {
            return;
        }
    }

    for (int i = 0; i < TOTAL_PLANTAS; i++) {
        if (plantas[i].pisada || !pisouNaPlanta(plantas[i].lugar, explorador.pes)) {
            continue;
        }
        plantas[i].pisada = true;
        cometerInfracao(INFRACAO_PLANTA);
        if (estado != GIG_EXPLORANDO) {
            return;
        }
    }

    if (IsKeyPressed(KEY_E)) {
        if (perto(X_POSTO, Y_POSTO) && !progressoPartida()->giganteConcluido) {
            progressoPartida()->giganteConcluido = true;
            estado = GIG_MARCO;
        }
    }
}

Tela atualizarTelaGigante(void)
{
    float dt = GetFrameTime();
    if (!isfinite(dt) || dt < 0) {
        dt = 0;
    }
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    tempo += dt;
    seguirComOsOlhos(dt);
    if (tremor > 0) tremor -= dt;
    if (lento > 0) lento -= dt;
    if (tempoAviso > 0) tempoAviso -= dt;
    for (int i = 0; i < TOTAL_PEDRAS; i++) {
        if (pedras[i].y < ALTURA_VIRTUAL) {
            pedras[i].y += pedras[i].velocidade * dt;
        }
    }

    if (estado == GIG_AJUDA) {
        if (confirmou() || IsKeyPressed(KEY_ESCAPE)) {
            explicou = true;
            estado = GIG_EXPLORANDO;
            avisar("Os olhos do gigante seguem cada passo.");
        }
        return TELA_GIGANTE;
    }

    if (estado == GIG_MARCO) {
        if (confirmou() || IsKeyPressed(KEY_ESCAPE)) {
            estado = GIG_EXPLORANDO;
        }
        return TELA_GIGANTE;
    }

    if (estado == GIG_PEGO) {
        if (confirmou()) {
            escolhida = 0;
            estado = GIG_ESCOLHENDO;
        }
        return TELA_GIGANTE;
    }

    if (estado == GIG_ESCOLHENDO) {
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            escolhida++;
            if (escolhida >= TOTAL_APELOS) {
                escolhida = 0;
            }
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            escolhida--;
            if (escolhida < 0) {
                escolhida = TOTAL_APELOS - 1;
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            Tripulante *quem = tripulanteControlado();
            int chance = chanceApeloGigante((TipoApelo)escolhida, &quem->tracos);
            convenceu = apeloConvenceu(chance, GetRandomValue(1, 100));
            if (convenceu) {
                snprintf(textoFala, sizeof textoFala,
                         "O gigante pensa e abre a mão.\n%s volta para a nave.", quem->nome);
            } else {
                /* Não convenceu: quem desceu não volta. A nave passa o
                   controle para outro tripulante (ver entrarTelaNave). */
                quem->vivo = false;
                snprintf(textoFala, sizeof textoFala,
                         "O gigante não aceita e fecha a mão.\n%s não volta para a nave.", quem->nome);
            }
            estado = GIG_RESULTADO;
        }
        return TELA_GIGANTE;
    }

    if (estado == GIG_RESULTADO) {
        if (confirmou() || IsKeyPressed(KEY_ESCAPE)) {
            return TELA_NAVE;
        }
        return TELA_GIGANTE;
    }

    /* GIG_EXPLORANDO */
    if (IsKeyPressed(KEY_ESCAPE)) {
        return TELA_NAVE;
    }
    if (IsKeyPressed(KEY_E) && perto(X_POUSO, Y_POUSO)) {
        return TELA_NAVE;
    }
    atualizarExploracao(dt);
    return TELA_GIGANTE;
}

/* ---------- Desenho ---------- */

static void desenharOlhos(EstadoGigante humor)
{
    /* Cor por cima do olho inteiro, conforme o humor. */
    if (texturaOlhos.id != 0) {
        if (humor == GIGANTE_ALERTA) {
            DrawTexture(texturaOlhos, 0, 0, Fade(GetColor(0xFFFAD7FF), 0.55f));
        } else if (humor == GIGANTE_REACAO) {
            DrawTexture(texturaOlhos, 0, 0, Fade(GetColor(0xF38054FF), 0.70f));
        } else if (humor == GIGANTE_CALMA) {
            DrawTexture(texturaOlhos, 0, 0, Fade(GetColor(0xC8BE96FF), 0.25f));
        }
    }

    /* A pupila encolhe quando o gigante se irrita. */
    float raio = 6.0f;
    Color borda = GetColor(0xB4957FFF);
    Color centro = GetColor(0x8C6F5CFF);
    if (humor == GIGANTE_ALERTA) {
        raio = 4.5f;
    } else if (humor == GIGANTE_REACAO) {
        raio = 3.8f;
        borda = GetColor(0x7A2A20FF);
        centro = GetColor(0x4A1612FF);
    }

    int alcance = (int)ceilf(raio);
    for (int i = 0; i < TOTAL_OLHOS; i++) {
        float cx = olhos[i].centroX + olharX * olhos[i].raioX * 0.32f;
        float cy = olhos[i].centroY + olharY * olhos[i].raioY * 0.25f;
        for (int dy = -alcance; dy <= alcance; dy++) {
            for (int dx = -alcance; dx <= alcance; dx++) {
                float distancia2 = (float)(dx * dx + dy * dy);
                if (distancia2 > raio * raio + 0.5f) {
                    continue;
                }
                int x = (int)roundf(cx) + dx;
                int y = (int)roundf(cy) + dy;
                /* Só pinta se o pixel fizer parte do olho. */
                if (x < 0 || x >= LARGURA_VIRTUAL || y < 0 || y >= ALTURA_OLHOS || !ehOlho[y][x]) {
                    continue;
                }
                if (distancia2 < 6) {
                    DrawPixel(x, y, centro);
                } else {
                    DrawPixel(x, y, borda);
                }
            }
        }
    }
}

static void desenharPlanta(const Planta *planta, float apagar)
{
    int x = (int)planta->lugar.x;
    int y = (int)planta->lugar.y;
    Color caule = GetColor(0x3F8A78FF);
    if (planta->pisada) {
        DrawRectangle(x - 5, y - 1, 10, 2, caule);
        DrawRectangle(x - 2, y - 2, 4, 1, GetColor(0x5FAE9AFF));
        return;
    }
    /* O brilho pulsa de leve; cada planta num ritmo um pouco diferente. */
    float pulso = (0.65f + 0.35f * sinf(tempo * 2.4f + (float)x)) * apagar;
    Color luz = GetColor(0x8FF5D4FF);
    DrawCircleGradient(x, y - 7, 13, Fade(luz, 0.22f * pulso), Fade(luz, 0));
    DrawRectangle(x - 1, y - 8, 2, 8, caule);
    DrawRectangle(x - 5, y - 5, 2, 5, caule);
    DrawRectangle(x + 4, y - 6, 2, 6, caule);
    DrawRectangle(x - 2, y - 12, 4, 4, Fade(luz, pulso));
    DrawRectangle(x - 6, y - 8, 3, 3, Fade(luz, pulso));
    DrawRectangle(x + 3, y - 9, 3, 3, Fade(luz, pulso));
}

/* O habitante: um quadradinho escuro de olhos amarelos. No ar ele
   estica um pouco; ao tocar o chão, achata. */
static void desenharHabitante(const HabitanteGigante *h)
{
    int x = (int)h->x;
    int y = (int)h->y;
    float pulo = alturaPuloHabitante(h);
    int subida = (int)(pulo * 9);
    int largura = 18, altura = 18;
    if (h->atordoado <= 0) {
        if (pulo > 0.15f) {
            largura = 16;
            altura = 20;
        } else if (h->relogio >= GIGANTE_TEMPO_PULO) {
            largura = 20;
            altura = 16;
        }
    }
    int esquerda = x - largura / 2;
    int topo = y - altura - subida;

    /* A sombra diminui quando ele sobe. */
    DrawEllipse(x, y, (float)(9 - subida / 3), 3, Fade(BLACK, 0.35f));
    DrawRectangle(esquerda, topo, largura, altura, GetColor(0x1F1830FF));
    DrawRectangle(esquerda + 2, topo + 2, largura - 4, altura - 4, GetColor(0x3D3160FF));
    DrawRectangle(esquerda + 2, topo + 2, largura - 4, 2, GetColor(0x5E4FA8FF));

    Color olho = GetColor(0xF2E6A0FF);
    if (h->atordoado > 0) {
        /* Atordoado: olhos fechados e uma estrelinha em cima. */
        DrawRectangle(x - 6, topo + 8, 4, 2, olho);
        DrawRectangle(x + 2, topo + 8, 4, 2, olho);
        if (fmodf(tempo, 0.4f) < 0.2f) {
            DrawRectangle(x - 2, topo - 8, 4, 4, olho);
        } else {
            DrawRectangle(x - 1, topo - 7, 2, 2, olho);
        }
    } else {
        /* Os olhos apontam um pouco para o lado em que ele anda. */
        int lado = 1;
        if (h->velocidade < 0) {
            lado = -1;
        }
        DrawRectangle(x - 5 + lado, topo + 6, 2, 4, olho);
        DrawRectangle(x + 3 + lado, topo + 6, 2, 4, olho);
    }
}

static void desenharPedras(void)
{
    for (int i = 0; i < TOTAL_PEDRAS; i++) {
        if (pedras[i].y < 0 || pedras[i].y >= ALTURA_VIRTUAL) {
            continue;
        }
        int x = (int)pedras[i].x;
        int y = (int)pedras[i].y;
        DrawRectangle(x, y, 3, 3, GetColor(0x4D3E6EFF));
        DrawRectangle(x + 1, y + 1, 2, 2, GetColor(0x2B2142FF));
    }
}

/* Texto com sombra, para ler por cima do cenário sem precisar de caixa. */
static void textoSombra(const char *s, int x, int y, Color cor)
{
    textoPlaneta(s, (float)x + 1, (float)y + 1, 8, BLACK);
    textoPlaneta(s, (float)x, (float)y, 8, cor);
}

/* Largura do texto na fonte de 8 px. Conta letras, não bytes: em UTF-8
   uma letra com acento ocupa 2 bytes, e o segundo começa com os bits 10. */
static int larguraTexto(const char *s)
{
    int largura = 0;
    for (const char *c = s; *c != '\0'; c++) {
        if (((unsigned char)*c & 0xC0) != 0x80) {
            largura += 8;
        }
    }
    return largura;
}

static void dicaAcimaDoJogador(const char *s)
{
    int largura = larguraTexto(s) + 16;
    int x = (int)explorador.pes.x - largura / 2;
    int y = (int)explorador.pes.y - 96;
    if (x < 8) x = 8;
    if (x + largura > LARGURA_VIRTUAL - 8) x = LARGURA_VIRTUAL - 8 - largura;
    caixaPlaneta(x, y, largura, 22);
    textoPlaneta(s, (float)(x + 8), (float)(y + 7), 8, COR_MARCADOR);
}

static void desenharInterface(EstadoGigante humor)
{
    Color corHumor = COR_TITULO;
    if (humor == GIGANTE_REACAO) {
        corHumor = COR_ALERTA;
    } else if (humor == GIGANTE_ALERTA) {
        corHumor = COR_MARCADOR;
    }

    /* Linha de cima, sem caixa, para não cobrir o rosto do gigante. */
    textoSombra(tripulanteControlado()->nome, 16, 8, COR_TITULO);
    textoSombra("Planalto do Gigante", 72, 8, COR_TITULO);
    textoSombra("INFRAÇÕES", 248, 8, COR_TITULO);
    DrawRectangle(328, 9, 60, 6, Fade(WHITE, 0.18f));
    DrawRectangle(328, 9, 60 * vigilancia.pontos / GIGANTE_LIMITE_INFRACOES, 6, COR_ALERTA);
    textoSombra(TextFormat("%d/%d", vigilancia.pontos, GIGANTE_LIMITE_INFRACOES), 396, 8, COR_TITULO);
    textoSombra(TextFormat("GIGANTE: %s", nomeEstadoGigante(humor)), 444, 8, corHumor);

    DrawRectangle(0, 346, LARGURA_VIRTUAL, 14, GetColor(0x17131FFF));
    textoPlaneta("WASD/SETAS Mover   SHIFT Correr   E Interagir   ESC Nave", 16, 350, 8, COR_MENU);

    if (estado != GIG_EXPLORANDO) {
        return;
    }
    if (perto(X_POUSO, Y_POUSO)) {
        dicaAcimaDoJogador("E  Voltar à nave");
    } else if (perto(X_POSTO, Y_POSTO)) {
        if (progressoPartida()->giganteConcluido) {
            dicaAcimaDoJogador("Marco já registrado");
        } else {
            dicaAcimaDoJogador("E  Registrar marco");
        }
    }
    if (tempoAviso > 0) {
        int largura = larguraTexto(aviso) + 16;
        int x = (LARGURA_VIRTUAL - largura) / 2;
        caixaPlaneta(x, 318, largura, 22);
        textoPlaneta(aviso, (float)(x + 8), 325, 8, COR_TITULO);
    }
}

/* Falantes: o gigante (o rosto vem do próprio fundo) e quem desceu. */
static Falante falanteGigante(void)
{
    Falante f = { "O Gigante", fundo, (Rectangle){ 230, 0, 180, 180 } };
    return f;
}

static Falante falanteVisitante(void)
{
    Falante f = { tripulanteControlado()->nome, explorador.sprite, (Rectangle){ 4, 0, 36, 36 } };
    return f;
}

static void desenharDialogos(void)
{
    if (estado == GIG_AJUDA) {
        Falante visitante = falanteVisitante();
        desenharFala(&visitante,
                     "Ao fundo, o gigante observa o planalto.\n"
                     "Não pise nas plantas brilhantes nem esbarre\n"
                     "nos habitantes: cada infração o irrita.\n"
                     "Chegue ao posto, do outro lado.",
                     "E / ENTER  Continuar", false);
    } else if (estado == GIG_MARCO) {
        Falante visitante = falanteVisitante();
        desenharFala(&visitante,
                     "Marco registrado no posto.\n"
                     "Volte ao círculo de pouso para subir à nave.",
                     "E / ENTER  Continuar", false);
    } else if (estado == GIG_PEGO) {
        Falante gigante = falanteGigante();
        desenharFala(&gigante, textoFala, "E / ENTER  Continuar", true);
    } else if (estado == GIG_ESCOLHENDO) {
        const char *falas[TOTAL_APELOS];
        for (int i = 0; i < TOTAL_APELOS; i++) {
            falas[i] = apeloGigante((TipoApelo)i)->fala;
        }
        Falante visitante = falanteVisitante();
        desenharEscolhaDialogo(&visitante,
                               TextFormat("O que %s faz?", tripulanteControlado()->nome),
                               falas, TOTAL_APELOS, escolhida,
                               apeloGigante((TipoApelo)escolhida)->detalhe,
                               "SETAS  Escolher   ENTER  Responder");
    } else if (estado == GIG_RESULTADO) {
        Falante gigante = falanteGigante();
        desenharFala(&gigante, textoFala, "E / ENTER  Voltar à nave", false);
    }
}

void desenharTelaGigante(void)
{
    if (!iniciou) {
        return;
    }
    EstadoGigante humor = estadoGigante(&vigilancia);
    ClearBackground(GetColor(0x28263BFF));

    /* O cenário inteiro treme junto; a interface fica parada. */
    Camera2D camera = { 0 };
    camera.zoom = 1;
    if (tremor > 0) {
        camera.offset = (Vector2){ (float)GetRandomValue(-2, 2), (float)GetRandomValue(-1, 1) };
    }
    BeginMode2D(camera);
    if (fundo.id != 0) {
        DrawTexture(fundo, 0, 0, WHITE);
    }
    desenharOlhos(humor);

    /* As plantas perdem o brilho enquanto o gigante reage. */
    float apagar = 1;
    if (humor == GIGANTE_REACAO) {
        apagar = 0.45f;
    }
    for (int i = 0; i < TOTAL_PLANTAS; i++) {
        desenharPlanta(&plantas[i], apagar);
    }

    /* Quem está mais para cima na tela é desenhado primeiro (fica atrás). */
    for (int i = 0; i < TOTAL_HABITANTES; i++) {
        if (habitantes[i].y <= explorador.pes.y) {
            desenharHabitante(&habitantes[i]);
        }
    }
    desenharExploradorPlaneta(&explorador);
    for (int i = 0; i < TOTAL_HABITANTES; i++) {
        if (habitantes[i].y > explorador.pes.y) {
            desenharHabitante(&habitantes[i]);
        }
    }

    desenharPedras();
    if (humor == GIGANTE_REACAO) {
        float vermelho = 0.06f + 0.04f * sinf(tempo * 6);
        DrawRectangle(-4, -4, LARGURA_VIRTUAL + 8, ALTURA_VIRTUAL + 8,
                      Fade(GetColor(0xCF4F3FFF), vermelho));
    }
    EndMode2D();

    desenharInterface(humor);
    desenharDialogos();
}

void encerrarTelaGigante(void)
{
    if (!iniciou) {
        return;
    }
    if (fundo.id != 0) {
        UnloadTexture(fundo);
    }
    if (texturaOlhos.id != 0) {
        UnloadTexture(texturaOlhos);
    }
    free(pixelsPiso);
    pixelsPiso = NULL;
    piso = (MapaExploracao){ 0 };
    encerrarExploradorPlaneta(&explorador);
    iniciou = false;
}
