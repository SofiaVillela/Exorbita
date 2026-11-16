/* tela_gigante.c: planeta do gigante.

   Um planalto de pedra com o rosto do gigante ao fundo. Os olhos dele
   seguem, devagar, quem desceu. Pisar numa planta brilhante ou esbarrar
   em um habitante (o bichinho que anda aos pulos) é uma infração. Com
   infrações demais o gigante pega o tripulante, que tenta se explicar:
   cada resposta depende de um traço (regras em gigante_logica.c).

   O objetivo é abrir o posto, perto do pouso, que guarda um pedaço do
   mapa estelar. Ele só abre com três cristais de luz: um no bosque das
   plantas (oeste), um no ninho dos habitantes e um no altar depois das
   pontes (leste). Nos destroços do extremo oeste há uma célula de
   combustível.

   O planalto tem 2560x360 e a câmera acompanha o tripulante para os
   lados. O desenho vem em duas camadas, feitas por
   ferramentas/gerar_gigante.py: o céu com o rosto do gigante, que fica
   parado na tela, e o terreno, que corre por cima dele. Por onde se
   pode andar vem de uma máscara do tamanho do terreno (branco = piso);
   cada pixel verde puro da máscara marca uma planta brilhante. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "constantes.h"
#include "cenario_planeta.h"
#include "partida.h"
#include "dialogo.h"
#include "gigante_logica.h"
#include "pilha_mochila.h"
#include "tela_gigante.h"
#include "sons.h"

typedef enum {
    GIG_AJUDA,        /* explicação, na primeira descida */
    GIG_EXPLORANDO,   /* andando pelo planalto */
    GIG_MENSAGEM,     /* o tripulante conta o que achou ou o que falta */
    GIG_PEGO,         /* o gigante pegou o tripulante */
    GIG_ESCOLHENDO,   /* lista de respostas */
    GIG_RESULTADO     /* se a resposta convenceu ou não; depois volta à nave */
} EstadoTelaGigante;

#define ARQUIVO_FUNDO    "assets/imagens/planetas/gigante_planalto.png"   /* desenho original: o centro do mapa */
#define ARQUIVO_CEU      "assets/imagens/planetas/gigante_ceu.png"
#define ARQUIVO_TERRENO  "assets/imagens/planetas/gigante_terreno.png"
#define ARQUIVO_MASCARA  "assets/mascaras/gigante_terreno_mascara.png"
#define ARQUIVO_ITENS    "assets/imagens/gigante_itens.png"
#define ARQUIVO_MAPA     "assets/imagens/sem_ar_itens.png"   /* de onde vem o desenho do pedaço do mapa */
#define QUADRO_MAPA      ITEM_MAPA   /* posição do pedaço do mapa nessa folha */

#define LARGURA_MAPA     2560
#define X_CENTRO         960     /* onde o desenho original começa dentro do mapa */
#define MARGEM_CEU       32      /* sobra do céu de cada lado: ele desliza até isto */

#define X_INICIO         (X_CENTRO + 90)    /* onde o tripulante aparece */
#define Y_INICIO         216
#define X_POUSO          (X_CENTRO + 83)    /* círculo de pouso desenhado no chão */
#define Y_POUSO          214
#define X_POSTO          (X_CENTRO + 489)   /* chão em frente à porta do posto */
#define Y_POSTO          248
#define X_CELULA         150     /* célula de combustível, nos destroços */
#define Y_CELULA         288
#define DISTANCIA_USAR   40      /* distância dos pés para o E funcionar */
#define EMPURRAO         13      /* o esbarrão afasta o tripulante do habitante */

/* Olhos: os pixels claros do fundo dentro destas faixas são os olhos.
   A pupila é desenhada só por cima deles. */
#define ALTURA_OLHOS     80      /* os olhos ficam nas primeiras 80 linhas */
#define LIMIAR_OLHO      140     /* vermelho acima disto = pixel do olho */
#define VELOCIDADE_OLHAR 1.5f    /* quanto menor, mais devagar o olho segue */
#define TOTAL_OLHOS      2

#define MAXIMO_PLANTAS   96      /* quantas plantas a máscara pode marcar */
#define TOTAL_HABITANTES 13
#define TOTAL_PEDRAS     14
#define TEMPO_AVISO      3.0f

enum { PREMIO_NENHUM, PREMIO_CRISTAL, PREMIO_CELULA, PREMIO_MAPA };

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

/* Onde ficam os cristais de luz (as mesmas posições de gerar_gigante.py). */
static const PontoExploracao lugaresCristais[GIGANTE_TOTAL_CRISTAIS] = {
    {  400, 196 },   /* bosque das plantas, a oeste */
    { 2010, 176 },   /* ninho dos habitantes, a leste */
    { 2466, 232 }    /* altar, depois das pontes */
};

/* x, y, minX, maxX, velocidade, relógio (o resto começa em zero). */
static const HabitanteGigante habitantesIniciais[TOTAL_HABITANTES] = {
    /* centro: os três de sempre */
    { 1090, 248, 1060, 1125,  48, 0.00f, 0 },
    { 1290, 227, 1222, 1364,  66, 0.20f, 0 },
    { 1530, 282, 1490, 1575, -54, 0.35f, 0 },
    /* oeste: clareira, bosque, trilha do sul e destroços */
    {  740, 212,  660,  850,  58, 0.05f, 0 },
    {  460, 226,  300,  620,  60, 0.10f, 0 },
    {  420, 306,  300,  600, -52, 0.30f, 0 },
    {  240, 250,  205,  282,  40, 0.15f, 0 },
    /* leste: clareira, ninho (três, um deles rente ao cristal), ponte e altar */
    { 1820, 262, 1770, 1878, -46, 0.25f, 0 },
    { 1950, 206, 1890, 2130,  70, 0.00f, 0 },
    { 2080, 232, 1890, 2108, -62, 0.30f, 0 },
    { 2030, 168, 1980, 2080,  44, 0.12f, 0 },
    { 2250, 190, 2130, 2390,  64, 0.20f, 0 },
    { 2460, 290, 2405, 2515,  50, 0.40f, 0 }
};

static Texture2D fundo;               /* só para o retrato do gigante nas falas */
static Texture2D ceu;                 /* rosto do gigante: fica parado na tela */
static Texture2D terreno;             /* o chão, que corre com a câmera */
static Texture2D folhaItens;          /* cristal e célula, 32x32 cada */
static Texture2D folhaMapa;
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
static Planta plantas[MAXIMO_PLANTAS];
static PontoExploracao lugaresPlantas[MAXIMO_PLANTAS];   /* lidos da máscara */
static int totalPlantas;
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
static int premio;                /* desenho mostrado junto da mensagem (PREMIO_...) */
static int escolhida;
static bool convenceu;

/* ---------- Carregamento ---------- */

static void carregarPiso(void)
{
    Image imagem = LoadImage(ARQUIVO_MASCARA);
    if (imagem.data == NULL || imagem.width != LARGURA_MAPA ||
        imagem.height != ALTURA_VIRTUAL) {
        TraceLog(LOG_ERROR, "GIGANTE: a mascara deve ter 2560x360");
        if (imagem.data != NULL) {
            UnloadImage(imagem);
        }
        return;
    }
    Color *cores = LoadImageColors(imagem);
    int quantidade = imagem.width * imagem.height;
    pixelsPiso = malloc((size_t)quantidade * sizeof(*pixelsPiso));
    if (cores != NULL && pixelsPiso != NULL) {
        totalPlantas = 0;
        for (int i = 0; i < quantidade; i++) {
            pixelsPiso[i] = (PixelExploracao){ cores[i].r, cores[i].g, cores[i].b, cores[i].a };
            /* Verde puro: aqui cresce uma planta brilhante. */
            if (cores[i].r == 0 && cores[i].g == 255 && cores[i].b == 0 && totalPlantas < MAXIMO_PLANTAS) {
                lugaresPlantas[totalPlantas].x = (float)(i % imagem.width);
                lugaresPlantas[totalPlantas].y = (float)(i / imagem.width);
                totalPlantas++;
            }
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
    Texture2D *texturas[] = { &fundo, &ceu, &terreno, &folhaItens, &folhaMapa };
    const char *arquivos[] = { ARQUIVO_FUNDO, ARQUIVO_CEU, ARQUIVO_TERRENO, ARQUIVO_ITENS, ARQUIVO_MAPA };
    for (int i = 0; i < 5; i++) {
        *texturas[i] = LoadTexture(arquivos[i]);
        if (texturas[i]->id != 0) {
            SetTextureFilter(*texturas[i], TEXTURE_FILTER_POINT);
        }
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
    for (int i = 0; i < totalPlantas; i++) {
        plantas[i].lugar = lugaresPlantas[i];
        plantas[i].pisada = false;
    }
    premio = PREMIO_NENHUM;
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

/* Canto esquerdo da parte do planalto que aparece na tela: a câmera
   segue o tripulante para os lados e para nas pontas do mapa. */
static float cantoDaCamera(void)
{
    return floorf(limitar(explorador.pes.x - LARGURA_VIRTUAL / 2, 0, LARGURA_MAPA - LARGURA_VIRTUAL));
}

/* O céu desliza só um pouquinho quando a câmera anda (MARGEM_CEU de
   uma ponta à outra do mapa): é o que dá a sensação de que o gigante
   está muito longe. Devolve o deslocamento do céu na tela. */
static int deslocamentoDoCeu(void)
{
    float meio = (LARGURA_MAPA - LARGURA_VIRTUAL) / 2.0f;
    return (int)roundf(-(cantoDaCamera() - meio) * MARGEM_CEU / meio);
}

static void mostrarMensagem(int qualPremio)
{
    premio = qualPremio;
    estado = GIG_MENSAGEM;
}

/* Os olhos giram aos poucos na direção de quem desceu. */
static void seguirComOsOlhos(float dt)
{
    /* No centro do mapa vale a conta de antes; nas pontas o olhar chega ao limite. */
    float alvoX = limitar((explorador.pes.x - LARGURA_MAPA / 2) / 320, -1, 1);
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

    /* O som do que foi feito: a planta amassada ou o bichinho assustado. */
    if (infracao == INFRACAO_PLANTA) {
        tocarSom(SOM_PLANTA);
    } else {
        tocarSom(SOM_HABITANTE);
    }
    if (pego) {
        tocarSom(SOM_GIGANTE_PEGA);
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
        tocarSom(SOM_GIGANTE_REAGE);
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

/* ---------- Cristais, célula e posto ---------- */

/* Índice do cristal ainda no chão que está ao alcance, ou -1. */
static int cristalPerto(void)
{
    for (int i = 0; i < GIGANTE_TOTAL_CRISTAIS; i++) {
        if (!temCristalGigante(progressoPartida()->cristaisGigante, i) &&
            perto(lugaresCristais[i].x, lugaresCristais[i].y)) {
            return i;
        }
    }
    return -1;
}

static bool celulaPerto(void)
{
    return !progressoPartida()->celulaGiganteUsada && perto(X_CELULA, Y_CELULA);
}

/* A tecla E, andando pelo planalto: cristal, célula ou posto. */
static void usarOQueEstaPerto(void)
{
    ProgressoPartida *jogo = progressoPartida();
    int cristal = cristalPerto();

    if (cristal >= 0) {
        jogo->cristaisGigante = recolherCristalGigante(jogo->cristaisGigante, cristal);
        int quantos = contarCristaisGigante(jogo->cristaisGigante);
        tocarSom(SOM_ITEM);
        if (postoGiganteAbre(jogo->cristaisGigante)) {
            snprintf(textoFala, sizeof textoFala,
                     "O terceiro cristal de luz.\n"
                     "Agora o posto, perto do pouso, pode abrir.");
        } else {
            snprintf(textoFala, sizeof textoFala,
                     "Um cristal de luz: %d de %d.\n"
                     "O gigante acompanha o cristal com os olhos,\n"
                     "mas não se mexe.",
                     quantos, GIGANTE_TOTAL_CRISTAIS);
        }
        mostrarMensagem(PREMIO_CRISTAL);
        return;
    }

    if (celulaPerto()) {
        jogo->celulaGiganteUsada = true;
        jogo->combustivel = abastecerComCelulaGigante(jogo->combustivel);
        tocarSom(SOM_ITEM);
        snprintf(textoFala, sizeof textoFala,
                 "Nos destroços de uma sonda antiga, uma\n"
                 "célula de combustível ainda cheia.\n"
                 "Combustível da nave: %d.", jogo->combustivel);
        mostrarMensagem(PREMIO_CELULA);
        return;
    }

    if (perto(X_POSTO, Y_POSTO) && !jogo->giganteConcluido) {
        if (postoGiganteAbre(jogo->cristaisGigante)) {
            jogo->giganteConcluido = true;
            tocarSom(SOM_PORTAO);
            tocarSom(SOM_VITORIA);
            snprintf(textoFala, sizeof textoFala,
                     "Os três cristais acendem o posto e o cofre\n"
                     "abre. Dentro, um pedaço do mapa estelar.\n"
                     "Volte ao círculo de pouso para subir à nave.");
            mostrarMensagem(PREMIO_MAPA);
        } else {
            tocarSom(SOM_NEGADO);
            snprintf(textoFala, sizeof textoFala,
                     "O posto está sem energia. O painel pede\n"
                     "três cristais de luz: há %d.\n"
                     "Eles crescem longe daqui, a oeste e a leste.",
                     contarCristaisGigante(jogo->cristaisGigante));
            mostrarMensagem(PREMIO_NENHUM);
        }
    }
}

/* ---------- Lógica ---------- */

static void atualizarExploracao(float dt)
{
    if (acalmarGigante(&vigilancia, dt)) {
        tocarSom(SOM_GIGANTE_ACALMA);
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

    for (int i = 0; i < totalPlantas; i++) {
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
        usarOQueEstaPerto();
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
    /* O zumbido de fundo acompanha a fúria: quanto mais pontos, mais forte e mais rápido. */
    definirTensao((float)vigilancia.pontos / GIGANTE_LIMITE_INFRACOES);
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

    if (estado == GIG_MENSAGEM) {
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
            tocarSom(SOM_UI_MOVER);
            escolhida++;
            if (escolhida >= TOTAL_APELOS) {
                escolhida = 0;
            }
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            tocarSom(SOM_UI_MOVER);
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
                tocarSom(SOM_GIGANTE_ACALMA);
                snprintf(textoFala, sizeof textoFala,
                         "O gigante pensa e abre a mão.\n%s volta para a nave.", quem->nome);
            } else {
                /* Não convenceu: quem desceu não volta. A nave passa o
                   controle para outro tripulante (ver entrarTelaNave). */
                quem->vivo = false;
                tocarSom(SOM_PERDA);
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

/* desloca: quanto o céu andou para o lado (ver deslocamentoDoCeu). */
static void desenharOlhos(EstadoGigante humor, int desloca)
{
    /* Cor por cima do olho inteiro, conforme o humor. */
    if (texturaOlhos.id != 0) {
        if (humor == GIGANTE_ALERTA) {
            DrawTexture(texturaOlhos, desloca, 0, Fade(GetColor(0xFFFAD7FF), 0.55f));
        } else if (humor == GIGANTE_REACAO) {
            DrawTexture(texturaOlhos, desloca, 0, Fade(GetColor(0xF38054FF), 0.70f));
        } else if (humor == GIGANTE_CALMA) {
            DrawTexture(texturaOlhos, desloca, 0, Fade(GetColor(0xC8BE96FF), 0.25f));
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
                    DrawPixel(x + desloca, y, centro);
                } else {
                    DrawPixel(x + desloca, y, borda);
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

/* Cristal de luz ainda no chão: flutua um pouco e pulsa. */
static void desenharCristal(int indice)
{
    float x = lugaresCristais[indice].x;
    float y = lugaresCristais[indice].y;
    float sobe = sinf(tempo * 2.2f + (float)indice * 1.7f) * 2.5f;
    float pulso = 0.7f + 0.3f * sinf(tempo * 3.1f + (float)indice);
    Color luz = GetColor(0xA5DCC4FF);
    DrawEllipse((int)x, (int)y, 9, 3, Fade(BLACK, 0.30f));
    DrawCircleGradient((int)x, (int)(y - 16 + sobe), 30, Fade(luz, 0.30f * pulso), Fade(luz, 0));
    DrawTextureRec(folhaItens, (Rectangle){ 0, 0, 32, 32 },
                   (Vector2){ x - 16, floorf(y - 34 + sobe) }, WHITE);
    /* Uma faísca sobe de tempos em tempos, para se ver de longe. */
    float faisca = fmodf(tempo * 0.8f + (float)indice * 0.37f, 1);
    DrawRectangle((int)x - 9 + indice * 5, (int)(y - 22 - faisca * 26), 2, 2, Fade(WHITE, 1 - faisca));
}

static void desenharCelula(void)
{
    DrawEllipse(X_CELULA, Y_CELULA, 8, 3, Fade(BLACK, 0.30f));
    DrawTextureRec(folhaItens, (Rectangle){ 32, 0, 32, 32 }, (Vector2){ X_CELULA - 16, Y_CELULA - 30 }, WHITE);
    if (fmodf(tempo, 1.2f) < 0.6f) {
        DrawRectangle(X_CELULA - 1, Y_CELULA - 29, 2, 2, GetColor(0xF2C14EFF));   /* luz piscando */
    }
}

/* Itens do chão. atras = true desenha só os que ficam atrás do tripulante. */
static void desenharItens(bool atras)
{
    const ProgressoPartida *jogo = progressoPartida();
    for (int i = 0; i < GIGANTE_TOTAL_CRISTAIS; i++) {
        if (!temCristalGigante(jogo->cristaisGigante, i) &&
            (lugaresCristais[i].y <= explorador.pes.y) == atras) {
            desenharCristal(i);
        }
    }
    if (!jogo->celulaGiganteUsada && (Y_CELULA <= explorador.pes.y) == atras) {
        desenharCelula();
    }
}

/* Painel do posto: três luzes, uma para cada cristal já recolhido.
   Com o posto aberto, a porta fica acesa. */
static void desenharPainelDoPosto(void)
{
    const ProgressoPartida *jogo = progressoPartida();
    int quantos = contarCristaisGigante(jogo->cristaisGigante);
    /* As luzes ficam no painel da parede, à direita da porta. */
    DrawRectangle(X_CENTRO + 535, 203, 8, 20, GetColor(0x2B273DFF));
    for (int i = 0; i < GIGANTE_TOTAL_CRISTAIS; i++) {
        int y = 205 + i * 6;
        if (i < quantos) {
            float pulso = 0.75f + 0.25f * sinf(tempo * 3 + (float)i);
            DrawRectangle(X_CENTRO + 537, y, 4, 4, Fade(GetColor(0xA5DCC4FF), pulso));
        } else {
            DrawRectangle(X_CENTRO + 537, y, 4, 4, GetColor(0x4B3D65FF));
        }
    }
    if (jogo->giganteConcluido) {
        DrawCircleGradient(X_CENTRO + 489, 222, 26, Fade(GetColor(0xA5DCC4FF), 0.28f), Fade(GetColor(0xA5DCC4FF), 0));
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
    int x = (int)(explorador.pes.x - cantoDaCamera()) - largura / 2;
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
    int cristais = contarCristaisGigante(progressoPartida()->cristaisGigante);
    Color corCristais = GetColor(0xA5DCC4FF);
    if (progressoPartida()->giganteConcluido) {
        textoSombra("MAPA OBTIDO", 72, 8, COR_MARCADOR);
    } else {
        textoSombra(TextFormat("CRISTAIS %d/%d", cristais, GIGANTE_TOTAL_CRISTAIS), 72, 8, corCristais);
    }
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
    } else if (cristalPerto() >= 0) {
        dicaAcimaDoJogador("E  Pegar cristal de luz");
    } else if (celulaPerto()) {
        dicaAcimaDoJogador("E  Pegar célula de combustível");
    } else if (perto(X_POSTO, Y_POSTO)) {
        if (progressoPartida()->giganteConcluido) {
            dicaAcimaDoJogador("Posto aberto: mapa já recolhido");
        } else if (postoGiganteAbre(progressoPartida()->cristaisGigante)) {
            dicaAcimaDoJogador("E  Abrir o posto");
        } else {
            dicaAcimaDoJogador("E  Examinar o posto");
        }
    }
    if (tempoAviso > 0) {
        int largura = larguraTexto(aviso) + 16;
        int x = (LARGURA_VIRTUAL - largura) / 2;
        caixaPlaneta(x, 318, largura, 22);
        textoPlaneta(aviso, (float)(x + 8), 325, 8, COR_TITULO);
    }
}

/* O que acabou de ser achado, grande, no meio da tela, com um clarão. */
static void desenharPremio(void)
{
    if (premio == PREMIO_NENHUM) {
        return;
    }
    Texture2D folha = folhaItens;
    Rectangle origem = { 0, 0, 32, 32 };
    Color luz = GetColor(0xA5DCC4FF);
    if (premio == PREMIO_CELULA) {
        origem.x = 32;
        luz = GetColor(0xF2C14EFF);
    } else if (premio == PREMIO_MAPA) {
        folha = folhaMapa;
        origem.x = QUADRO_MAPA * 32;
        luz = COR_MARCADOR;
    }
    float pulso = 0.8f + 0.2f * sinf(tempo * 3);
    int cx = LARGURA_VIRTUAL / 2;
    int cy = 150 + (int)(sinf(tempo * 2) * 3);
    DrawCircleGradient(cx, cy, 70, Fade(luz, 0.40f * pulso), Fade(luz, 0));
    DrawTexturePro(folha, origem, (Rectangle){ (float)cx - 48, (float)cy - 48, 96, 96 }, (Vector2){ 0, 0 }, 0, WHITE);
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
                     "O posto, a leste, pede três cristais de luz.",
                     "E / ENTER  Continuar", false);
    } else if (estado == GIG_MENSAGEM) {
        Falante visitante = falanteVisitante();
        desenharPremio();
        desenharFala(&visitante, textoFala, "E / ENTER  Continuar", false);
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
    Vector2 treme = { 0, 0 };
    if (tremor > 0) {
        treme = (Vector2){ (float)GetRandomValue(-2, 2), (float)GetRandomValue(-1, 1) };
    }

    /* 1. O céu e o rosto do gigante: parados na tela (só deslizam um pouco). */
    Camera2D cameraCeu = { 0 };
    cameraCeu.zoom = 1;
    cameraCeu.offset = treme;
    int desloca = deslocamentoDoCeu();
    BeginMode2D(cameraCeu);
    if (ceu.id != 0) {
        DrawTexture(ceu, desloca - MARGEM_CEU, 0, WHITE);
    }
    desenharOlhos(humor, desloca);
    EndMode2D();

    /* 2. O terreno e tudo o que está nele: correm com a câmera. */
    Camera2D camera = { 0 };
    camera.zoom = 1;
    camera.offset = treme;
    camera.target = (Vector2){ cantoDaCamera(), 0 };
    BeginMode2D(camera);
    if (terreno.id != 0) {
        DrawTexture(terreno, 0, 0, WHITE);
    }
    desenharPainelDoPosto();

    /* As plantas perdem o brilho enquanto o gigante reage. */
    float apagar = 1;
    if (humor == GIGANTE_REACAO) {
        apagar = 0.45f;
    }
    for (int i = 0; i < totalPlantas; i++) {
        desenharPlanta(&plantas[i], apagar);
    }
    desenharItens(true);

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
    desenharItens(false);
    EndMode2D();

    /* 3. Pedras caindo e o clarão vermelho: na frente de tudo, presos à tela. */
    BeginMode2D(cameraCeu);
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
    Texture2D *texturas[] = { &fundo, &ceu, &terreno, &folhaItens, &folhaMapa };
    for (int i = 0; i < 5; i++) {
        if (texturas[i]->id != 0) {
            UnloadTexture(*texturas[i]);
        }
        *texturas[i] = (Texture2D){ 0 };
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
