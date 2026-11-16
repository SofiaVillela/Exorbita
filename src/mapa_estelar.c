/* mapa_estelar.c: painel de planetas, aberto pelo console central da nave.

   Os planetas ficam numa lista circular duplamente encadeada
   (lista_planetas.c). O painel mostra um planeta por vez, grande e no
   centro. A seta da esquerda vai para o ant e a da direita para o prox;
   como a lista é circular, do último se vai para o primeiro sem nenhum if.

   Regras:
   - toda viagem gasta o custo do planeta;
   - só se viaja com combustível suficiente para o custo; sem nenhum
     destino ao alcance, a nave fica presa (final ruim, tela_nave.c);
   - os planetas abrem em ordem: o Sem Ar já começa aberto; visitar o
     Sem Ar abre o Gigante; o Gigante abre o Ser; o Ser abre o Robô; o
     Robô abre o Extraterrestre, o último, onde está o último pedaço
     do mapa estelar;
   - a Via Láctea começa bloqueada e abre quando os três pedaços do
     mapa estelar estão na nave (Sem Ar, Gigante e Extraterrestre). */

#include <string.h>
#include "raylib.h"
#include "constantes.h"
#include "recursos.h"
#include "fundo.h"
#include "animacao.h"
#include "partida.h"
#include "lista_planetas.h"
#include "mapa_estelar.h"
#include "sons.h"
#include "tanque.h"

/* Tanque de combustível do painel, em pé à direita do planeta. */
#define X_TANQUE_PAINEL       560   /* centro */
#define Y_TANQUE_PAINEL       100   /* topo da tampa: centrado com o planeta */
#define LARGURA_TANQUE_PAINEL 64
#define ALTURA_TANQUE_PAINEL  120

#define TOTAL_PLANETAS_PAINEL 6

/* Dados para montar a lista. A ordem aqui é a ordem no painel.
   destino = TELA_NAVE quer dizer "ainda sem tela": o painel mostra o
   planeta, mas não deixa pousar. arquivoBloqueado só existe na Via
   Láctea: é a imagem desenhada enquanto ela está bloqueada.
   requisito é o índice em dados[] do planeta que precisa ser visitado
   para este abrir (SEM_REQUISITO num bloqueado = a Via Láctea, que
   abre com o mapa estelar completo). */
#define SEM_REQUISITO -1
enum { SEM_AR, GIGANTE, SER, ROBO, EXTRATERRESTRE, VIA_LACTEA };

typedef struct {
    const char *nome;
    const char *arquivo;
    const char *arquivoBloqueado;
    int custo;
    Tela destino;
    bool bloqueado;
    int requisito;
} DadosPlaneta;

static const DadosPlaneta dados[TOTAL_PLANETAS_PAINEL] = {
    { "SEM AR", "assets/imagens/planeta_sem_ar.png", NULL,
      CUSTO_SEM_AR, TELA_SEM_AR, false, SEM_REQUISITO },
    { "GIGANTE", "assets/imagens/planeta_gigante.png", NULL,
      CUSTO_GIGANTE, TELA_GIGANTE, true, SEM_AR },
    { "SER", "assets/imagens/planeta_ser.png", NULL,
      CUSTO_SER, TELA_SER, true, GIGANTE },
    { "ROBÔ", "assets/imagens/planeta_robo.png", NULL,
      CUSTO_ROBO, TELA_ROBO, true, SER },
    { "EXTRATERRESTRE", "assets/imagens/planeta_extraterrestre.png", NULL,
      CUSTO_EXTRATERRESTRE, TELA_EXTRATERRESTRE, true, ROBO },
    { "VIA LÁCTEA", "assets/imagens/via_lactea.png",
      "assets/imagens/via_lactea_bloqueada.png",
      CUSTO_VIA_LACTEA, TELA_FINAL, true, SEM_REQUISITO }
};

/* As imagens ficam fora da lista, porque a lista não usa raylib.
   animacoes[i] e bloqueadas[i] correspondem a dados[i]. */
static Animacao animacoes[TOTAL_PLANETAS_PAINEL];
static Animacao bloqueadas[TOTAL_PLANETAS_PAINEL];

static Planeta *lista = NULL;        /* início da lista circular */
static Planeta *selecionado = NULL;  /* nó que está no centro do painel */

static bool iniciado = false;
static bool aberto = false;
static Tela viagemSolicitada = TELA_NAVE;

static const char *aviso = NULL;     /* mensagem de alerta, se houver */
static float tempoAviso = 0;

/* ---------- Funções de apoio ---------- */

/* Acha a posição de um nó em dados[], comparando o caminho do arquivo.
   strcmp compara o texto; == compararia só os endereços. */
static int indiceDe(const Planeta *p)
{
    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        if (strcmp(p->arquivo, dados[i].arquivo) == 0) {
            return i;
        }
    }
    return 0;
}

/* Animação certa para o planeta: a bloqueada, se ele estiver bloqueado
   e tiver essa imagem; senão, a normal. */
static const Animacao *animacaoDe(const Planeta *p)
{
    int i = indiceDe(p);
    if (p->bloqueado && dados[i].arquivoBloqueado != NULL) {
        return &bloqueadas[i];
    }
    return &animacoes[i];
}

/* Desenha o quadro atual do planeta com o centro em (x, y), ampliado
   pela escala. Igual ao desenharAnimacao, mas a partir do centro. */
static void desenharPlaneta(const Planeta *p, int x, int y, int escala)
{
    const Animacao *a = animacaoDe(p);
    if (a->folha.id == 0 || a->colunas == 0) {
        return;   /* imagem não carregou: não desenha (e não divide por zero) */
    }
    int coluna = a->quadroAtual % a->colunas;
    int linha = a->quadroAtual / a->colunas;
    Rectangle origem = { coluna * a->larguraQuadro, linha * a->alturaQuadro,
                         a->larguraQuadro, a->alturaQuadro };
    int tamanho = a->larguraQuadro * escala;
    Rectangle destino = { x - tamanho / 2, y - tamanho / 2, tamanho, tamanho };
    /* Bloqueado sem imagem própria: o planeta fica escurecido. */
    Color tom = WHITE;
    if (p->bloqueado && dados[indiceDe(p)].arquivoBloqueado == NULL) {
        tom = GetColor(0x6A6478FF);
    }
    DrawTexturePro(a->folha, origem, destino, (Vector2){ 0, 0 }, 0, tom);
}

/* Cadeado 12x13 em pixel art. S = arco de metal, O = contorno,
   Y = corpo dourado, H = brilho, D = sombra do corpo, K = fechadura. */
static const char *cadeado[13] = {
    "...SSSSSS...",
    "..SS....SS..",
    ".SS......SS.",
    ".SS......SS.",
    ".SS......SS.",
    "OOOOOOOOOOOO",
    "OHYYYYYYYYYO",
    "OHYYYKKYYYYO",
    "OYYYYKKYYYYO",
    "OYYYYKKYYYYO",
    "OYYYYYYYYYYO",
    "ODDDDDDDDDDO",
    "OOOOOOOOOOOO",
};

/* Desenha o cadeado com o centro em (x, y), cada pixel com "escala" px. */
static void desenharCadeado(int x, int y, int escala)
{
    int x0 = x - 12 * escala / 2;
    int y0 = y - 13 * escala / 2;
    for (int linha = 0; linha < 13; linha++) {
        for (int coluna = 0; coluna < 12; coluna++) {
            char c = cadeado[linha][coluna];
            Color cor;
            if (c == 'S') cor = GetColor(0xB8B0C8FF);
            else if (c == 'O' || c == 'K') cor = GetColor(0x3A2F4AFF);
            else if (c == 'Y') cor = COR_MARCADOR;
            else if (c == 'H') cor = GetColor(0xFFF0B0FF);
            else if (c == 'D') cor = GetColor(0xB08A3AFF);
            else continue;
            int px = x0 + coluna * escala;
            int py = y0 + linha * escala;
            DrawRectangle(px + escala, py + escala, escala, escala, Fade(BLACK, 0.4f));   /* sombra */
            DrawRectangle(px, py, escala, escala, cor);
        }
    }
}

/* Escreve um texto com o centro em x. */
static void textoCentralizado(const char *texto, int x, int y, int tamanho, Color cor)
{
    Vector2 medida = MeasureTextEx(fontePadrao(), texto, tamanho, 0);
    DrawTextEx(fontePadrao(), texto, (Vector2){ x - (int)medida.x / 2, y },
               tamanho, 0, cor);
}

/* Abre o que já pode abrir: os planetas pelo requisito visitado e a
   Via Láctea pelos pedaços do mapa. Os pedaços chegam durante as
   missões, por isso é chamada sempre que o painel é aberto. */
static void atualizarBloqueios(void)
{
    atualizarDesbloqueios(lista, mapaEstelarCompleto());
}

static void avisar(const char *mensagem)
{
    /* Copia: a mensagem pode vir do TextFormat, que reaproveita o
       mesmo espaço nas próximas chamadas. */
    static char textoAviso[96];
    strncpy(textoAviso, mensagem, sizeof textoAviso - 1);
    textoAviso[sizeof textoAviso - 1] = '\0';
    aviso = textoAviso;
    tempoAviso = TEMPO_AVISO_PAINEL;
    tocarSom(SOM_NEGADO);      /* todo aviso daqui é uma viagem que não pôde ser feita */
}

/* Tenta viajar para o planeta selecionado. Devolve true se a viagem
   aconteceu. Cada regra que impede a viagem mostra um aviso. */
static bool tentarViajar(void)
{
    ProgressoPartida *jogo = progressoPartida();

    if (selecionado->bloqueado) {
        if (selecionado->requisito != NULL) {
            avisar(TextFormat("Bloqueado: visite o planeta %s antes.",
                              selecionado->requisito->nome));
        } else {
            avisar(TextFormat("Bloqueado: faltam pedaços do mapa estelar (%d/%d).",
                              pedacosMapaObtidos(), TOTAL_PEDACOS_MAPA));
        }
        return false;
    }
    if (selecionado->destino == TELA_NAVE) {
        avisar("Pouso ainda indisponível neste planeta.");
        return false;
    }
    if (jogo->combustivel < selecionado->custo) {
        avisar("Combustível insuficiente para esta viagem.");
        return false;
    }

    jogo->combustivel -= selecionado->custo;
    selecionado->visitado = true;

    /* Abre o que dependia deste planeta. */
    atualizarBloqueios();

    viagemSolicitada = selecionado->destino;
    return true;
}

/* ---------- Funções do mapa_estelar.h ---------- */

void iniciarMapaEstelar(void)
{
    if (iniciado) {
        return;
    }

    /* Monta a lista circular na ordem de dados[]. */
    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        inserirPlaneta(&lista, dados[i].nome, dados[i].arquivo,
                       dados[i].custo, dados[i].destino, dados[i].bloqueado);

        carregarAnimacao(&animacoes[i], dados[i].arquivo,
                         TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
        if (dados[i].arquivoBloqueado != NULL) {
            carregarAnimacao(&bloqueadas[i], dados[i].arquivoBloqueado,
                             TAMANHO_PLANETA, TAMANHO_PLANETA, QUADROS_PLANETA);
        }
    }

    /* Liga cada planeta ao seu requisito. nos[i] corresponde a dados[i]. */
    Planeta *nos[TOTAL_PLANETAS_PAINEL];
    Planeta *atual = lista;
    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        nos[i] = atual;
        atual = atual->prox;
    }
    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        if (dados[i].requisito != SEM_REQUISITO) {
            nos[i]->requisito = nos[dados[i].requisito];
        }
    }

    selecionado = lista;   /* começa no primeiro planeta */
    iniciado = true;
}

void abrirMapaEstelar(void)
{
    aberto = true;
    atualizarBloqueios();   /* um pedaço novo do mapa pode ter aberto a Via Láctea */
    viagemSolicitada = TELA_NAVE;
    aviso = NULL;
    tempoAviso = 0;
    /* selecionado não muda: o painel abre onde o jogador parou. */
}

Tela destinoEscolhidoMapaEstelar(void)
{
    return viagemSolicitada;
}

bool atualizarMapaEstelar(void)
{
    if (!aberto || selecionado == NULL) {
        return false;
    }

    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        atualizarAnimacao(&animacoes[i]);
        if (dados[i].arquivoBloqueado != NULL) {
            atualizarAnimacao(&bloqueadas[i]);
        }
    }

    if (tempoAviso > 0) {
        tempoAviso -= GetFrameTime();
    }

    /* Esc fecha sem viajar: a tela da nave continua. */
    if (IsKeyPressed(KEY_ESCAPE)) {
        aberto = false;
        viagemSolicitada = TELA_NAVE;
        return true;
    }

    /* Setas andam pela lista: ant para a esquerda, prox para a direita. */
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        selecionado = selecionado->ant;
        tocarSom(SOM_UI_MOVER);
        aviso = NULL;
    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        selecionado = selecionado->prox;
        tocarSom(SOM_UI_MOVER);
        aviso = NULL;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if (tentarViajar()) {
            aberto = false;
            return true;
        }
    }
    return false;
}

void desenharMapaEstelar(void)
{
    if (!aberto || selecionado == NULL) {
        return;
    }
    ProgressoPartida *jogo = progressoPartida();

    /* Guia de estilo: painéis usam o fundo de espaço com estrelas. */
    desenharFundo();

    DrawTextEx(fontePadrao(), "PAINEL DE PLANETAS", (Vector2){ MARGEM_PAINEL, MARGEM_PAINEL },
               16, 0, COR_TITULO);

    /* Direita: o tanque grande. Os blocos que esta viagem gastaria
       piscam em pêssego no topo da pilha. */
    int custoViagem = selecionado->bloqueado ? 0 : selecionado->custo;
    textoCentralizado("COMBUSTÍVEL", X_TANQUE_PAINEL, Y_TANQUE_PAINEL - 16, 8, COR_TITULO);
    desenharTanque(X_TANQUE_PAINEL - LARGURA_TANQUE_PAINEL / 2, Y_TANQUE_PAINEL,
                   LARGURA_TANQUE_PAINEL, ALTURA_TANQUE_PAINEL, jogo->combustivel, custoViagem);
    int yNumeros = Y_TANQUE_PAINEL + ALTURA_TANQUE_PAINEL + 8;
    textoCentralizado(TextFormat("%d", jogo->combustivel), X_TANQUE_PAINEL, yNumeros, 16,
                      jogo->combustivel <= 20 ? COR_ALERTA : COR_TITULO);
    if (custoViagem > 0) {
        textoCentralizado(TextFormat("-%d", custoViagem), X_TANQUE_PAINEL, yNumeros + 20, 8, COR_ALERTA);
    }

    /* Planeta selecionado, grande e no centro (pedido do playtest). */
    int metade = TAMANHO_PLANETA * ESCALA_PLANETA_PAINEL / 2;
    desenharPlaneta(selecionado, X_CENTRO_PAINEL, Y_CENTRO_PAINEL, ESCALA_PLANETA_PAINEL);
    if (selecionado->bloqueado) {
        desenharCadeado(X_CENTRO_PAINEL, Y_CENTRO_PAINEL, 4);   /* 48 x 52 px */
    }

    /* Setas dos lados, só se existir mais de um planeta. */
    if (selecionado->ant != selecionado) {
        textoCentralizado("<", X_CENTRO_PAINEL - metade - ESPACO_SETA_PAINEL,
                          Y_CENTRO_PAINEL - 8, 16, COR_MARCADOR);
        textoCentralizado(">", X_CENTRO_PAINEL + metade + ESPACO_SETA_PAINEL,
                          Y_CENTRO_PAINEL - 8, 16, COR_MARCADOR);
    }

    int y = Y_CENTRO_PAINEL + metade + 12;
    textoCentralizado(selecionado->nome, X_CENTRO_PAINEL, y, 16, COR_TITULO);

    /* Situação do planeta: bloqueado, visitado ou não visitado. */
    const char *situacao;
    Color corSituacao = COR_MENU;
    if (selecionado->bloqueado) {
        situacao = "BLOQUEADO";
        corSituacao = COR_ALERTA;
    } else if (selecionado->visitado) {
        situacao = "VISITADO";
    } else {
        situacao = "NÃO VISITADO";
        corSituacao = COR_MARCADOR;
    }
    textoCentralizado(TextFormat("CUSTO %d   %s", selecionado->custo, situacao),
                      X_CENTRO_PAINEL, y + 24, 8, corSituacao);

    /* Quanto sobra depois da viagem. Em pêssego se não der para ir. */
    int depois = jogo->combustivel - selecionado->custo;
    Color corDepois = COR_MENU;
    if (depois < 0) {
        corDepois = COR_ALERTA;
    }
    textoCentralizado(TextFormat("DEPOIS DA VIAGEM: %d", depois),
                      X_CENTRO_PAINEL, y + 38, 8, corDepois);

    if (aviso != NULL && tempoAviso > 0) {
        textoCentralizado(aviso, X_CENTRO_PAINEL, y + 54, 8, COR_ALERTA);
    }

    /* Teclas, no rodapé. */
    textoCentralizado("SETAS  Escolher     ENTER  Viajar     ESC  Voltar",
                      X_CENTRO_PAINEL, ALTURA_VIRTUAL - 16, 8, COR_MENU);
}

void encerrarMapaEstelar(void)
{
    if (!iniciado) {
        return;
    }
    for (int i = 0; i < TOTAL_PLANETAS_PAINEL; i++) {
        descarregarAnimacao(&animacoes[i]);
        if (dados[i].arquivoBloqueado != NULL) {
            descarregarAnimacao(&bloqueadas[i]);
        }
    }
    liberarPlanetas(&lista);
    selecionado = NULL;
    iniciado = false;
    aberto = false;
}

bool algumDestinoAlcancavel(void)
{
    if (lista == NULL) {
        return true;   /* painel ainda não montado: não dá para julgar */
    }
    atualizarBloqueios();   /* conta a Via Láctea, se o mapa acabou de ficar completo */
    int combustivel = progressoPartida()->combustivel;
    Planeta *atual = lista;
    do {
        if (!atual->bloqueado && atual->destino != TELA_NAVE &&
            atual->custo <= combustivel) {
            return true;
        }
        atual = atual->prox;
    } while (atual != lista);
    return false;
}

Planeta *listaPlanetasMapa(void)
{
    return lista;
}

void reiniciarMapaEstelar(void)
{
    if (lista == NULL) {
        return;
    }
    /* Percorre a lista circular e devolve cada planeta ao que está em dados[]. */
    Planeta *atual = lista;
    do {
        int i = indiceDe(atual);
        atual->visitado = false;
        atual->bloqueado = dados[i].bloqueado;
        atual = atual->prox;
    } while (atual != lista);

    selecionado = lista;
    aberto = false;
    viagemSolicitada = TELA_NAVE;
    aviso = NULL;
    tempoAviso = 0;
}
