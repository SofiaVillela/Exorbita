/* tela_extraterrestre.c: planeta extraterrestre.

   Uma cidade à noite, maior que a tela: duas ruas, um beco, uma praça
   com holograma e o pátio onde o módulo pousa. A câmera acompanha quem
   desceu. Na rua de baixo há um morador; na rua de cima, na porta do
   bar, está o viajante, que carrega um pedaço do mapa estelar.

   O viajante só aposta o mapa numa partida de Blefe Orbital (as regras
   ficam em cartas.c). Com ou sem tradutor, ele chama para dentro do bar,
   onde fica a mesa de jogo. Vale uma partida por pouso: quem perde só
   tenta de novo pousando outra vez.

   No canto do bar há um fliperama com um jogo de naves (fliperama.c).

   Sem o tradutor (tradutor.c), a fala dos alienígenas vira símbolos.

   A rua é uma imagem de 1280x720 e o bar, uma de 640x360. O piso de
   cada um vem de uma máscara do mesmo tamanho (branco = piso). Tudo é
   gerado por ferramentas/gerar_extraterrestre.py; as posições daqui são
   as de lá vezes 2. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "animacao.h"
#include "cartas.h"
#include "cenario_planeta.h"
#include "constantes.h"
#include "dialogo.h"
#include "fliperama.h"
#include "partida.h"
#include "pilha_mochila.h"
#include "recursos.h"
#include "sons.h"
#include "tradutor.h"
#include "tela_extraterrestre.h"

typedef enum { LUGAR_RUA, LUGAR_BAR } Lugar;

typedef enum {
    ET_ANDANDO,
    ET_FALA,        /* alguém falando, uma página por vez */
    ET_PERGUNTA,    /* lista de opções: o que dizer ou o convite */
    ET_REGRAS,
    ET_CARTAS,
    ET_TROCA,       /* a tela escurece ao entrar ou sair do bar */
    ET_FLIPERAMA    /* jogando no fliperama do bar */
} ModoET;

typedef enum {
    ALVO_NENHUM,
    ALVO_MODULO, ALVO_MORADOR, ALVO_VIAJANTE, ALVO_PORTA_BAR,     /* na rua */
    ALVO_MESA, ALVO_BALCAO, ALVO_FREGUES, ALVO_SAIDA, ALVO_FLIPERAMA   /* no bar */
} Alvo;

/* Quem fala. Os quatro primeiros são também a linha de cada um na
   folha de alienígenas. */
typedef enum { QUEM_VIAJANTE, QUEM_MORADOR, QUEM_BALCAO, QUEM_FREGUES, QUEM_TRIPULANTE } Quem;

typedef struct {
    Quem quem;
    const char *texto;
} Pagina;

/* O que acontece quando a conversa acaba. */
typedef enum { DEPOIS_NADA, DEPOIS_PERGUNTAR_AO_MORADOR, DEPOIS_CONVITE } Depois;
typedef enum { PERGUNTA_MORADOR, PERGUNTA_CONVITE } TipoPergunta;

#define ARQUIVO_RUA         "assets/imagens/planetas/et_rua.png"
#define ARQUIVO_RUA_PISO    "assets/mascaras/et_rua_mascara.png"
#define ARQUIVO_BAR         "assets/imagens/planetas/et_bar.png"
#define ARQUIVO_BAR_PISO    "assets/mascaras/et_bar_mascara.png"
#define ARQUIVO_BAR_FRENTE  "assets/imagens/planetas/et_bar_frente.png"
#define ARQUIVO_MESA        "assets/imagens/planetas/et_mesa.png"
#define ARQUIVO_ALIENS      "assets/imagens/et_aliens.png"
#define ARQUIVO_POSTE       "assets/imagens/et_objetos.png"
#define ARQUIVO_CARTAS      "assets/imagens/et_cartas.png"
#define ARQUIVO_ITENS       "assets/imagens/sem_ar_itens.png"   /* de onde vem o desenho do pedaço do mapa */
#define ARQUIVO_PLANETA     "assets/imagens/planeta_extraterrestre.png"

#define LARGURA_RUA      1280
#define ALTURA_RUA       720
#define X_INICIO         216     /* ao lado da escada do módulo */
#define Y_INICIO         668
#define X_MODULO         190     /* pé da escada */
#define Y_MODULO         656
#define DISTANCIA_MODULO 56
#define X_PORTA_BAR      1096    /* chão em frente à porta, na rua de cima */
#define Y_PORTA_BAR      184
#define X_HOLOGRAMA      678     /* centro da base do holograma */
#define Y_HOLOGRAMA      336

#define X_ENTRADA_BAR    320     /* onde se aparece ao entrar no bar */
#define Y_ENTRADA_BAR    314
#define Y_SAIDA_BAR      334     /* passar daqui para baixo é sair pela porta */
#define X_FLIPERAMA      564     /* chão em frente ao fliperama */
#define Y_FLIPERAMA      152
#define X_MESA           472     /* centro da mesa de cartas */
#define Y_MESA           248

#define TOTAL_POSTES     6
#define LARGURA_POSTE    24
#define ALTURA_POSTE     72
#define LARGURA_CARTA    76
#define ALTURA_CARTA     104
#define QUADRO_VERSO     0       /* na folha: verso, valores de 1 a 9 e lugar vazio */
#define QUADRO_VAZIO     10
#define TAMANHO_ITEM     32

#define TEMPO_TROCA      0.5f    /* escurece na primeira metade, clareia na segunda */
#define TEMPO_AVISO      3.0f
#define TAM_FALA         400

#define COR_CIANO        GetColor(0x68D8CDFF)
#define COR_PAINEL       GetColor(0x211C2EFF)
#define COR_BORDA        GetColor(0x8C77ADFF)

/* Um alienígena parado no cenário. */
typedef struct {
    Quem quem;
    float x, y;            /* pés */
    bool viraParaOlhar;    /* vira o rosto para quem chega perto */
    Rectangle movel;       /* parte da camada de móveis desenhada por cima dele (largura 0 = nenhuma) */
} Morador;

static const Vector2 postes[TOTAL_POSTES] = {
    { 424, 256 }, { 860, 256 }, { 120, 520 }, { 472, 520 }, { 860, 520 }, { 1180, 520 }
};

static const Morador moradorDaRua    = { QUEM_MORADOR, 636, 456, true, { 0, 0, 0, 0 } };
static const Morador viajanteNaRua   = { QUEM_VIAJANTE, 1144, 196, true, { 0, 0, 0, 0 } };
/* No bar, cada um fica atrás de um móvel: o retângulo é o móvel. */
static const Morador viajanteNaMesa  = { QUEM_VIAJANTE, X_MESA, 232, false, { 414, 220, 116, 60 } };
static const Morador atendente       = { QUEM_BALCAO, 172, 156, true, { 38, 138, 268, 50 } };
static const Morador fregues         = { QUEM_FREGUES, 128, 276, false, { 94, 258, 68, 48 } };

/* ---------- Falas ----------
   Cada linha tem no máximo 60 letras: é o que cabe na caixa de diálogo
   e também o que cabe no texto do tradutor, que troca cada letra por
   um símbolo de 2 bytes. */
static const Pagina falaMoradorSemTradutor[] = {
    { QUEM_MORADOR, "Bons ventos, forasteiro!\nProcura alguém na cidade?" },
    { QUEM_TRIPULANTE, "(Não entendi uma palavra.\nSem um tradutor, não dá para conversar aqui.)" }
};
static const Pagina falaMorador[] = {
    { QUEM_MORADOR, "Bons ventos, forasteiro!\nProcura alguém na cidade?" }
};
static const Pagina respostaCumprimento[] = {
    { QUEM_MORADOR, "Faz tempo que ninguém de fora pousa aqui.\nSeja bem-vindo. A cidade fica calma quando escurece." }
};
static const Pagina respostaSobreViajante[] = {
    { QUEM_MORADOR, "Ah, o Claude. Viaja de estrela em estrela\ne nunca fica muito tempo no mesmo lugar." },
    { QUEM_MORADOR, "Ele carrega um pedaço de mapa estelar.\nNão vende: só aposta, numa partida de Blefe Orbital." },
    { QUEM_MORADOR, "Fica na porta do bar, na rua de cima, à direita.\nCuidado: ele mente o valor das cartas sem piscar." }
};
static const Pagina conviteViajante[] = {
    { QUEM_VIAJANTE, "Um traje que eu nunca vi! Você vem de longe.\nEu sou Claude. Também estou só de passagem." },
    { QUEM_VIAJANTE, "Tenho um pedaço de mapa estelar que não me serve.\nNão vendo. Mas aposto ele numa partida de cartas." },
    { QUEM_VIAJANTE, "A mesa do bar está livre. Vamos entrar?" }
};
static const Pagina conviteViajanteSemTradutor[] = {
    { QUEM_VIAJANTE, "Um traje que eu nunca vi! Você vem de longe.\nEu sou Claude. Também estou só de passagem." },
    { QUEM_VIAJANTE, "Tenho um pedaço de mapa estelar que não me serve.\nNão vendo. Mas aposto ele numa partida de cartas." },
    { QUEM_TRIPULANTE, "(Não entendi nada. Mas ele mostra um baralho\ne aponta para a porta do bar.)" }
};
static const Pagina conviteDeNovo[] = {
    { QUEM_VIAJANTE, "Mudou de ideia? A mesa continua livre." }
};
static const Pagina conviteAmistoso[] = {
    { QUEM_VIAJANTE, "O mapa já é seu, e foi bem ganho.\nMas uma partida por amizade eu nunca recuso." }
};
static const Pagina falaAtendente[] = {
    { QUEM_BALCAO, "O de sempre? Ah, você não é daqui.\nA casa não serve nada que um traje desses aguente." }
};
static const Pagina falaFregues[] = {
    { QUEM_FREGUES, "O Claude blefa mais quando a carta dele é baixa.\nOu será o contrário? Já perdi a conta." },
    { QUEM_FREGUES, "Já jogou no fliperama ali do canto?\nNinguém aqui passou da quinta onda." }
};
static const Pagina falaNaoEntendi[] = {
    { QUEM_TRIPULANTE, "(Sem um tradutor, não dá para saber o que foi dito.)" }
};
static const Pagina falaSemRevanche[] = {
    { QUEM_VIAJANTE, "Por hoje chega. Uma partida por pouso:\né a regra da casa." }
};
static const Pagina fimGanhouMapa[] = {
    { QUEM_VIAJANTE, "Bem jogado. Apostei e perdi: o pedaço do mapa é seu.\nQue ele leve vocês de volta para casa." },
    { QUEM_TRIPULANTE, "(Ganhei o pedaço do mapa estelar!\nÉ hora de voltar para o módulo de pouso.)" }
};
static const Pagina fimPerdeuMapa[] = {
    { QUEM_VIAJANTE, "Hoje a sorte ficou do meu lado.\nO mapa continua comigo. Volte em outro pouso." },
    { QUEM_TRIPULANTE, "(Perdi. Ele guarda o mapa e recolhe as cartas.\nSó dá para tentar de novo em outro pouso.)" }
};
static const Pagina fimGanhouAmistosa[] = {
    { QUEM_VIAJANTE, "Você lê bem um blefe. Foi uma boa partida." }
};
static const Pagina fimPerdeuAmistosa[] = {
    { QUEM_VIAJANTE, "Desta vez a mesa foi minha. Boa partida." }
};
#define TOTAL(vetor) ((int)(sizeof(vetor) / sizeof((vetor)[0])))

/* ---------- Estado ---------- */
static Texture2D fundoRua, fundoBar, frenteBar, fundoMesa;
static Texture2D folhaAliens, folhaPoste, folhaCartas, folhaItens;
static Animacao planeta;                 /* o planeta girando, usado no holograma */
static PixelExploracao *pixelsRua, *pixelsBar;
static MapaExploracao pisoRua, pisoBar;
static ExploradorPlaneta explorador;

static bool iniciou = false;
static Lugar lugar;
static ModoET modo;
static Alvo alvo;
static float tempo;                      /* relógio das animações */

static bool viajanteNoBar;               /* aceitou o convite: ele espera na mesa */
static bool jaConvidou;                  /* o viajante já se apresentou neste pouso */
static bool jogouNestePouso;             /* a partida deste pouso já aconteceu */
static bool valendoMapa;                 /* a partida em andamento vale o pedaço do mapa */

static const Pagina *conversa;
static int totalPaginas, paginaAtual;
static Depois depois;
static char textoPagina[TAM_FALA];
static TipoPergunta pergunta;
static int opcao;

static float tempoTroca;
static Lugar destinoTroca;
static bool trocou;

static PartidaCartas cartas;
static FalaTraduzida anuncio;            /* o que o viajante disse da carta dele */
static int cartaSelecionada, acaoSelecionada;
static float subida[CARTAS_NA_MAO];      /* quanto cada carta da mão está levantada */
static float tempoDesistir;              /* ESC duas vezes para desistir */

static float tempoAviso;
static char aviso[96];

/* ---------- Carregamento ---------- */

static Texture2D carregarImagem(const char *arquivo)
{
    Texture2D textura = LoadTexture(arquivo);
    if (textura.id != 0) {
        SetTextureFilter(textura, TEXTURE_FILTER_POINT);
    }
    return textura;
}

/* Lê a máscara do piso (branco = dá para andar). */
static void carregarPiso(const char *arquivo, int largura, int altura,
                         PixelExploracao **pixels, MapaExploracao *piso)
{
    Image imagem = LoadImage(arquivo);
    if (imagem.data == NULL || imagem.width != largura || imagem.height != altura) {
        TraceLog(LOG_ERROR, "EXTRATERRESTRE: mascara com tamanho errado: %s", arquivo);
        if (imagem.data != NULL) {
            UnloadImage(imagem);
        }
        return;
    }
    Color *cores = LoadImageColors(imagem);
    int quantidade = imagem.width * imagem.height;
    *pixels = malloc((size_t)quantidade * sizeof(**pixels));
    if (cores != NULL && *pixels != NULL) {
        for (int i = 0; i < quantidade; i++) {
            (*pixels)[i] = (PixelExploracao){ cores[i].r, cores[i].g, cores[i].b, cores[i].a };
        }
        *piso = (MapaExploracao){ *pixels, imagem.width, imagem.height };
    }
    if (cores != NULL) {
        UnloadImageColors(cores);
    }
    UnloadImage(imagem);
}

void iniciarTelaExtraterrestre(void)
{
    if (iniciou) {
        return;
    }
    fundoRua = carregarImagem(ARQUIVO_RUA);
    fundoBar = carregarImagem(ARQUIVO_BAR);
    frenteBar = carregarImagem(ARQUIVO_BAR_FRENTE);
    fundoMesa = carregarImagem(ARQUIVO_MESA);
    folhaAliens = carregarImagem(ARQUIVO_ALIENS);
    folhaPoste = carregarImagem(ARQUIVO_POSTE);
    folhaCartas = carregarImagem(ARQUIVO_CARTAS);
    folhaItens = carregarImagem(ARQUIVO_ITENS);
    carregarFliperama();
    carregarAnimacao(&planeta, ARQUIVO_PLANETA, TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
    carregarPiso(ARQUIVO_RUA_PISO, LARGURA_RUA, ALTURA_RUA, &pixelsRua, &pisoRua);
    carregarPiso(ARQUIVO_BAR_PISO, LARGURA_VIRTUAL, ALTURA_VIRTUAL, &pixelsBar, &pisoBar);
    iniciarExploradorPlaneta(&explorador, (PontoExploracao){ X_INICIO, Y_INICIO });
    iniciou = true;
}

static void colocarExplorador(float x, float y, int direcao)
{
    explorador.pes = (PontoExploracao){ x, y };
    explorador.direcao = direcao;
    explorador.quadro = 0;
    explorador.tempo = 0;
    explorador.correndo = false;
}

void entrarTelaExtraterrestre(void)
{
    if (!iniciou) {
        iniciarTelaExtraterrestre();
    }
    lugar = LUGAR_RUA;
    modo = ET_ANDANDO;
    alvo = ALVO_NENHUM;
    colocarExplorador(X_INICIO, Y_INICIO, 3);

    /* Cada pouso é um novo encontro: o viajante volta para a porta do
       bar e a partida do pouso ainda não aconteceu. */
    viajanteNoBar = false;
    jaConvidou = false;
    jogouNestePouso = false;
    valendoMapa = false;
    tempoAviso = 0;
    tempoTroca = 0;
    tempoDesistir = 0;
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

static float limitar(float valor, float minimo, float maximo)
{
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

static void avisar(const char *mensagem)
{
    snprintf(aviso, sizeof aviso, "%s", mensagem);
    tempoAviso = TEMPO_AVISO;
}

/* Largura de um texto na fonte de 8 px: cada letra ocupa 8 px. Bytes
   de continuação do UTF-8 (10xxxxxx) não contam como letra. */
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

/* O nome do viajante só é conhecido por quem entende o que ele diz. */
static const char *nomeViajante(void)
{
    if (progressoPartida()->temTradutor) {
        return "CLAUDE";
    }
    return "VIAJANTE";
}

/* Canto de cima da parte do mundo que aparece na tela. Na rua a câmera
   segue o tripulante e para nas bordas; o bar cabe inteiro na tela. */
static Vector2 cantoDaCamera(void)
{
    Vector2 canto = { 0, 0 };
    if (lugar == LUGAR_RUA) {
        canto.x = floorf(limitar(explorador.pes.x - LARGURA_VIRTUAL / 2, 0, LARGURA_RUA - LARGURA_VIRTUAL));
        canto.y = floorf(limitar(explorador.pes.y - 204, 0, ALTURA_RUA - ALTURA_VIRTUAL));
    }
    return canto;
}

/* ---------- Conversas ---------- */

/* Prepara o texto da página atual. A fala de um alienígena passa pelo
   tradutor, linha por linha: sem o aparelho, vira símbolos. */
static void prepararPagina(void)
{
    const Pagina *p = &conversa[paginaAtual];
    if (p->quem == QUEM_TRIPULANTE) {
        snprintf(textoPagina, sizeof textoPagina, "%s", p->texto);
        return;
    }
    tocarSom(SOM_FALA_ALIEN);
    bool temTradutor = progressoPartida()->temTradutor;
    textoPagina[0] = '\0';
    const char *inicio = p->texto;
    while (*inicio != '\0') {
        char linha[TRADUTOR_TAM_TEXTO];
        size_t tamanho = strcspn(inicio, "\n");
        if (tamanho >= sizeof linha) {
            tamanho = sizeof linha - 1;
        }
        memcpy(linha, inicio, tamanho);
        linha[tamanho] = '\0';

        FalaAlien original = { linha, false };
        FalaTraduzida traduzida;
        traduzir(&original, temTradutor, 0, 0, &traduzida);
        size_t usados = strlen(textoPagina);
        snprintf(textoPagina + usados, sizeof textoPagina - usados, "%s", traduzida.texto);

        inicio += strcspn(inicio, "\n");
        if (*inicio == '\n') {
            usados = strlen(textoPagina);
            snprintf(textoPagina + usados, sizeof textoPagina - usados, "\n");
            inicio++;
        }
    }
}

static void comecarConversa(const Pagina *paginas, int total, Depois aoTerminar)
{
    conversa = paginas;
    totalPaginas = total;
    paginaAtual = 0;
    depois = aoTerminar;
    modo = ET_FALA;
    prepararPagina();
}

static void abrirPergunta(TipoPergunta tipo)
{
    pergunta = tipo;
    opcao = 0;
    modo = ET_PERGUNTA;
}

static void comecarTroca(Lugar destino)
{
    tocarSom(SOM_PORTA_BAR);       /* a porta de vaivém, entrando ou saindo */
    destinoTroca = destino;
    tempoTroca = 0;
    trocou = false;
    modo = ET_TROCA;
}

static void conversarComMorador(void)
{
    if (progressoPartida()->temTradutor) {
        comecarConversa(falaMorador, TOTAL(falaMorador), DEPOIS_PERGUNTAR_AO_MORADOR);
    } else {
        comecarConversa(falaMoradorSemTradutor, TOTAL(falaMoradorSemTradutor), DEPOIS_NADA);
    }
}

static void conversarComViajante(void)
{
    ProgressoPartida *jogo = progressoPartida();
    if (jogo->extraterrestreConcluido) {
        comecarConversa(conviteAmistoso, TOTAL(conviteAmistoso), DEPOIS_CONVITE);
    } else if (jaConvidou) {
        comecarConversa(conviteDeNovo, TOTAL(conviteDeNovo), DEPOIS_CONVITE);
    } else if (jogo->temTradutor) {
        comecarConversa(conviteViajante, TOTAL(conviteViajante), DEPOIS_CONVITE);
    } else {
        comecarConversa(conviteViajanteSemTradutor, TOTAL(conviteViajanteSemTradutor), DEPOIS_CONVITE);
    }
    jaConvidou = true;
}

/* Fala de quem está no bar. Sem tradutor, o tripulante comenta depois. */
static void conversarNoBar(const Pagina *paginas, int total)
{
    comecarConversa(paginas, total, DEPOIS_NADA);
}

/* ---------- Cartas ---------- */

static void anunciarCarta(void)
{
    char mensagem[64];
    snprintf(mensagem, sizeof(mensagem), "Minha carta vale %d.", cartas.anuncioAlien);
    FalaAlien original = { mensagem, cartas.anuncioAlien != cartas.maoAlien[cartas.indiceAlien] };
    float sorteio = (float)GetRandomValue(0, 9999) / 10000;
    traduzir(&original, progressoPartida()->temTradutor,
             tripulanteControlado()->tracos.inteligencia, sorteio, &anuncio);
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        if (!cartas.usadasJogador[i]) {
            cartaSelecionada = i;
            break;
        }
    }
    acaoSelecionada = 0;
    tocarSom(SOM_FALA_ALIEN);
}

static void novaPartida(void)
{
    iniciarCartas(&cartas, (uint32_t)GetRandomValue(1, 1000000));
    valendoMapa = !progressoPartida()->extraterrestreConcluido;
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        subida[i] = 0;
    }
    tempoDesistir = 0;
    tocarSom(SOM_CARTA_MEXE);
    anunciarCarta();
    modo = ET_CARTAS;
}

/* A partida do pouso acabou: entrega (ou não) o pedaço do mapa e volta
   para o bar, onde o viajante comenta o resultado. */
static void encerrarPartida(bool venceu)
{
    ProgressoPartida *jogo = progressoPartida();
    jogouNestePouso = true;
    if (venceu) {
        jogo->vitoriasCartas++;
        tocarSom(SOM_VITORIA);
    } else {
        tocarSom(SOM_DERROTA);
    }
    if (venceu && valendoMapa) {
        jogo->extraterrestreConcluido = true;
        comecarConversa(fimGanhouMapa, TOTAL(fimGanhouMapa), DEPOIS_NADA);
    } else if (venceu) {
        comecarConversa(fimGanhouAmistosa, TOTAL(fimGanhouAmistosa), DEPOIS_NADA);
    } else if (valendoMapa) {
        comecarConversa(fimPerdeuMapa, TOTAL(fimPerdeuMapa), DEPOIS_NADA);
    } else {
        comecarConversa(fimPerdeuAmistosa, TOTAL(fimPerdeuAmistosa), DEPOIS_NADA);
    }
}

static void atualizarEscolhaDeCarta(void)
{
    int movimento = (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) -
                    (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT));
    if (movimento != 0) {
        tocarSom(SOM_CARTA_MEXE);
        for (int n = 0; n < CARTAS_NA_MAO; n++) {
            cartaSelecionada = (cartaSelecionada + movimento + CARTAS_NA_MAO) % CARTAS_NA_MAO;
            if (!cartas.usadasJogador[cartaSelecionada]) {
                break;
            }
        }
    }
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        if (IsKeyPressed(KEY_ONE + i) && !cartas.usadasJogador[i]) {
            cartaSelecionada = i;
        }
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S)) {
        acaoSelecionada = 1 - acaoSelecionada;
        tocarSom(SOM_UI_MOVER);
    }
    if (IsKeyPressed(KEY_ENTER) && jogarCartas(&cartas, cartaSelecionada, acaoSelecionada == 1)) {
        /* A carta bate na mesa e a rodada já tem um resultado. */
        tocarSom(SOM_CARTA_JOGA);
        if (cartas.venceuRodada) {
            tocarSom(SOM_RODADA_GANHA);
        } else {
            tocarSom(SOM_RODADA_PERDE);
        }
    }
}

static void atualizarCartas(float dt)
{
    /* A carta escolhida sobe um pouco; as outras descem. */
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        float destino = 0;
        if (i == cartaSelecionada && cartas.fase == CARTAS_ESCOLHENDO && !cartas.usadasJogador[i]) {
            destino = 12;
        }
        subida[i] += (destino - subida[i]) * limitar(dt * 14, 0, 1);
    }
    if (tempoDesistir > 0) {
        tempoDesistir -= dt;
    }

    if (cartas.fase == CARTAS_ENCERRADAS) {
        if (IsKeyPressed(KEY_ENTER)) {
            encerrarPartida(consumirVitoriaCartas(&cartas));
        }
        return;
    }
    /* Sair no meio da partida é desistir: pede ESC duas vezes. */
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (tempoDesistir > 0) {
            encerrarPartida(false);
        } else {
            tempoDesistir = TEMPO_AVISO;
        }
        return;
    }
    if (cartas.fase == CARTAS_ESCOLHENDO) {
        atualizarEscolhaDeCarta();
    } else if (IsKeyPressed(KEY_ENTER)) {
        if (avancarRodadaCartas(&cartas)) {
            anunciarCarta();
        }
    }
}

/* ---------- Lógica ---------- */

/* O que está perto o bastante para usar com E: o mais próximo ganha. */
static Alvo encontrarAlvo(void)
{
    typedef struct { Alvo alvo; float x, y, raio; } Ponto;
    Ponto pontos[5];
    int total = 0;
    if (lugar == LUGAR_RUA) {
        pontos[total++] = (Ponto){ ALVO_MODULO, X_MODULO, Y_MODULO, DISTANCIA_MODULO };
        pontos[total++] = (Ponto){ ALVO_MORADOR, moradorDaRua.x, moradorDaRua.y, 48 };
        pontos[total++] = (Ponto){ ALVO_PORTA_BAR, X_PORTA_BAR, Y_PORTA_BAR - 8, 36 };
        if (!viajanteNoBar) {
            pontos[total++] = (Ponto){ ALVO_VIAJANTE, viajanteNaRua.x, viajanteNaRua.y, 48 };
        }
    } else {
        pontos[total++] = (Ponto){ ALVO_MESA, X_MESA, Y_MESA, 64 };
        pontos[total++] = (Ponto){ ALVO_BALCAO, atendente.x, atendente.y, 72 };
        pontos[total++] = (Ponto){ ALVO_FREGUES, fregues.x, fregues.y, 56 };
        pontos[total++] = (Ponto){ ALVO_SAIDA, X_ENTRADA_BAR, Y_SAIDA_BAR - 6, 26 };
        pontos[total++] = (Ponto){ ALVO_FLIPERAMA, X_FLIPERAMA, Y_FLIPERAMA, 34 };
    }
    Alvo encontrado = ALVO_NENHUM;
    float menor = 1e9f;
    for (int i = 0; i < total; i++) {
        float d = distancia(explorador.pes.x, explorador.pes.y, pontos[i].x, pontos[i].y);
        if (d <= pontos[i].raio && d < menor) {
            menor = d;
            encontrado = pontos[i].alvo;
        }
    }
    return encontrado;
}

/* Tecla E na mesa de cartas do bar. */
static void usarMesa(void)
{
    if (!viajanteNoBar) {
        tocarSom(SOM_NEGADO);
        avisar("A mesa de cartas está vazia: falta um adversário.");
    } else if (jogouNestePouso) {
        conversarNoBar(falaSemRevanche, TOTAL(falaSemRevanche));
    } else {
        modo = ET_REGRAS;
    }
}

static Tela usarAlvo(void)
{
    switch (alvo) {
        case ALVO_MODULO:    return TELA_NAVE;
        case ALVO_MORADOR:   conversarComMorador(); break;
        case ALVO_VIAJANTE:  conversarComViajante(); break;
        case ALVO_PORTA_BAR: comecarTroca(LUGAR_BAR); break;
        case ALVO_SAIDA:     comecarTroca(LUGAR_RUA); break;
        case ALVO_MESA:      usarMesa(); break;
        case ALVO_BALCAO:    conversarNoBar(falaAtendente, TOTAL(falaAtendente)); break;
        case ALVO_FREGUES:   conversarNoBar(falaFregues, TOTAL(falaFregues)); break;
        case ALVO_FLIPERAMA:
            abrirFliperama();
            modo = ET_FLIPERAMA;
            break;
        default: break;
    }
    return TELA_EXTRATERRESTRE;
}

static void atualizarFala(void)
{
    if (!confirmou()) {
        return;
    }
    paginaAtual++;
    if (paginaAtual < totalPaginas) {
        prepararPagina();
        return;
    }
    /* Acabou. Quem falou por último foi um alienígena e não há
       tradutor: o tripulante diz que não entendeu. */
    bool alienPorUltimo = conversa[totalPaginas - 1].quem != QUEM_TRIPULANTE;
    if (alienPorUltimo && depois == DEPOIS_NADA && !progressoPartida()->temTradutor &&
        conversa != falaNaoEntendi) {
        comecarConversa(falaNaoEntendi, TOTAL(falaNaoEntendi), DEPOIS_NADA);
        return;
    }
    if (depois == DEPOIS_PERGUNTAR_AO_MORADOR) {
        abrirPergunta(PERGUNTA_MORADOR);
    } else if (depois == DEPOIS_CONVITE) {
        abrirPergunta(PERGUNTA_CONVITE);
    } else {
        modo = ET_ANDANDO;
    }
}

static void atualizarPergunta(void)
{
    int total = 2;
    if (pergunta == PERGUNTA_MORADOR) {
        total = 3;
    }
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        tocarSom(SOM_UI_MOVER);
        opcao = (opcao + total - 1) % total;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        tocarSom(SOM_UI_MOVER);
        opcao = (opcao + 1) % total;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        modo = ET_ANDANDO;
        return;
    }
    if (!confirmou()) {
        return;
    }
    if (pergunta == PERGUNTA_MORADOR) {
        /* Depois de cada resposta, a lista volta: dá para perguntar as duas coisas. */
        if (opcao == 0) {
            comecarConversa(respostaCumprimento, TOTAL(respostaCumprimento), DEPOIS_PERGUNTAR_AO_MORADOR);
        } else if (opcao == 1) {
            comecarConversa(respostaSobreViajante, TOTAL(respostaSobreViajante), DEPOIS_PERGUNTAR_AO_MORADOR);
        } else {
            modo = ET_ANDANDO;
        }
    } else if (opcao == 0) {
        viajanteNoBar = true;          /* aceitou: os dois entram no bar */
        comecarTroca(LUGAR_BAR);
    } else {
        modo = ET_ANDANDO;
    }
}

static void atualizarTroca(float dt)
{
    tempoTroca += dt;
    if (!trocou && tempoTroca >= TEMPO_TROCA / 2) {
        trocou = true;
        lugar = destinoTroca;
        if (lugar == LUGAR_BAR) {
            colocarExplorador(X_ENTRADA_BAR, Y_ENTRADA_BAR, 1);
            definirAmbiente(AMBIENTE_BAR);
            definirPiso(PISO_MADEIRA);
        } else {
            colocarExplorador(X_PORTA_BAR, Y_PORTA_BAR + 8, 0);
            definirAmbiente(AMBIENTE_CIDADE);
            definirPiso(PISO_ASFALTO);
        }
    }
    if (tempoTroca >= TEMPO_TROCA) {
        modo = ET_ANDANDO;
    }
}

static Tela atualizarAndando(float dt)
{
    /* Os alienígenas da rua também bloqueiam a passagem. */
    ObstaculoExploracao obstaculos[2];
    int total = 0;
    const MapaExploracao *piso = &pisoBar;
    if (lugar == LUGAR_RUA) {
        piso = &pisoRua;
        obstaculos[total++] = (ObstaculoExploracao){ moradorDaRua.x - 10, moradorDaRua.y - 8, 20, 8 };
        if (!viajanteNoBar) {
            obstaculos[total++] = (ObstaculoExploracao){ viajanteNaRua.x - 10, viajanteNaRua.y - 8, 20, 8 };
        }
    }
    atualizarExploradorPlaneta(&explorador, piso, obstaculos, total, dt, true);
    alvo = encontrarAlvo();

    if (lugar == LUGAR_BAR && (explorador.pes.y >= Y_SAIDA_BAR || IsKeyPressed(KEY_ESCAPE))) {
        comecarTroca(LUGAR_RUA);     /* saiu pela porta */
        return TELA_EXTRATERRESTRE;
    }
    if (lugar == LUGAR_RUA && IsKeyPressed(KEY_ESCAPE)) {
        return TELA_NAVE;
    }
    if (IsKeyPressed(KEY_E)) {
        return usarAlvo();
    }
    return TELA_EXTRATERRESTRE;
}

Tela atualizarTelaExtraterrestre(void)
{
    float dt = GetFrameTime();
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    tempo += dt;
    atualizarAnimacao(&planeta);
    if (tempoAviso > 0) {
        tempoAviso -= dt;
    }

    switch (modo) {
        case ET_FALA:     atualizarFala(); break;
        case ET_PERGUNTA: atualizarPergunta(); break;
        case ET_TROCA:    atualizarTroca(dt); break;
        case ET_CARTAS:   atualizarCartas(dt); break;
        case ET_REGRAS:
            if (IsKeyPressed(KEY_ESCAPE)) {
                modo = ET_ANDANDO;
            } else if (confirmou()) {
                novaPartida();
            }
            break;
        case ET_FLIPERAMA:
            if (!atualizarTelaFliperama()) {
                definirAmbiente(AMBIENTE_BAR);
                modo = ET_ANDANDO;      /* ESC: volta para o bar */
            }
            break;
        case ET_ANDANDO:  return atualizarAndando(dt);
    }
    return TELA_EXTRATERRESTRE;
}

/* ---------- Desenho: peças ---------- */

static void sombraDoTexto(const char *s, float x, float y, int tamanho, Color cor)
{
    textoPlaneta(s, x + 1, y + 1, tamanho, BLACK);
    textoPlaneta(s, x, y, tamanho, cor);
}

/* Painel com os cantos cortados, no lugar do retângulo comum. */
static void painel(int x, int y, int w, int h, Color fundo, Color borda)
{
    DrawRectangle(x + 5, y + 4, w - 4, h, Fade(BLACK, 0.45f));
    DrawRectangle(x + 2, y, w - 4, h, borda);
    DrawRectangle(x, y + 2, w, h - 4, borda);
    DrawRectangle(x + 1, y + 1, w - 2, h - 2, borda);
    DrawRectangle(x + 3, y + 1, w - 6, h - 2, fundo);
    DrawRectangle(x + 1, y + 3, w - 2, h - 6, fundo);
    DrawRectangle(x + 2, y + 2, w - 4, h - 4, fundo);
}

/* Um alienígena da folha: linha = quem é; coluna = para onde olha
   (0 frente, 2 direita, 4 esquerda) mais 1 quando pisca. */
static void desenharAlien(Quem quem, float x, float y, int vista, float escala, bool sombra)
{
    int coluna = vista * 2;
    if (fmodf(tempo + (float)quem * 1.3f, 3.7f) < 0.14f) {
        coluna++;                      /* piscada */
    }
    float largura = LARGURA_PERSONAGEM * escala;
    float altura = ALTURA_PERSONAGEM * escala;
    if (sombra) {
        DrawEllipse((int)x, (int)y - 3, 13 * escala, 4 * escala, Fade(BLACK, 0.4f));
    }
    Rectangle origem = { (float)(coluna * LARGURA_PERSONAGEM), (float)(quem * ALTURA_PERSONAGEM),
                         LARGURA_PERSONAGEM, ALTURA_PERSONAGEM };
    Rectangle destino = { floorf(x - largura / 2), floorf(y - altura), largura, altura };
    DrawTexturePro(folhaAliens, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

static void desenharMorador(const Morador *m)
{
    int vista = 0;
    float dx = explorador.pes.x - m->x;
    if (m->viraParaOlhar && distancia(explorador.pes.x, explorador.pes.y, m->x, m->y) < 150) {
        if (dx > 28) {
            vista = 1;
        } else if (dx < -28) {
            vista = 2;
        }
    }
    desenharAlien(m->quem, m->x, m->y, vista, 1, m->movel.width == 0);
    if (m->movel.width > 0) {
        /* O móvel na frente dele (balcão ou mesa) cobre as pernas. */
        DrawTextureRec(frenteBar, m->movel, (Vector2){ m->movel.x, m->movel.y }, WHITE);
    }
}

static void desenharPoste(Vector2 base)
{
    float x = base.x - 6, y = base.y - ALTURA_POSTE + 4;
    DrawTextureRec(folhaPoste, (Rectangle){ 0, 0, LARGURA_POSTE, ALTURA_POSTE }, (Vector2){ x, y }, WHITE);
    /* A lanterna oscila de leve entre mais e menos acesa. */
    float brilho = 0.20f + 0.08f * sinf(tempo * 3 + base.x);
    DrawRectangle((int)x + 14, (int)y + 4, 10, 10, Fade(GetColor(0xFFF6D8FF), brilho));
}

/* Holograma da praça: o planeta girando, em luz azulada e com listras. */
static void desenharHolograma(void)
{
    int coluna = planeta.quadroAtual % planeta.colunas;
    int linha = planeta.quadroAtual / planeta.colunas;
    float flutua = sinf(tempo * 1.6f) * 3;
    float tamanho = TAMANHO_PLANETA;   /* no tamanho original, para os pixels ficarem inteiros */
    float x = X_HOLOGRAMA - tamanho / 2, y = Y_HOLOGRAMA - 12 - tamanho + flutua;
    float tremor = 0.62f + 0.10f * sinf(tempo * 9) + (fmodf(tempo, 4.1f) < 0.08f ? -0.3f : 0);

    BeginBlendMode(BLEND_ADDITIVE);
    /* feixe que sai da base */
    DrawTriangle((Vector2){ X_HOLOGRAMA, Y_HOLOGRAMA - 8 }, (Vector2){ x + tamanho - 8, y + tamanho - 8 },
                 (Vector2){ x + 8, y + tamanho - 8 }, Fade(COR_CIANO, 0.10f));
    if (planeta.folha.id != 0) {
        Rectangle origem = { (float)(coluna * TAMANHO_PLANETA), (float)(linha * TAMANHO_PLANETA),
                             TAMANHO_PLANETA, TAMANHO_PLANETA };
        Rectangle destino = { floorf(x), floorf(y), tamanho, tamanho };
        /* Primeiro o planeta em azul, meio transparente; depois o brilho por cima. */
        EndBlendMode();
        DrawTexturePro(planeta.folha, origem, destino, (Vector2){ 0, 0 }, 0, Fade(GetColor(0x7FE3D9FF), tremor));
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(planeta.folha, origem, destino, (Vector2){ 0, 0 }, 0, Fade(GetColor(0x4FE0D8FF), 0.35f));
    }
    EndBlendMode();
    /* listras escuras que sobem, como numa tela antiga */
    int passo = (int)(tempo * 12) % 4;
    for (int ly = (int)y + passo; ly < (int)(y + tamanho); ly += 4) {
        DrawRectangle((int)x + 6, ly, (int)tamanho - 12, 1, Fade(GetColor(0x16121FFF), 0.35f));
    }
}

/* Luzes que piscam: o letreiro do bar falha de vez em quando e a
   antena do módulo pisca em vermelho. */
static void desenharLuzesDaRua(void)
{
    if (fmodf(tempo, 5.3f) > 4.9f && (int)(tempo * 24) % 2 == 0) {
        DrawRectangle(1026, 50, 140, 30, Fade(GetColor(0x16121FFF), 0.55f));
    }
    float brilho = 0.10f + 0.06f * sinf(tempo * 2.2f);
    DrawRectangle(1076, 122, 40, 12, Fade(GetColor(0xFFD98AFF), brilho));     /* luz de dentro do bar */
    if (fmodf(tempo, 1.4f) < 0.7f) {
        DrawRectangle(144, 514, 6, 6, GetColor(0xEE9A8AFF));
    }
}

static void desenharRua(void)
{
    if (fundoRua.id != 0) {
        DrawTexture(fundoRua, 0, 0, WHITE);
    }
    desenharLuzesDaRua();

    /* Postes, alienígenas e o tripulante: quem está mais para baixo na
       tela é desenhado por último, para ficar na frente. */
    typedef struct { float y; int tipo, indice; } Figura;
    Figura figuras[TOTAL_POSTES + 3];
    int total = 0;
    for (int i = 0; i < TOTAL_POSTES; i++) {
        figuras[total++] = (Figura){ postes[i].y, 0, i };
    }
    figuras[total++] = (Figura){ moradorDaRua.y, 1, 0 };
    if (!viajanteNoBar) {
        figuras[total++] = (Figura){ viajanteNaRua.y, 2, 0 };
    }
    figuras[total++] = (Figura){ explorador.pes.y, 3, 0 };
    for (int i = 1; i < total; i++) {
        Figura atual = figuras[i];
        int j = i;
        while (j > 0 && figuras[j - 1].y > atual.y) {
            figuras[j] = figuras[j - 1];
            j--;
        }
        figuras[j] = atual;
    }
    for (int i = 0; i < total; i++) {
        if (figuras[i].tipo == 0) {
            desenharPoste(postes[figuras[i].indice]);
        } else if (figuras[i].tipo == 1) {
            desenharMorador(&moradorDaRua);
        } else if (figuras[i].tipo == 2) {
            desenharMorador(&viajanteNaRua);
        } else {
            desenharExploradorPlaneta(&explorador);
        }
    }
    desenharHolograma();
}

static void desenharBar(void)
{
    if (fundoBar.id != 0) {
        DrawTexture(fundoBar, 0, 0, WHITE);
    }
    /* Os três ficam atrás de móveis, sempre acima de quem anda pelo
       salão: são desenhados antes, e o tripulante por cima. Só quando o
       tripulante está mais ao fundo que alguém ele é desenhado antes. */
    const Morador *pessoas[3];
    int total = 0;
    pessoas[total++] = &atendente;
    if (viajanteNoBar) {
        pessoas[total++] = &viajanteNaMesa;
    }
    pessoas[total++] = &fregues;
    bool desenhouTripulante = false;
    for (int i = 0; i < total; i++) {
        if (!desenhouTripulante && explorador.pes.y < pessoas[i]->y) {
            desenharExploradorPlaneta(&explorador);
            desenhouTripulante = true;
        }
        desenharMorador(pessoas[i]);
    }
    if (!desenhouTripulante) {
        desenharExploradorPlaneta(&explorador);
    }
    /* O letreiro de neon pulsa devagar. */
    float pulso = 0.06f + 0.05f * sinf(tempo * 2.4f);
    DrawRectangle(354, 54, 120, 48, Fade(GetColor(0xF08CC8FF), pulso));
}

/* ---------- Desenho: interface da rua e do bar ---------- */

static const char *objetivoAtual(void)
{
    ProgressoPartida *jogo = progressoPartida();
    if (jogo->extraterrestreConcluido) {
        return "o pedaço do mapa é seu; volte ao módulo de pouso";
    }
    if (jogouNestePouso) {
        return "sem revanche neste pouso; volte ao módulo";
    }
    if (viajanteNoBar) {
        return "sente-se à mesa de cartas do bar";
    }
    if (jaConvidou) {
        return "aceite a partida do viajante, na porta do bar";
    }
    return "descubra quem na cidade tem um pedaço do mapa";
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
    DrawRectangle(sx - 5, sy - 5, 10, 10, COR_PAINEL);
    DrawRectangle(sx - 3, sy - 3, 6, 6, COR_MARCADOR);
    DrawRectangleLines(sx - 6 - pulso, sy - 6 - pulso, 12 + pulso * 2, 12 + pulso * 2, Fade(COR_MARCADOR, 0.5f));
    int tx = sx + 10;
    if (sx > LARGURA_VIRTUAL / 2) {
        tx = sx - 10 - 40;
    }
    sombraDoTexto("POUSO", (float)tx, (float)sy - 4, 8, COR_MARCADOR);
}

static const char *dicaDoAlvo(void)
{
    switch (alvo) {
        case ALVO_MODULO:    return "E  Voltar à nave";
        case ALVO_MORADOR:
        case ALVO_VIAJANTE:
        case ALVO_BALCAO:
        case ALVO_FREGUES:   return "E  Conversar";
        case ALVO_PORTA_BAR: return "E  Entrar no bar";
        case ALVO_SAIDA:     return "E  Sair do bar";
        case ALVO_FLIPERAMA: return "E  Jogar no fliperama";
        case ALVO_MESA:
            if (viajanteNoBar && !jogouNestePouso) {
                return "E  Sentar à mesa de cartas";
            }
            return "E  Mesa de cartas";
        default: return NULL;
    }
}

static void desenharInterface(Vector2 canto)
{
    ProgressoPartida *jogo = progressoPartida();

    /* Faixa de cima: quem desceu, tradutor, pedaços do mapa e objetivo. */
    DrawRectangle(0, 0, LARGURA_VIRTUAL, 34, Fade(GetColor(0x17131FFF), 0.86f));
    textoPlaneta(tripulanteControlado()->nome, 16, 7, 8, COR_TITULO);
    desenharTradutor(92, 1, 0.5f, jogo->temTradutor);     /* o aparelho: aceso se está com a tripulação */
    if (jogo->temTradutor) {
        textoPlaneta("TRADUTOR LIGADO", 112, 7, 8, COR_CIANO);
    } else {
        textoPlaneta("SEM TRADUTOR", 112, 7, 8, COR_ALERTA);
    }
    if (lugar == LUGAR_BAR) {
        textoPlaneta("BAR", 280, 7, 8, COR_SUBTITULO);
    } else {
        textoPlaneta("CIDADE", 280, 7, 8, COR_SUBTITULO);
    }
    if (folhaItens.id != 0) {
        Rectangle origem = { (float)(ITEM_MAPA * TAMANHO_ITEM), 0, TAMANHO_ITEM, TAMANHO_ITEM };
        DrawTexturePro(folhaItens, origem, (Rectangle){ 512, 1, 16, 16 }, (Vector2){ 0, 0 }, 0, WHITE);
    }
    textoPlaneta(TextFormat("MAPA %d/%d", pedacosMapaObtidos(), TOTAL_PEDACOS_MAPA), 532, 7, 8, COR_MARCADOR);
    textoPlaneta("OBJETIVO", 16, 21, 8, COR_MARCADOR);
    textoPlaneta(objetivoAtual(), 88, 21, 8, COR_MENU);

    DrawRectangle(0, 346, LARGURA_VIRTUAL, 14, GetColor(0x17131FFF));
    if (lugar == LUGAR_BAR) {
        textoPlaneta("WASD/SETAS Mover   SHIFT Correr   E Interagir   ESC Sair do bar", 16, 350, 8, COR_MENU);
    } else {
        textoPlaneta("WASD/SETAS Mover   SHIFT Correr   E Interagir   ESC Nave", 16, 350, 8, COR_MENU);
    }

    if (modo != ET_ANDANDO) {
        return;
    }
    if (lugar == LUGAR_RUA) {
        desenharSetaDoModulo(canto);
    }
    const char *dica = dicaDoAlvo();
    if (dica != NULL) {
        int largura = larguraTexto(dica) + 16;
        int x = (int)(explorador.pes.x - canto.x) - largura / 2;
        int y = (int)(explorador.pes.y - canto.y) + 8;
        x = (int)limitar((float)x, 8, (float)(LARGURA_VIRTUAL - 8 - largura));
        if (y > ALTURA_VIRTUAL - 40) {
            y = (int)(explorador.pes.y - canto.y) - 96;
        }
        caixaPlaneta(x, y, largura, 22);
        textoPlaneta(dica, (float)(x + 8), (float)(y + 7), 8, COR_MARCADOR);
    }
    if (tempoAviso > 0) {
        int largura = larguraTexto(aviso) + 16;
        int x = (LARGURA_VIRTUAL - largura) / 2;
        caixaPlaneta(x, 42, largura, 22);
        textoPlaneta(aviso, (float)(x + 8), 49, 8, COR_TITULO);
    }
}

static Falante falanteDe(Quem quem)
{
    Falante f;
    if (quem == QUEM_TRIPULANTE) {
        f = (Falante){ tripulanteControlado()->nome, explorador.sprite, (Rectangle){ 4, 0, 36, 36 } };
        return f;
    }
    static const char *nomes[] = { "VIAJANTE", "MORADOR", "ATENDENTE", "FREGUÊS" };
    const char *nome = nomes[quem];
    if (quem == QUEM_VIAJANTE) {
        nome = nomeViajante();
    }
    /* Rosto: a parte de cima do quadro de frente. */
    f = (Falante){ nome, folhaAliens, (Rectangle){ 4, (float)(quem * ALTURA_PERSONAGEM), 36, 36 } };
    return f;
}

static void desenharFalaAtual(void)
{
    Falante falante = falanteDe(conversa[paginaAtual].quem);
    bool temMais = paginaAtual < totalPaginas - 1;
    const char *rodape = "E / ENTER  Continuar";
    if (conversa[paginaAtual].quem != QUEM_TRIPULANTE && !progressoPartida()->temTradutor) {
        rodape = "Sem tradutor: fala desconhecida   E / ENTER  Continuar";
    }
    desenharFala(&falante, textoPagina, rodape, temMais);
}

static void desenharPergunta(void)
{
    Falante falante = falanteDe(QUEM_TRIPULANTE);
    if (pergunta == PERGUNTA_MORADOR) {
        static const char *opcoes[] = { "Cumprimentar", "Perguntar sobre o viajante", "Ir embora" };
        static const char *detalhes[] = {
            "Uma saudação de quem chegou agora.",
            "Quem é o viajante de que falam na cidade?",
            "Encerrar a conversa."
        };
        desenharEscolhaDialogo(&falante, "O que dizer ao morador?", opcoes, 3, opcao, detalhes[opcao],
                               "W / S  Escolher   E / ENTER  Confirmar");
        return;
    }
    static const char *opcoes[] = { "Entrar no bar e jogar", "Agora não" };
    const char *detalhe = "Ele espera na porta do bar.";
    if (opcao == 0 && progressoPartida()->extraterrestreConcluido) {
        detalhe = "Partida amistosa: o mapa já está com você.";
    } else if (opcao == 0) {
        detalhe = "Só uma partida por pouso: perdeu, ele guarda o mapa.";
    }
    desenharEscolhaDialogo(&falante, "Aceitar o convite do viajante?", opcoes, 2, opcao, detalhe,
                           "W / S  Escolher   E / ENTER  Confirmar");
}

static void desenharRegras(void)
{
    DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, 0.7f));
    painel(24, 26, 592, 308, COR_PAINEL, COR_BORDA);
    textoPlaneta("BLEFE ORBITAL", 44, 46, 16, COR_TITULO);
    /* Uma carta de cada tipo, para enfeitar o canto. */
    for (int i = 0; i < 3; i++) {
        Rectangle origem = { (float)((2 + i * 3) * LARGURA_CARTA), 0, LARGURA_CARTA, ALTURA_CARTA };
        Rectangle destino = { (float)(468 + i * 42), 40, LARGURA_CARTA / 2, ALTURA_CARTA / 2 };
        DrawTexturePro(folhaCartas, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
    }
    static const char *linhas[] = {
        "Baralho de 1 a 9, sem repetição.",
        "Você e o viajante recebem 3 cartas.",
        "Ele escolhe uma carta oculta e anuncia um valor.",
        "Escolha sua carta e uma ação:",
        "  COMPARAR: vence a maior carta real.",
        "  DESCONFIAR: você vence se ele mentiu.",
        "Cada rodada gasta uma carta de cada mão.",
        "Vence quem somar 2 pontos; são até 3 rodadas.",
        "Sem tradutor, a fala vira símbolos; os números ficam.",
        "Com tradutor, a leitura do tom de voz ainda pode errar."
    };
    for (int i = 0; i < 10; i++) {
        textoPlaneta(linhas[i], 44, (float)(100 + i * 17), 8, COR_MENU);
    }
    if (progressoPartida()->extraterrestreConcluido) {
        textoPlaneta("Partida amistosa: o mapa já está com você.", 44, 278, 8, COR_CIANO);
    } else {
        textoPlaneta("Vale o pedaço do mapa. Só uma partida por pouso.", 44, 278, 8, COR_ALERTA);
    }
    textoPlaneta("ENTER  Começar a partida", 44, 306, 8, COR_MARCADOR);
    textoPlaneta("ESC  Ainda não", 484, 306, 8, COR_MENU);
}

/* ---------- Desenho: mesa de cartas ---------- */

/* quadro: 0 é o verso, 1 a 9 são os valores e 10 é o lugar vazio. */
static void desenharCarta(int quadro, float x, float y, float escala)
{
    Rectangle origem = { (float)(quadro * LARGURA_CARTA), 0, LARGURA_CARTA, ALTURA_CARTA };
    Rectangle destino = { floorf(x), floorf(y), LARGURA_CARTA * escala, ALTURA_CARTA * escala };
    DrawRectangle((int)destino.x + 3, (int)destino.y + 4, (int)destino.width, (int)destino.height,
                  Fade(BLACK, 0.35f));
    DrawTexturePro(folhaCartas, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

/* Bolinhas de pontos: cheias para os pontos já feitos. */
static void desenharPontos(int x, int y, int pontos, Color cor)
{
    for (int i = 0; i < 2; i++) {
        DrawRectangle(x + i * 12, y, 8, 8, GetColor(0x40334FFF));
        if (i < pontos) {
            DrawRectangle(x + i * 12 + 1, y + 1, 6, 6, cor);
        }
    }
}

static void desenharBalaoDoAnuncio(void)
{
    bool temTradutor = progressoPartida()->temTradutor;
    int x = 206, y = 46, w = 418, h = 82;
    painel(x, y, w, h, GetColor(0xF1E8D2FF), GetColor(0x241A3AFF));
    /* ponta do balão, virada para o viajante */
    for (int i = 0; i < 8; i++) {
        DrawRectangle(x - 8 + i, y + 34 - i / 2, 1, 10 + i, GetColor(0x241A3AFF));
    }
    for (int i = 2; i < 10; i++) {
        DrawRectangle(x - 8 + i, y + 36 - i / 2, 1, 6 + i, GetColor(0xF1E8D2FF));
    }
    textoPlaneta(TextFormat("%s ANUNCIA", nomeViajante()), (float)x + 16, (float)y + 12, 8, GetColor(0x7A2A5CFF));
    textoPlaneta(anuncio.texto, (float)x + 16, (float)y + 30, 16, GetColor(0x241A3AFF));
    if (!temTradutor) {
        textoPlaneta("sem tradutor: só os números são legíveis", (float)x + 16, (float)y + 60, 8,
                     GetColor(0x7D7796FF));
    } else {
        Color cor = GetColor(0x2B8A8FFF);
        if (anuncio.leitura == LEITURA_SUSPEITA) {
            cor = GetColor(0xB03020FF);
        }
        DrawRectangle(x + 16, y + 60, 8, 8, cor);
        textoPlaneta(textoLeitura(anuncio.leitura), (float)x + 30, (float)y + 60, 8, cor);
    }
}

/* As três cartas do viajante, viradas para baixo. A que ele escolheu
   avança; no resultado, ela vira e mostra o valor verdadeiro. */
static void desenharCartasDoViajante(void)
{
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        float x = 330 + (float)i * 46, y = 174;
        bool escolhida = i == cartas.indiceAlien;
        if (cartas.usadasAlien[i] && !(escolhida && cartas.fase != CARTAS_ESCOLHENDO)) {
            continue;                               /* já saiu do jogo */
        }
        int quadro = QUADRO_VERSO;
        if (escolhida) {
            y += 10;
            if (cartas.fase != CARTAS_ESCOLHENDO) {
                quadro = cartas.maoAlien[i];
            }
        }
        desenharCarta(quadro, x, y, 0.5f);
        if (escolhida && cartas.fase == CARTAS_ESCOLHENDO) {
            float pulso = 0.5f + 0.5f * sinf(tempo * 5);
            DrawRectangleLinesEx((Rectangle){ x - 2, y - 2, 42, 56 }, 1, Fade(COR_MARCADOR, 0.4f + 0.6f * pulso));
        }
    }
    sombraDoTexto("ele joga esta", 476, 204, 8, GetColor(0x9FE8D6FF));
}

static void desenharMaoDoJogador(void)
{
    for (int i = 0; i < CARTAS_NA_MAO; i++) {
        float x = 66 + (float)i * 84, y = 238 - subida[i];
        if (cartas.usadasJogador[i]) {
            Rectangle origem = { (float)(QUADRO_VAZIO * LARGURA_CARTA), 0, LARGURA_CARTA, ALTURA_CARTA };
            DrawTextureRec(folhaCartas, origem, (Vector2){ x, 238 }, WHITE);
            continue;
        }
        desenharCarta(cartas.maoJogador[i], x, y, 1);
        if (i == cartaSelecionada && cartas.fase == CARTAS_ESCOLHENDO) {
            DrawRectangleLinesEx((Rectangle){ x - 3, y - 3, LARGURA_CARTA + 6, ALTURA_CARTA + 6 }, 2, COR_MARCADOR);
        }
        sombraDoTexto(TextFormat("%d", i + 1), x + 34, 338, 8, COR_MENU);
    }
}

static void desenharBotoesDeAcao(void)
{
    static const char *acoes[] = { "COMPARAR", "DESCONFIAR" };
    static const char *explica[] = { "vence a maior carta", "vence se ele mentiu" };
    for (int i = 0; i < 2; i++) {
        int x = 336, y = 242 + i * 48;
        bool escolhida = acaoSelecionada == i;
        Color fundo = GetColor(0x2A2238FF), borda = GetColor(0x514464FF), cor = COR_MENU;
        if (escolhida) {
            fundo = GetColor(0x3F3157FF);
            borda = COR_MARCADOR;
            cor = COR_MARCADOR;
        }
        painel(x, y, 224, 42, fundo, borda);
        textoPlaneta(acoes[i], (float)x + 14, (float)y + 8, 16, cor);
        textoPlaneta(explica[i], (float)x + 14, (float)y + 28, 8, escolhida ? COR_TITULO : COR_SUBTITULO);
    }
}

static void desenharResultadoDaRodada(void)
{
    bool fim = cartas.fase == CARTAS_ENCERRADAS;
    bool blefe = cartas.anuncioAlien != cartas.maoAlien[cartas.indiceAlien];
    DrawRectangle(0, 34, LARGURA_VIRTUAL, ALTURA_VIRTUAL - 48, Fade(BLACK, 0.6f));
    painel(96, 80, 448, 216, COR_PAINEL, COR_BORDA);

    const char *titulo = "RODADA PERDIDA";
    Color corTitulo = COR_ALERTA;
    if (cartas.venceuRodada) {
        titulo = "RODADA VENCIDA";
        corTitulo = COR_CIANO;
    }
    if (fim && cartas.pontosJogador >= 2) {
        titulo = "VOCÊ VENCEU A PARTIDA";
        corTitulo = COR_MARCADOR;
    } else if (fim) {
        titulo = "VOCÊ PERDEU A PARTIDA";
        corTitulo = COR_ALERTA;
    }
    textoPlaneta(titulo, 116, 96, 16, corTitulo);

    /* As duas cartas da rodada, lado a lado. */
    textoPlaneta("VOCÊ", 138, 126, 8, COR_MENU);
    desenharCarta(cartas.cartaJogada, 116, 140, 1);
    textoPlaneta(nomeViajante(), 222, 126, 8, COR_MENU);
    desenharCarta(cartas.maoAlien[cartas.indiceAlien], 212, 140, 1);

    textoPlaneta(TextFormat("Ele anunciou %d.", cartas.anuncioAlien), 312, 146, 8, COR_TITULO);
    if (blefe) {
        textoPlaneta("Era blefe.", 312, 162, 8, COR_ALERTA);
    } else {
        textoPlaneta("Era verdade.", 312, 162, 8, COR_CIANO);
    }
    if (cartas.desconfiou) {
        textoPlaneta("Você desconfiou.", 312, 186, 8, COR_MENU);
    } else {
        textoPlaneta("Você comparou os valores.", 312, 186, 8, COR_MENU);
    }
    textoPlaneta(TextFormat("Placar: você %d x %d ele", cartas.pontosJogador, cartas.pontosAlien),
                 312, 210, 8, COR_SUBTITULO);

    if (fim && cartas.pontosJogador >= 2 && valendoMapa) {
        /* O prêmio: o pedaço do mapa, brilhando. */
        float pulso = 0.5f + 0.5f * sinf(tempo * 4);
        Rectangle origem = { (float)(ITEM_MAPA * TAMANHO_ITEM), 0, TAMANHO_ITEM, TAMANHO_ITEM };
        DrawRectangle(500, 218, 40, 40, Fade(COR_MARCADOR, 0.15f + 0.2f * pulso));
        DrawTexturePro(folhaItens, origem, (Rectangle){ 504, 222, 32, 32 }, (Vector2){ 0, 0 }, 0, WHITE);
        textoPlaneta("O pedaço do mapa é seu!", 312, 234, 8, COR_MARCADOR);
    } else if (fim && valendoMapa) {
        textoPlaneta("O mapa fica com ele.", 312, 234, 8, COR_ALERTA);
    }
    if (fim) {
        textoPlaneta("ENTER  Continuar", 116, 272, 8, COR_MARCADOR);
    } else {
        textoPlaneta("ENTER  Próxima rodada", 116, 272, 8, COR_MARCADOR);
    }
}

static void desenharMesaDeCartas(void)
{
    /* Fundo do bar, o viajante e, por cima dele, o tampo da mesa. */
    DrawTextureRec(fundoMesa, (Rectangle){ 0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL }, (Vector2){ 0, 0 }, WHITE);
    float balanco = sinf(tempo * 1.8f) * 1.5f;
    desenharAlien(QUEM_VIAJANTE, 120, 214 + balanco, 0, 3, false);
    DrawTextureRec(fundoMesa, (Rectangle){ 0, ALTURA_VIRTUAL, LARGURA_VIRTUAL, ALTURA_VIRTUAL },
                   (Vector2){ 0, 0 }, WHITE);

    /* Faixa de cima: rodada e placar. */
    DrawRectangle(0, 0, LARGURA_VIRTUAL, 34, Fade(GetColor(0x17131FFF), 0.9f));
    textoPlaneta("BLEFE ORBITAL", 16, 9, 16, COR_TITULO);
    textoPlaneta(TextFormat("RODADA %d/3", cartas.rodada), 248, 13, 8, COR_SUBTITULO);
    textoPlaneta("VOCÊ", 372, 13, 8, COR_MENU);
    desenharPontos(408, 13, cartas.pontosJogador, COR_MARCADOR);
    textoPlaneta(nomeViajante(), 448, 13, 8, COR_MENU);
    desenharPontos(448 + larguraTexto(nomeViajante()) + 6, 13, cartas.pontosAlien, GetColor(0xD86AA4FF));
    if (valendoMapa && folhaItens.id != 0) {
        Rectangle origem = { (float)(ITEM_MAPA * TAMANHO_ITEM), 0, TAMANHO_ITEM, TAMANHO_ITEM };
        DrawTexturePro(folhaItens, origem, (Rectangle){ 600, 1, 32, 32 }, (Vector2){ 0, 0 }, 0, WHITE);
    }

    desenharBalaoDoAnuncio();
    desenharCartasDoViajante();
    desenharMaoDoJogador();
    desenharBotoesDeAcao();

    DrawRectangle(0, 346, LARGURA_VIRTUAL, 14, GetColor(0x17131FFF));
    if (tempoDesistir > 0) {
        textoPlaneta("ESC de novo para desistir: conta como derrota", 16, 350, 8, COR_ALERTA);
    } else {
        textoPlaneta("A/D Carta   W/S Ação   ENTER Jogar   ESC Desistir", 16, 350, 8, COR_MENU);
    }
    if (cartas.fase != CARTAS_ESCOLHENDO) {
        desenharResultadoDaRodada();
    }
}

void desenharTelaExtraterrestre(void)
{
    if (!iniciou) {
        return;
    }
    if (modo == ET_CARTAS) {
        desenharMesaDeCartas();
        return;
    }
    if (modo == ET_FLIPERAMA) {
        desenharTelaFliperama();
        return;
    }

    ClearBackground(GetColor(0x16121FFF));
    Vector2 canto = cantoDaCamera();
    Camera2D camera = { 0 };
    camera.target = canto;
    camera.zoom = 1;

    BeginMode2D(camera);
    if (lugar == LUGAR_RUA) {
        desenharRua();
    } else {
        desenharBar();
    }
    EndMode2D();

    desenharInterface(canto);
    if (modo == ET_FALA) {
        desenharFalaAtual();
    } else if (modo == ET_PERGUNTA) {
        desenharPergunta();
    } else if (modo == ET_REGRAS) {
        desenharRegras();
    } else if (modo == ET_TROCA) {
        /* Escurece até a metade do tempo e depois clareia. */
        float metade = TEMPO_TROCA / 2;
        float escuro = 1 - fabsf(tempoTroca - metade) / metade;
        DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, Fade(BLACK, limitar(escuro, 0, 1)));
    }
}

void encerrarTelaExtraterrestre(void)
{
    if (!iniciou) {
        return;
    }
    Texture2D texturas[] = { fundoRua, fundoBar, frenteBar, fundoMesa,
                             folhaAliens, folhaPoste, folhaCartas, folhaItens };
    for (int i = 0; i < TOTAL(texturas); i++) {
        if (texturas[i].id != 0) {
            UnloadTexture(texturas[i]);
        }
    }
    descarregarFliperama();
    descarregarAnimacao(&planeta);
    free(pixelsRua);
    free(pixelsBar);
    pixelsRua = NULL;
    pixelsBar = NULL;
    pisoRua = (MapaExploracao){ 0 };
    pisoBar = (MapaExploracao){ 0 };
    encerrarExploradorPlaneta(&explorador);
    iniciou = false;
}
