/* tela_sem_ar.c: planeta sem ar.

   Uma superfície de poeira rosa, maior que a tela: a câmera acompanha
   quem desceu. Não há ar, então o oxigênio do traje cai o tempo todo.
   O objetivo é abrir o portão da estação abandonada, pegar o pedaço do
   mapa e voltar ao módulo de pouso.

   Os itens encostados vão para a mochila, que é uma pilha
   (pilha_mochila.c): só o topo pode ser usado (E) ou largado (Q). O
   portão pede dois itens, um de cada vez, e as regras ficam em
   sem_ar_logica.c. Se o oxigênio acabar, o tripulante não volta.

   O cenário é uma imagem de 1280x720 e o piso vem de uma máscara do
   mesmo tamanho (branco = piso). As duas são geradas por
   ferramentas/gerar_sem_ar.py; as posições daqui são as de lá vezes 2. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "constantes.h"
#include "cenario_planeta.h"
#include "partida.h"
#include "dialogo.h"
#include "animacao.h"
#include "memorial.h"
#include "pilha_mochila.h"
#include "sem_ar_logica.h"
#include "tela_sem_ar.h"

typedef enum {
    SA_AJUDA,        /* explicação, na primeira descida */
    SA_EXPLORANDO,
    SA_RESULTADO,    /* voltou ao módulo: diz se trouxe o mapa */
    SA_SUFOCANDO,    /* o oxigênio acabou: o tripulante cai */
    SA_MEMORIAL      /* tela de despedida */
} EstadoSemAr;

#define ARQUIVO_FUNDO    "assets/imagens/planetas/sem_ar_superficie.png"
#define ARQUIVO_MASCARA  "assets/mascaras/sem_ar_mascara.png"
#define ARQUIVO_ITENS    "assets/imagens/sem_ar_itens.png"
#define ARQUIVO_PLANETA  "assets/imagens/planeta_sem_ar.png"

#define LARGURA_MUNDO    1280
#define ALTURA_MUNDO     720
#define TAMANHO_ITEM     32      /* cada item da folha tem 32x32 */

#define X_INICIO         216     /* ao lado da escada do módulo */
#define Y_INICIO         656
#define X_MODULO         190     /* pé da escada */
#define Y_MODULO         644
#define DISTANCIA_MODULO 56

/* Portão: o vão na parede de baixo da estação. */
#define PORTAO_X         1060
#define PORTAO_Y         220
#define PORTAO_LARGURA   48
#define PORTAO_TOPO      12      /* altura da parte de cima da folha do portão */
#define PORTAO_FRENTE    24      /* altura da parte da frente */
#define X_USAR_PORTAO    1096    /* chão em frente ao portão e ao terminal */
#define Y_USAR_PORTAO    276
#define DISTANCIA_PORTAO 52
#define X_TERMINAL       1136    /* tela do terminal, ao lado do portão */
#define Y_TERMINAL       234

#define TOTAL_RESPIRADOUROS 5
#define TOTAL_ITENS_CHAO    11
#define TEMPO_AVISO         3.0f
#define TEMPO_SUFOCAR       3.4f   /* duração da queda antes do memorial */

typedef struct {
    TipoItem tipo;
    float x, y;        /* ponto no chão */
    bool oculto;       /* só aparece para tripulantes curiosos */
    bool ativo;        /* ainda está no chão */
    bool ignorar;      /* acabou de ser largado: não pega de novo até se afastar */
} ItemChao;

static const ItemChao itensIniciais[TOTAL_ITENS_CHAO] = {
    { ITEM_PLACA,     116, 132, false, true, false },   /* canto noroeste */
    { ITEM_CELULA,   1180, 620, false, true, false },   /* canto sudeste */
    { ITEM_SUCATA,    300, 312, false, true, false },
    { ITEM_SUCATA,   1040, 320, false, true, false },
    { ITEM_ROCHA,    1120, 572, false, true, false },
    { ITEM_CILINDRO,  300, 180, false, true, false },
    { ITEM_CILINDRO,  660, 180, false, true, false },
    { ITEM_CILINDRO,  732, 592, false, true, false },
    { ITEM_CILINDRO,  904, 340, false, true, false },
    { ITEM_CILINDRO, 1212, 360, true,  true, false },
    { ITEM_MAPA,     1180, 190, false, true, false }    /* dentro da estação */
};

static const Vector2 respiradouros[TOTAL_RESPIRADOUROS] = {
    { 324, 352 }, { 932, 480 }, { 804, 320 }, { 500, 524 }, { 1000, 368 }
};

static Texture2D fundo;
static Texture2D folhaItens;
static Animacao planeta;              /* planeta girando, usado no memorial */
static PixelExploracao *pixelsPiso;
static MapaExploracao piso;

static ExploradorPlaneta explorador;
static PilhaMochila mochila;
static ItemChao itens[TOTAL_ITENS_CHAO];
static EstadoPortao portao;
static float aberturaPortao;          /* 0 fechado, 1 aberto */

static bool iniciou = false;
static bool explicou = false;
static EstadoSemAr estado;
static float tempo;                   /* relógio das animações e dos jatos */
static float tempoMissao;             /* segundos na superfície */
static float oxigenio;
static bool noJato;
static bool avisouJato;
static bool trouxeMapa;
static float tempoMorte;
static float tempoAviso;
static char aviso[96];
static char textoFala[200];

/* ---------- Carregamento ---------- */

static void carregarPiso(void)
{
    Image imagem = LoadImage(ARQUIVO_MASCARA);
    if (imagem.data == NULL || imagem.width != LARGURA_MUNDO || imagem.height != ALTURA_MUNDO) {
        TraceLog(LOG_ERROR, "SEM AR: a mascara deve ter 1280x720");
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

void iniciarTelaSemAr(void)
{
    if (iniciou) {
        return;
    }
    fundo = LoadTexture(ARQUIVO_FUNDO);
    if (fundo.id != 0) {
        SetTextureFilter(fundo, TEXTURE_FILTER_POINT);
    }
    folhaItens = LoadTexture(ARQUIVO_ITENS);
    if (folhaItens.id != 0) {
        SetTextureFilter(folhaItens, TEXTURE_FILTER_POINT);
    }
    carregarAnimacao(&planeta, ARQUIVO_PLANETA, TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
    carregarPiso();
    iniciarExploradorPlaneta(&explorador, (PontoExploracao){ X_INICIO, Y_INICIO });
    iniciou = true;
}

void entrarTelaSemAr(void)
{
    if (!iniciou) {
        iniciarTelaSemAr();
    }
    ProgressoPartida *jogo = progressoPartida();
    Tripulante *quem = tripulanteControlado();

    /* O portão continua como ficou na última visita. */
    portao = (EstadoPortao)jogo->etapaPortaoSemAr;
    aberturaPortao = 0;
    if (portao == PORTAO_ABERTO) {
        aberturaPortao = 1;
    }

    iniciarPilha(&mochila);
    for (int i = 0; i < TOTAL_ITENS_CHAO; i++) {
        itens[i] = itensIniciais[i];
        /* Itens já gastos no portão ou já levados para a nave não voltam. */
        if (itens[i].tipo == ITEM_CELULA && portao != PORTAO_SEM_ENERGIA) {
            itens[i].ativo = false;
        }
        if (itens[i].tipo == ITEM_PLACA && portao == PORTAO_ABERTO) {
            itens[i].ativo = false;
        }
        if (itens[i].tipo == ITEM_MAPA && jogo->semArConcluido) {
            itens[i].ativo = false;
        }
    }

    explorador.pes = (PontoExploracao){ X_INICIO, Y_INICIO };
    explorador.direcao = 3;
    explorador.quadro = 0;
    explorador.tempo = 0;
    explorador.correndo = false;

    /* O traje sai da nave com o tanque cheio. */
    oxigenio = OXIGENIO_MAXIMO;
    quem->oxigenio = OXIGENIO_MAXIMO;

    tempo = 0;
    tempoMissao = 0;
    tempoMorte = 0;
    tempoAviso = 0;
    noJato = false;
    avisouJato = false;
    trouxeMapa = false;
    if (explicou) {
        estado = SA_EXPLORANDO;
    } else {
        estado = SA_AJUDA;
    }
}

/* ---------- Funções de apoio ---------- */

static bool confirmou(void)
{
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E);
}

static float distancia(float ax, float ay, float bx, float by)
{
    return hypotf(ax - bx, ay - by);
}

static bool pertoDoModulo(void)
{
    return distancia(explorador.pes.x, explorador.pes.y, X_MODULO, Y_MODULO) <= DISTANCIA_MODULO;
}

static bool pertoDoPortao(void)
{
    return portao != PORTAO_ABERTO &&
           distancia(explorador.pes.x, explorador.pes.y, X_USAR_PORTAO, Y_USAR_PORTAO) <= DISTANCIA_PORTAO;
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

/* Canto de cima da parte do mundo que aparece na tela. A câmera segue
   o tripulante e para nas bordas do mapa. */
static Vector2 cantoDaCamera(void)
{
    Vector2 canto;
    canto.x = floorf(limitar(explorador.pes.x - LARGURA_VIRTUAL / 2, 0, LARGURA_MUNDO - LARGURA_VIRTUAL));
    canto.y = floorf(limitar(explorador.pes.y - 204, 0, ALTURA_MUNDO - ALTURA_VIRTUAL));
    return canto;
}

/* Enquanto o portão não abriu de verdade, o vão bloqueia a passagem. */
static int obstaculoPortao(ObstaculoExploracao *obstaculo)
{
    if (aberturaPortao >= 0.9f) {
        return 0;
    }
    *obstaculo = (ObstaculoExploracao){ PORTAO_X, PORTAO_Y, PORTAO_LARGURA, PORTAO_TOPO + PORTAO_FRENTE };
    return 1;
}

static bool itemVisivel(const ItemChao *item)
{
    if (!item->ativo) {
        return false;
    }
    if (item->oculto) {
        return itemOcultoVisivel(tripulanteControlado()->tracos.curiosidade);
    }
    return true;
}

/* ---------- Lógica ---------- */

static void pegarItensProximos(void)
{
    for (int i = 0; i < TOTAL_ITENS_CHAO; i++) {
        ItemChao *item = &itens[i];
        if (!itemVisivel(item)) {
            continue;
        }
        float d = distancia(explorador.pes.x, explorador.pes.y - 4, item->x, item->y);
        if (item->ignorar) {
            /* Largado ou recusado: só volta a valer depois de se afastar. */
            if (d > SEM_AR_RAIO_ITEM * 2) {
                item->ignorar = false;
            }
            continue;
        }
        if (d > SEM_AR_RAIO_ITEM) {
            continue;
        }
        ResultadoPegar resultado = pegarItem(&mochila, item->tipo, &oxigenio);
        if (resultado == PEGAR_CHEIA) {
            item->ignorar = true;
            avisar("Mochila cheia. Q larga o item do topo.");
        } else if (resultado == PEGAR_RESPIROU) {
            item->ativo = false;
            avisar(TextFormat("Oxigênio recarregado: +%d.", SEM_AR_RECARGA_CILINDRO));
        } else {
            item->ativo = false;
            avisar(TextFormat("Pegou: %s. Agora é o topo da mochila.", nomeItem(item->tipo)));
        }
        return;   /* um item por vez */
    }
}

/* Q: tira o topo da mochila e deixa no chão, aos pés do tripulante. */
static void largarTopo(void)
{
    TipoItem tipo;
    if (!desempilhar(&mochila, &tipo)) {
        avisar("A mochila está vazia.");
        return;
    }
    /* Reaproveita uma casa livre do vetor de itens do chão. */
    for (int i = 0; i < TOTAL_ITENS_CHAO; i++) {
        if (itens[i].ativo) {
            continue;
        }
        itens[i] = (ItemChao){ tipo, explorador.pes.x, explorador.pes.y - 2, false, true, true };
        break;
    }
    avisar(TextFormat("Largou: %s.", nomeItem(tipo)));
}

static void usarPortao(void)
{
    TipoItem pedido = itemPedidoPortao(portao);
    TipoItem topo = TOTAL_ITENS;
    topoDaPilha(&mochila, &topo);
    ResultadoPortao resultado = usarTopoNoPortao(&portao, &mochila);

    if (resultado == USO_LIGOU) {
        avisar("A célula encaixou. O terminal acendeu e pede a placa.");
    } else if (resultado == USO_ABRIU) {
        avisar("Placa aceita. O portão está abrindo.");
    } else if (resultado == USO_ITEM_ERRADO) {
        avisar(TextFormat("O terminal pede: %s. No topo: %s.", nomeItem(pedido), nomeItem(topo)));
    } else if (resultado == USO_MOCHILA_VAZIA) {
        avisar(TextFormat("O terminal pede: %s.", nomeItem(pedido)));
    }
    progressoPartida()->etapaPortaoSemAr = (int)portao;
}

static void voltarAoModulo(void)
{
    Tripulante *quem = tripulanteControlado();
    trouxeMapa = pilhaContem(&mochila, ITEM_MAPA);
    if (trouxeMapa) {
        progressoPartida()->semArConcluido = true;
        snprintf(textoFala, sizeof textoFala,
                 "O pedaço do mapa está guardado.\n%s sobe a escada e o módulo decola.", quem->nome);
    } else if (progressoPartida()->semArConcluido) {
        snprintf(textoFala, sizeof textoFala,
                 "O pedaço do mapa já está na nave.\n%s sobe a escada e o módulo decola.", quem->nome);
    } else {
        snprintf(textoFala, sizeof textoFala,
                 "%s volta sem o pedaço do mapa.\nA estação continua lá, do jeito que ficou.", quem->nome);
    }
    quem->oxigenio = OXIGENIO_MAXIMO;   /* o tanque é enchido de novo na nave */
    estado = SA_RESULTADO;
}

static void comecarASufocar(void)
{
    Tripulante *quem = tripulanteControlado();
    quem->oxigenio = 0;
    quem->vivo = false;      /* a nave passa o controle a outro (entrarTelaNave) */
    tempoMorte = 0;
    tempoAviso = 0;
    estado = SA_SUFOCANDO;
}

static void abrirDespedida(void)
{
    const Tripulante *quem = tripulanteControlado();
    DadosMemorial dados = { 0 };
    dados.nome = quem->nome;
    dados.idade = quem->idade;
    dados.causa = "Ficou sem oxigênio no planeta sem ar.";
    dados.destaque = destaqueTripulante(&quem->tracos);

    int segundos = (int)tempoMissao;
    snprintf(dados.linhas[0], MEMORIAL_TAM_LINHA, "Tempo na superfície: %d min %02d s",
             segundos / 60, segundos % 60);
    if (pilhaContem(&mochila, ITEM_MAPA)) {
        snprintf(dados.linhas[1], MEMORIAL_TAM_LINHA, "O pedaço do mapa ficou para trás.");
    } else if (mochila.quantidade > 0) {
        snprintf(dados.linhas[1], MEMORIAL_TAM_LINHA, "Itens perdidos com a mochila: %d", mochila.quantidade);
    } else {
        snprintf(dados.linhas[1], MEMORIAL_TAM_LINHA, "A mochila estava vazia.");
    }
    int vivos = tripulantesVivos(progressoPartida()->tripulacao);
    if (vivos == 0) {
        snprintf(dados.linhas[2], MEMORIAL_TAM_LINHA, "Ninguém restou a bordo.");
    } else if (vivos == 1) {
        snprintf(dados.linhas[2], MEMORIAL_TAM_LINHA, "Resta 1 tripulante a bordo.");
    } else {
        snprintf(dados.linhas[2], MEMORIAL_TAM_LINHA, "Restam %d tripulantes a bordo.", vivos);
    }
    dados.totalLinhas = 3;
    abrirMemorial(&dados);
    estado = SA_MEMORIAL;
}

static void atualizarExploracao(float dt)
{
    ObstaculoExploracao obstaculo;
    int quantidade = obstaculoPortao(&obstaculo);
    bool moveu = atualizarExploradorPlaneta(&explorador, &piso, &obstaculo, quantidade, dt, true);
    tempoMissao += dt;

    /* Está dentro de algum jato de gás? */
    noJato = false;
    for (int i = 0; i < TOTAL_RESPIRADOUROS; i++) {
        if (faseJato(tempo, (float)i * 1.3f) != JATO_ATIVO) {
            continue;
        }
        if (distancia(explorador.pes.x, explorador.pes.y, respiradouros[i].x, respiradouros[i].y)
            < SEM_AR_RAIO_JATO) {
            noJato = true;
        }
    }
    if (noJato && !avisouJato) {
        avisouJato = true;
        avisar("O jato de gás faz o traje vazar. Saia de perto!");
    }

    oxigenio = gastarOxigenio(oxigenio, dt, moveu && explorador.correndo, noJato);
    tripulanteControlado()->oxigenio = (int)ceilf(oxigenio);
    if (oxigenio <= 0) {
        comecarASufocar();
        return;
    }

    pegarItensProximos();
    if (IsKeyPressed(KEY_Q)) {
        largarTopo();
    }
    if (IsKeyPressed(KEY_E)) {
        if (pertoDoPortao()) {
            usarPortao();
        } else if (pertoDoModulo()) {
            voltarAoModulo();
        }
    }
}

Tela atualizarTelaSemAr(void)
{
    float dt = GetFrameTime();
    if (!isfinite(dt) || dt < 0) {
        dt = 0;
    }
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    tempo += dt;
    if (tempoAviso > 0) {
        tempoAviso -= dt;
    }
    if (portao == PORTAO_ABERTO && aberturaPortao < 1) {
        aberturaPortao += dt / SEM_AR_TEMPO_PORTAO;
        if (aberturaPortao > 1) {
            aberturaPortao = 1;
        }
    }

    if (estado == SA_AJUDA) {
        if (confirmou()) {
            explicou = true;
            estado = SA_EXPLORANDO;
            avisar("O oxigênio do traje já está caindo.");
        }
        return TELA_SEM_AR;
    }
    if (estado == SA_RESULTADO) {
        if (confirmou()) {
            return TELA_NAVE;
        }
        return TELA_SEM_AR;
    }
    if (estado == SA_SUFOCANDO) {
        tempoMorte += dt;
        if (tempoMorte >= TEMPO_SUFOCAR) {
            abrirDespedida();
        }
        return TELA_SEM_AR;
    }
    if (estado == SA_MEMORIAL) {
        atualizarAnimacao(&planeta);
        if (atualizarMemorial(dt)) {
            return TELA_NAVE;
        }
        return TELA_SEM_AR;
    }

    atualizarExploracao(dt);
    return TELA_SEM_AR;
}

/* ---------- Desenho do mundo (dentro da câmera) ---------- */

/* Retângulo alinhado à grade de 2 px do cenário, para os efeitos terem
   o mesmo tamanho de pixel do fundo. */
static void bloco2(float x, float y, int largura, int altura, Color cor)
{
    DrawRectangle((int)floorf(x / 2) * 2, (int)floorf(y / 2) * 2, largura, altura, cor);
}

/* Luzes do módulo de pouso: o sinal da antena, as luzes de posição e
   um reflexo que desce pela janela. */
static void desenharLuzesDoModulo(void)
{
    if (fmodf(tempo, 1.2f) < 0.18f) {
        DrawRectangle(144, 502, 6, 6, GetColor(0xFF6A5AFF));
        DrawCircle(147, 505, 9, Fade(GetColor(0xFF6A5AFF), 0.22f));
    }
    int acesa = (int)(tempo * 2.5f) % 4;
    for (int i = 0; i < 3; i++) {
        if (i == acesa) {
            DrawRectangle(144 + i * 24, 602, 4, 4, GetColor(0xFFE08AFF));
        }
    }
    float varredura = fmodf(tempo, 3.0f) / 3.0f;
    if (varredura < 0.5f) {
        DrawRectangle(138, 546 + (int)(varredura * 2 * 22) / 2 * 2, 34, 2, Fade(GetColor(0xB9F5DEFF), 0.3f));
    }
}

static void desenharRespiradouros(void)
{
    Color gas = GetColor(0xDDF58CFF);
    Color gasEscuro = GetColor(0x9CC25AFF);
    for (int i = 0; i < TOTAL_RESPIRADOUROS; i++) {
        float x = respiradouros[i].x;
        float y = respiradouros[i].y;
        FaseJato fase = faseJato(tempo, (float)i * 1.3f);
        if (fase == JATO_AVISO) {
            /* Borbulhas pequenas avisam que o jato vem aí. */
            for (int k = 0; k < 3; k++) {
                float sobe = fmodf(tempo * 3 + (float)k * 0.33f, 1.0f);
                bloco2(x - 6 + (float)k * 6, y - 2 - sobe * 10, 2, 2, Fade(gas, 0.8f - sobe * 0.6f));
            }
        } else if (fase == JATO_ATIVO) {
            /* Nuvens que sobem, crescem e somem. */
            for (int k = 0; k < 9; k++) {
                float sobe = fmodf(tempo * 1.7f + (float)k / 9.0f, 1.0f);
                int lado = 4 + (int)(sobe * 7) * 2;
                float desvio = sinf((float)k * 2.1f + tempo * 3) * 8 * sobe;
                float px = x + desvio - (float)lado / 2;
                float py = y - sobe * 56 - (float)lado / 2;
                if (sobe > 0.85f) {
                    continue;   /* some de uma vez no alto, sem ficar quase invisível */
                }
                bloco2(px + 2, py + 2, lado, lado, gasEscuro);
                bloco2(px, py, lado, lado, Fade(gas, 1 - sobe * 0.3f));
            }
            /* O chão em volta mostra até onde o gás alcança. */
            DrawEllipse((int)x, (int)y, SEM_AR_RAIO_JATO, SEM_AR_RAIO_JATO * 0.45f, Fade(gas, 0.28f));
        }
    }
}

/* Folhas do portão: duas metades que correm para dentro das paredes. */
static void desenharPortao(void)
{
    int metade = PORTAO_LARGURA / 2;
    int visivel = (int)((float)metade * (1 - aberturaPortao)) / 2 * 2;
    Color luz = GetColor(0x7A2A20FF);                  /* sem energia: vermelho apagado */
    if (portao == PORTAO_PEDE_PLACA) {
        luz = COR_MARCADOR;
    } else if (portao == PORTAO_ABERTO) {
        luz = GetColor(0x68D8CDFF);
    }
    for (int lado = 0; lado < 2; lado++) {
        if (visivel <= 0) {
            break;
        }
        int x = PORTAO_X;
        if (lado == 1) {
            x = PORTAO_X + PORTAO_LARGURA - visivel;
        }
        int y = PORTAO_Y;
        DrawRectangle(x, y, visivel, PORTAO_TOPO, GetColor(0x8A84A0FF));
        DrawRectangle(x, y, visivel, 2, GetColor(0xC4BFD6FF));
        DrawRectangle(x, y + PORTAO_TOPO, visivel, PORTAO_FRENTE, GetColor(0x2B2142FF));
        DrawRectangle(x, y + PORTAO_TOPO + 2, visivel, PORTAO_FRENTE - 4, GetColor(0x4A4560FF));
        DrawRectangle(x, y + PORTAO_TOPO + 4, visivel, 4, luz);
        /* Faixa de aviso: os blocos ficam presos ao chão, então parecem
           correr quando a folha se move. */
        for (int bx = x; bx < x + visivel; bx += 2) {
            if ((bx / 4) % 2 == 0) {
                DrawRectangle(bx, y + PORTAO_TOPO + 14, 2, 6, COR_MARCADOR);
            }
        }
    }
    if (visivel > 0) {
        /* Fresta entre as duas folhas. */
        DrawRectangle(PORTAO_X + visivel - 2, PORTAO_Y, 2, PORTAO_TOPO + PORTAO_FRENTE, GetColor(0x1C1826FF));
    }
}

/* Tela do terminal: apagada, pedindo a placa ou liberada. */
static void desenharTerminal(void)
{
    int x = X_TERMINAL, y = Y_TERMINAL;
    if (portao == PORTAO_SEM_ENERGIA) {
        if (fmodf(tempo, 1.0f) < 0.5f) {
            DrawRectangle(x + 12, y + 2, 2, 2, GetColor(0xEE9A8AFF));
        }
        DrawRectangleLines(x + 2, y + 4, 8, 6, GetColor(0x4A4560FF));   /* desenho de uma pilha vazia */
        DrawRectangle(x + 10, y + 6, 2, 2, GetColor(0x4A4560FF));
    } else if (portao == PORTAO_PEDE_PLACA) {
        float pulso = 0.75f + 0.25f * sinf(tempo * 5);
        DrawRectangle(x, y, 16, 12, Fade(COR_MARCADOR, pulso));
        DrawRectangle(x + 2, y + 2, 12, 8, GetColor(0x2B2142FF));       /* desenho da placa */
        DrawRectangle(x + 4, y + 4, 6, 2, COR_MARCADOR);
    } else {
        DrawRectangle(x, y, 16, 12, GetColor(0x68D8CDFF));
        DrawRectangle(x + 2, y + 6, 2, 2, GetColor(0x1C1826FF));        /* um "certo" */
        DrawRectangle(x + 4, y + 8, 2, 2, GetColor(0x1C1826FF));
        DrawRectangle(x + 6, y + 6, 2, 2, GetColor(0x1C1826FF));
        DrawRectangle(x + 8, y + 4, 2, 2, GetColor(0x1C1826FF));
        DrawRectangle(x + 10, y + 2, 2, 2, GetColor(0x1C1826FF));
    }

    /* Balão acima do terminal com o item que ele pede. */
    TipoItem pedido = itemPedidoPortao(portao);
    if (pedido != TOTAL_ITENS && folhaItens.id != 0) {
        int bx = x - 3, by = y - 44 + (int)(sinf(tempo * 2.5f) * 2);
        DrawRectangle(bx, by, 22, 22, GetColor(0x211C2EFF));
        DrawRectangleLines(bx, by, 22, 22, GetColor(0x8C77ADFF));
        DrawRectangle(bx + 9, by + 22, 4, 2, GetColor(0x8C77ADFF));
        Rectangle origem = { (float)(pedido * TAMANHO_ITEM), 0, TAMANHO_ITEM, TAMANHO_ITEM };
        Rectangle destino = { (float)bx + 3, (float)by + 3, 16, 16 };
        DrawTexturePro(folhaItens, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
    }
}

static void desenharItensDoChao(void)
{
    for (int i = 0; i < TOTAL_ITENS_CHAO; i++) {
        const ItemChao *item = &itens[i];
        if (!itemVisivel(item)) {
            continue;
        }
        int x = (int)item->x;
        int y = (int)item->y;
        /* Sobe e desce de leve, para se destacar do chão. */
        int flutua = (int)(sinf(tempo * 3 + (float)i) * 2);
        DrawEllipse(x, y, 9, 3, Fade(BLACK, 0.3f));
        if (item->tipo == ITEM_MAPA) {
            float pulso = 0.25f + 0.15f * sinf(tempo * 3);
            DrawCircle(x, y - 14, 20, Fade(GetColor(0x68D8CDFF), pulso));
        }
        if (folhaItens.id != 0) {
            DrawTextureRec(folhaItens, (Rectangle){ (float)(item->tipo * TAMANHO_ITEM), 0, TAMANHO_ITEM, TAMANHO_ITEM },
                           (Vector2){ (float)(x - TAMANHO_ITEM / 2), (float)(y - 28 + flutua) }, WHITE);
        } else {
            DrawRectangle(x - 6, y - 16 + flutua, 12, 12, COR_MARCADOR);
        }
        /* Brilho que pisca: nos cilindros (ajuda a achar) e no item escondido. */
        if (item->tipo == ITEM_CILINDRO || item->oculto) {
            if (fmodf(tempo + (float)i * 0.37f, 1.4f) < 0.2f) {
                DrawRectangle(x + 8, y - 28 + flutua, 2, 6, WHITE);
                DrawRectangle(x + 6, y - 26 + flutua, 6, 2, WHITE);
            }
        }
    }
}

/* O tripulante sem ar: treme, cai de lado e levanta poeira. */
static void desenharQueda(void)
{
    float x = floorf(explorador.pes.x);
    float y = floorf(explorador.pes.y);
    float angulo = 0;
    if (tempoMorte < 0.9f) {
        x += (float)((int)(tempoMorte * 30) % 2) * 2 - 1;   /* tremor */
    } else {
        angulo = limitar((tempoMorte - 0.9f) / 0.5f, 0, 1) * 90;
        if (explorador.direcao == 2) {
            angulo = -angulo;                                /* cai para o lado em que olhava */
        }
    }
    DrawEllipse((int)x, (int)y - 3, 16, 4, Fade(BLACK, 0.4f));
    if (explorador.sprite.id != 0) {
        Rectangle origem = { 0, (float)(explorador.direcao * ALTURA_PERSONAGEM),
                             LARGURA_PERSONAGEM, ALTURA_PERSONAGEM };
        Rectangle destino = { x, y, LARGURA_PERSONAGEM, ALTURA_PERSONAGEM };
        Vector2 pes = { LARGURA_PERSONAGEM / 2.0f, ALTURA_PERSONAGEM };
        DrawTexturePro(explorador.sprite, origem, destino, pes, angulo, WHITE);
    }
    /* Poeira subindo quando o corpo toca o chão. */
    float poeira = tempoMorte - 1.4f;
    if (poeira > 0 && poeira < 1.2f) {
        for (int k = 0; k < 8; k++) {
            float lado = (float)(k % 2 * 2 - 1);
            float px = x + lado * (10 + poeira * 30 + (float)k * 3);
            float py = y - 4 - poeira * (6 + (float)(k % 3) * 5);
            bloco2(px, py, 4, 4, Fade(GetColor(0xF0C6D3FF), (1.2f - poeira) * 0.6f));
        }
    }
}

/* ---------- Interface (fora da câmera) ---------- */

/* Largura do texto na fonte de 8 px. Conta letras, não bytes. */
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

static const char *objetivoAtual(void)
{
    if (pilhaContem(&mochila, ITEM_MAPA)) {
        return "volte ao módulo de pouso com o pedaço do mapa";
    }
    if (progressoPartida()->semArConcluido) {
        return "o mapa já está na nave; volte ao módulo de pouso";
    }
    if (portao == PORTAO_ABERTO) {
        return "pegue o pedaço do mapa dentro da estação";
    }
    if (portao == PORTAO_PEDE_PLACA) {
        return "leve a placa de acesso ao terminal do portão";
    }
    return "ligue o terminal do portão com a célula de energia";
}

static void desenharMochila(void)
{
    int x = 454, y = 250, largura = 170, altura = 90;
    DrawRectangle(x, y, largura, altura, Fade(GetColor(0x211C2EFF), 0.88f));
    DrawRectangleLines(x, y, largura, altura, GetColor(0x8C77ADFF));
    textoPlaneta(TextFormat("MOCHILA %d/%d", mochila.quantidade, MOCHILA_CAPACIDADE),
                 (float)x + 8, (float)y + 7, 8, COR_SUBTITULO);
    textoPlaneta("TOPO", (float)(x + largura - 40), (float)y + 7, 8, COR_MARCADOR);

    /* Uma linha por casa da pilha; a de cima é o topo. */
    for (int i = 0; i < MOCHILA_CAPACIDADE; i++) {
        int linha = y + 22 + i * 16;
        DrawRectangleLines(x + 6, linha, 16, 16, GetColor(0x514464FF));
        if (i >= mochila.quantidade) {
            continue;
        }
        TipoItem tipo = itemNaPosicao(&mochila, i);
        Color cor = COR_MENU;
        if (i == 0) {
            cor = COR_MARCADOR;
            DrawRectangle(x + 5, linha - 1, largura - 10, 18, Fade(COR_MARCADOR, 0.12f));
        }
        if (folhaItens.id != 0) {
            Rectangle origem = { (float)(tipo * TAMANHO_ITEM), 0, TAMANHO_ITEM, TAMANHO_ITEM };
            Rectangle destino = { (float)x + 6, (float)linha, 16, 16 };
            DrawTexturePro(folhaItens, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
        }
        textoPlaneta(nomeItem(tipo), (float)x + 28, (float)linha + 4, 8, cor);
    }
}

/* Quando o módulo sai da tela, um marcador na borda mostra para que lado ele está. */
static void desenharSetaDoModulo(Vector2 canto)
{
    float x = X_MODULO - 22 - canto.x;
    float y = Y_MODULO - 70 - canto.y;
    if (x >= 0 && x < LARGURA_VIRTUAL && y >= 36 && y < ALTURA_VIRTUAL - 16) {
        return;   /* está visível: não precisa de seta */
    }
    int sx = (int)limitar(x, 12, LARGURA_VIRTUAL - 12);
    int sy = (int)limitar(y, 48, ALTURA_VIRTUAL - 30);
    int pulso = (int)(fmodf(tempo, 0.8f) * 5);
    DrawRectangle(sx - 5, sy - 5, 10, 10, GetColor(0x211C2EFF));
    DrawRectangle(sx - 3, sy - 3, 6, 6, COR_MARCADOR);
    DrawRectangleLines(sx - 6 - pulso, sy - 6 - pulso, 12 + pulso * 2, 12 + pulso * 2, Fade(COR_MARCADOR, 0.5f));
    int tx = sx + 10;
    if (sx > LARGURA_VIRTUAL / 2) {
        tx = sx - 10 - 40;
    }
    textoPlaneta("POUSO", (float)tx + 1, (float)sy - 3, 8, BLACK);
    textoPlaneta("POUSO", (float)tx, (float)sy - 4, 8, COR_MARCADOR);
}

/* Dica de tecla perto do tripulante: acima da cabeça ou, se abaixo for
   true, sob os pés (para não cobrir o que está acima dele). */
static void dicaDoJogador(const char *s, Vector2 canto, bool abaixo)
{
    int largura = larguraTexto(s) + 16;
    int x = (int)(explorador.pes.x - canto.x) - largura / 2;
    int y = (int)(explorador.pes.y - canto.y) - 96;
    if (abaixo) {
        y = (int)(explorador.pes.y - canto.y) + 8;
    }
    x = (int)limitar((float)x, 8, (float)(LARGURA_VIRTUAL - 8 - largura));
    if (y < 40) {
        y = 40;
    }
    caixaPlaneta(x, y, largura, 22);
    textoPlaneta(s, (float)(x + 8), (float)(y + 7), 8, COR_MARCADOR);
}

static void desenharInterface(Vector2 canto)
{
    bool baixo = oxigenio <= SEM_AR_OXIGENIO_BAIXO;

    /* Oxigênio baixo: a tela pulsa em vermelho pelas bordas. */
    if (baixo && estado == SA_EXPLORANDO) {
        float pulso = 0.10f + 0.10f * (0.5f + 0.5f * sinf(tempo * 8));
        Color alerta = Fade(COR_ALERTA, pulso);
        DrawRectangle(0, 34, LARGURA_VIRTUAL, 10, alerta);
        DrawRectangle(0, ALTURA_VIRTUAL - 24, LARGURA_VIRTUAL, 10, alerta);
        DrawRectangle(0, 44, 10, ALTURA_VIRTUAL - 68, alerta);
        DrawRectangle(LARGURA_VIRTUAL - 10, 44, 10, ALTURA_VIRTUAL - 68, alerta);
        DrawRectangle(0, 34, LARGURA_VIRTUAL, ALTURA_VIRTUAL - 48, Fade(COR_ALERTA, pulso * 0.4f));
    }

    /* Faixa de cima: nome, oxigênio e objetivo. */
    DrawRectangle(0, 0, LARGURA_VIRTUAL, 34, Fade(GetColor(0x17131FFF), 0.86f));
    textoPlaneta(tripulanteControlado()->nome, 16, 7, 8, COR_TITULO);
    textoPlaneta("OXIGÊNIO", 72, 7, 8, COR_TITULO);
    Color corBarra = GetColor(0x68D8CDFF);
    if (noJato || (baixo && fmodf(tempo, 0.5f) < 0.3f)) {
        corBarra = COR_ALERTA;
    }
    DrawRectangle(146, 7, 124, 8, GetColor(0x40334FFF));
    DrawRectangle(146, 7, (int)(124 * oxigenio / OXIGENIO_MAXIMO), 8, corBarra);
    DrawRectangleLines(145, 6, 126, 10, GetColor(0x8C77ADFF));
    textoPlaneta(TextFormat("%d", (int)ceilf(oxigenio)), 278, 7, 8, corBarra);
    if (noJato) {
        textoPlaneta("VAZANDO!", 312, 7, 8, COR_ALERTA);
    } else if (baixo) {
        textoPlaneta("BAIXO", 312, 7, 8, COR_ALERTA);
    } else if (explorador.correndo) {
        textoPlaneta("correndo: gasta mais", 312, 7, 8, COR_MENU);
    }
    textoPlaneta("OBJETIVO", 16, 21, 8, COR_MARCADOR);
    textoPlaneta(objetivoAtual(), 88, 21, 8, COR_MENU);

    DrawRectangle(0, 346, LARGURA_VIRTUAL, 14, GetColor(0x17131FFF));
    textoPlaneta("WASD/SETAS Mover   SHIFT Correr   E Usar   Q Largar o topo", 16, 350, 8, COR_MENU);

    desenharMochila();
    if (estado != SA_EXPLORANDO) {
        return;
    }
    desenharSetaDoModulo(canto);
    if (pertoDoPortao()) {
        dicaDoJogador("E  Usar o topo da mochila", canto, true);
    } else if (pertoDoModulo()) {
        dicaDoJogador("E  Voltar à nave", canto, false);
    }
    if (tempoAviso > 0) {
        int largura = larguraTexto(aviso) + 16;
        int x = (LARGURA_VIRTUAL - largura) / 2;
        caixaPlaneta(x, 42, largura, 22);
        textoPlaneta(aviso, (float)(x + 8), 49, 8, COR_TITULO);
    }
}

static Falante falanteVisitante(void)
{
    Falante f = { tripulanteControlado()->nome, explorador.sprite, (Rectangle){ 4, 0, 36, 36 } };
    return f;
}

/* O fim: o mundo escurece num círculo que se fecha em volta do tripulante. */
static void desenharEscurecer(Vector2 canto)
{
    Vector2 centro = { explorador.pes.x - canto.x, explorador.pes.y - canto.y - 20 };
    float fecha = limitar((tempoMorte - 1.2f) / 1.9f, 0, 1);
    float raio = 420 * (1 - fecha);
    if (tempoMorte > 1.2f) {
        DrawRing(centro, raio, 1200, 0, 360, 64, BLACK);
    }
    /* Vermelho piscando enquanto ainda está de pé. */
    if (tempoMorte < 1.2f && fmodf(tempoMorte, 0.3f) < 0.15f) {
        DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(COR_ALERTA, 0.22f));
    }
    if (fmodf(tempoMorte, 0.6f) < 0.4f) {
        const char *texto = "OXIGÊNIO ESGOTADO";
        int x = (LARGURA_VIRTUAL - larguraTexto(texto) * 2) / 2;
        textoPlaneta(texto, (float)x + 2, 62, 16, BLACK);
        textoPlaneta(texto, (float)x, 60, 16, COR_ALERTA);
    }
}

void desenharTelaSemAr(void)
{
    if (!iniciou) {
        return;
    }
    if (estado == SA_MEMORIAL) {
        desenharMemorial(explorador.sprite, &planeta);
        return;
    }

    ClearBackground(GetColor(0x15121CFF));
    Vector2 canto = cantoDaCamera();
    Camera2D camera = { 0 };
    camera.target = canto;
    camera.zoom = 1;

    BeginMode2D(camera);
    if (fundo.id != 0) {
        DrawTexture(fundo, 0, 0, WHITE);
    }
    desenharLuzesDoModulo();
    desenharTerminal();
    desenharItensDoChao();
    /* O portão fica na frente de quem está dentro da estação e atrás
       de quem está do lado de fora. */
    bool dentro = explorador.pes.y < PORTAO_Y + PORTAO_TOPO + PORTAO_FRENTE;
    if (!dentro) {
        desenharPortao();
    }
    if (estado == SA_SUFOCANDO) {
        desenharQueda();
    } else {
        desenharExploradorPlaneta(&explorador);
    }
    if (dentro) {
        desenharPortao();
    }
    desenharRespiradouros();
    EndMode2D();

    if (estado == SA_SUFOCANDO) {
        desenharEscurecer(canto);
        return;
    }
    desenharInterface(canto);
    if (estado == SA_AJUDA) {
        Falante visitante = falanteVisitante();
        desenharFala(&visitante,
                     "Aqui não há ar: o oxigênio do traje cai o tempo todo.\n"
                     "A mochila é uma pilha: só dá para usar (E) ou largar (Q)\n"
                     "o item do topo, o último que entrou.\n"
                     "Abra o portão da estação e traga o pedaço do mapa.",
                     "E / ENTER  Continuar", false);
    } else if (estado == SA_RESULTADO) {
        Falante visitante = falanteVisitante();
        desenharFala(&visitante, textoFala, "E / ENTER  Voltar à nave", false);
    }
}

void encerrarTelaSemAr(void)
{
    if (!iniciou) {
        return;
    }
    if (fundo.id != 0) {
        UnloadTexture(fundo);
    }
    if (folhaItens.id != 0) {
        UnloadTexture(folhaItens);
    }
    descarregarAnimacao(&planeta);
    free(pixelsPiso);
    pixelsPiso = NULL;
    piso = (MapaExploracao){ 0 };
    encerrarExploradorPlaneta(&explorador);
    iniciou = false;
}
